import argparse
import os
from typing import Any, Dict, Mapping, Optional, Tuple, Union, Type, List, TypeVar

import gymnasium as gym
gym.logger.set_level(gym.logger.ERROR)

import ptcg

import warnings
warnings.filterwarnings('ignore', category=UserWarning)

import random
import multiprocessing
multiprocessing.set_start_method('forkserver', force=True)
from multiprocessing import Process

import numpy as np
import torch as th
import torch.nn as nn

from utils import Monitor2P, SubprocVecEnv2P, DummyVecEnv
from algorithms import LeaguePPO
from league import PayoffManager, League, Learner

from stable_baselines3.common.utils import get_linear_fn
from stable_baselines3.common.policies import ActorCriticPolicy
from stable_baselines3.common.torch_layers import BaseFeaturesExtractor
from stable_baselines3.common.distributions import (
    CategoricalDistribution,
    Distribution,
)

import wandb

# Check for CUDA availability
# device = th.device("cuda" if th.cuda.is_available() else "cpu")
# print(f"Using device: {device}")
device = "cpu"

random.seed(0)
np.random.seed(0)
th.random.manual_seed(0)

class PTCGPolicy(ActorCriticPolicy):
    def __init__(
        self,
        observation_space: gym.spaces.Space,
        action_space: gym.spaces.Space,
        lr_schedule: callable,
        net_arch: List[int] = [64, 64],
        activation_fn: nn.Module = nn.Tanh,
        features_extractor_class: BaseFeaturesExtractor = BaseFeaturesExtractor,
        features_extractor_kwargs: Dict[str, Any] = None,
        normalize_images: bool = True,
        optimizer_class: th.optim.Optimizer = th.optim.Adam,
        optimizer_kwargs: Dict[str, Any] = None,
        policy_type: str = "MLP",
        use_sde: bool = False,
        sde_sample_freq: int = -1,
        log_std_init: float = -2.0,
    ):
        super().__init__(
            observation_space,
            action_space,
            lr_schedule,
            net_arch,
            activation_fn,
            features_extractor_class,
            features_extractor_kwargs,
            normalize_images,
            optimizer_class,
            optimizer_kwargs,
        )
        self.use_sde = use_sde
        self.sde_sample_freq = sde_sample_freq
        self.policy_type = policy_type
        self.action_dims = action_space.n

        # SDE-specific attributes
        self.log_std = None
        self.log_std_init = log_std_init
        self.sample_weights()

        if policy_type == "MLP":
            self.policy_net = nn.Sequential(
                nn.Linear(64, 128),
                nn.ReLU(),
                nn.Linear(128, 128),
                nn.ReLU(),
                nn.Linear(128, self.action_dims)
            )
        elif policy_type == "LSTM":
            self.lstm = nn.LSTM(64, 64, batch_first=True)
            self.policy_net = nn.Linear(64, self.action_dims)
        else:
            raise ValueError(f"Unknown policy type: {policy_type}")

        self.value_net = nn.Linear(64, 1)

    def forward(self, obs: th.Tensor, action_masks: th.Tensor = None, deterministic: bool = False, use_sde: bool = False) -> Tuple[th.Tensor, th.Tensor, th.Tensor]:
        # assert action_masks is not None, "forward: action_masks not none"
        features = self.extract_features(obs)
        
        if self.policy_type == "LSTM":
            features, _ = self.lstm(features.unsqueeze(1))
            features = features.squeeze(1)
        
        latent_pi = self.mlp_extractor.policy_net(features)
        latent_vf = self.mlp_extractor.value_net(features)
        
        values = self.value_net(latent_vf)
        action_logits = self.policy_net(latent_pi)
        
        if self.use_sde:
            action_logits += self.log_std.exp() * th.randn_like(action_logits)
        
        if action_masks is not None:
            action_logits = th.where(action_masks, action_logits, th.tensor(-float('inf'), device=action_logits.device))
        
        distribution = self.get_action_dist_from_latent(action_logits)
        actions = distribution.get_actions(deterministic=deterministic)
        log_prob = distribution.log_prob(actions)
        
        return actions, values, log_prob

    def get_action_dist_from_latent(self, latent_pi: th.Tensor) -> CategoricalDistribution:
        action_logits = latent_pi
        
        # Create the CategoricalDistribution instance
        distribution = CategoricalDistribution(action_dim=action_logits.shape[-1])
        
        # Set the parameters of the distribution
        return distribution.proba_distribution(action_logits)

    def evaluate_actions(self, obs: th.Tensor, actions: th.Tensor, action_masks: th.Tensor = None) -> Tuple[th.Tensor, th.Tensor, th.Tensor]:
        # assert action_masks is not None, "evaluate_actions: action_masks not none"
        features = self.extract_features(obs)
        latent_pi = self.mlp_extractor.policy_net(features)
        latent_vf = self.mlp_extractor.value_net(features)
        
        values = self.value_net(latent_vf)
        action_logits = self.policy_net(latent_pi)

        if self.use_sde:
            action_logits += self.log_std.exp() * th.randn_like(action_logits)
        
        if action_masks is not None:
            action_logits = th.where(action_masks, action_logits, th.tensor(-float('inf'), device=action_logits.device))
        
        distribution = self.get_action_dist_from_latent(action_logits)
        log_prob = distribution.log_prob(actions)
        entropy = distribution.entropy()
        
        return values, log_prob, entropy


    def predict(self, observation: th.Tensor, action_masks: th.Tensor = None, deterministic: bool = False) -> Tuple[th.Tensor, Optional[th.Tensor]]:
        with th.no_grad():
            actions, values, _ = self.forward(observation, action_masks, deterministic)
        return actions, values
    
    def sample_weights(self, batch_size: int = 1) -> None:
        if self.use_sde:
            self.log_std = th.randn(batch_size, self.action_dims)
            self.log_std = self.log_std_init + 0.5 * self.log_std

    
    def reset_noise(self, batch_size: int = 1) -> None:
        """
        Sample new weights for the state dependent exploration
        """
        if self.use_sde:
            self.sample_weights(batch_size)


def make_env(seed):
    def _init():
        env = gym.make('PTCG-v0')
        env.unwrapped.seed(seed)
        env = Monitor2P(env)
        return env
    return _init

def constructor(args, player, log_name='', seed=1):
    # env = DummyVecEnv(env)
    env = SubprocVecEnv2P([make_env(seed) for i in range(args.num_env)])
    # env = make_env(seed)
    lr_schedule = get_linear_fn(
        start=3e-4,
        end=1e-5,
        end_fraction=0.1
    )
    # policy = PTCGPolicy(env.observation_space, env.action_space, lr_schedule)
    return LeaguePPO(
        player,
        PTCGPolicy, 
        env,
        device="cpu", 
        verbose=1,
        n_steps=512,
        batch_size=512*args.num_env, # 512,
        n_epochs=4,
        gamma=0.94,
        learning_rate=1e-4, # lr_schedule,
        clip_range=0.1, # clip_range_schedule,
        tensorboard_log=None if log_name is None else os.path.join(args.log_dir, log_name),
        seed=seed,
        other_learning_rate=1e-4, # other_lr_schedule,
    )


def worker(idx, learner, total_steps, rollout_opponent_num):
    print(f"worker {learner.player.name} start")
    # with th.cuda.device(idx % th.cuda.device_count()):
    learner.player.construct_agent()
    learner.run(total_timesteps=total_steps, rollout_opponent_num=rollout_opponent_num)


def main_league():
    parser = argparse.ArgumentParser(description='Reset game stats')
    parser.add_argument('--save-dir', help='The directory to save the trained models', default="trained_models/ma")
    parser.add_argument('--log-dir', help='The directory to save logs', default="logs/ma")
    parser.add_argument('--num-env', type=int, help='How many envirorments to create', default=1)
    parser.add_argument('--total-steps', type=int, help='How many total steps to train', default=int(1e5)) # 1e5
    # parser.add_argument('--left-model-file', help='The left model to continue to learn from')
    # parser.add_argument('--right-model-file', help='The right model to continue to learn from')
    parser.add_argument('--rollout-opponent-num', type=int, help='Numbers of opponents to interact for each update', default=2) # 2
    args = parser.parse_args()

    print("command line args:" + str(args))

    p1_model = constructor(args, "left", log_name=None)
    p2_model = constructor(args, "right", log_name=None)

    initial_agents = {
        'left': p1_model,
        'right': p2_model,
    }

    with PayoffManager() as manager:
        shared_payoff = manager.Payoff(args.save_dir)
        league = League(args=args, initial_agents=initial_agents, constructor=constructor, payoff=shared_payoff, main_agents=1, main_exploiters=1, league_exploiters=2)
        processes = []
        for idx in range(league.size()):
            player = league.get_player(idx)
            learner = Learner(player)
            process = Process(target=worker, args=(idx, learner, args.total_steps, args.rollout_opponent_num))
            # process.daemon=True  # all processes closed when the main stops
            processes.append(process)
        for p in processes:
            p.start()
        for p in processes:
            p.join()


def main():
    parser = argparse.ArgumentParser(description='Reset game stats')
    parser.add_argument('--save-dir', help='The directory to save the trained models', default="trained_models/ma")
    parser.add_argument('--log-dir', help='The directory to save logs', default="logs/ma")
    parser.add_argument('--num-env', type=int, help='How many envirorments to create', default=64)
    parser.add_argument('--total-steps', type=int, help='How many total steps to train', default=int(1e10)) # 1e5
    # parser.add_argument('--left-model-file', help='The left model to continue to learn from')
    # parser.add_argument('--right-model-file', help='The right model to continue to learn from')
    parser.add_argument('--rollout-opponent-num', type=int, help='Numbers of opponents to interact for each update', default=5) # 2
    args = parser.parse_args()

    print("command line args:" + str(args))

    p1_model = constructor(args, "left", log_name=None)
    p2_model = constructor(args, "right", log_name=None)

    initial_agents = {
        'left': p1_model,
        'right': p2_model,
    }

    with PayoffManager() as manager:
        shared_payoff = manager.Payoff(args.save_dir)
        league = League(args=args, initial_agents=initial_agents, constructor=constructor, payoff=shared_payoff, main_agents=1, main_exploiters=1, league_exploiters=2)
        processes = []
        for idx in range(league.size()):
            player = league.get_player(idx)
            learner = Learner(player)
            worker(idx, learner, args.total_steps, args.rollout_opponent_num)
            # process = Process(target=worker, args=(idx, learner, args.total_steps, args.rollout_opponent_num))
            # process.daemon=True  # all processes closed when the main stops
        #     processes.append(process)
        # for p in processes:
        #     p.start()
        # for p in processes:
        #     p.join()

if __name__ == "__main__":
    main()
    # total_timesteps = 1000000
    # num_self_play_games = 100
    # # wandb.init(project="ptcg-self-play", config={
    # #     "num_policies": num_policies,
    # #     "total_timesteps": total_timesteps,
    # #     "num_self_play_games": num_self_play_games
    # # })

    # env = gym.make('PTCG-v0')
    
    # policy_pool = PolicyPool(env, num_policies=num_policies)
    
    # best_policy = train_with_self_play(env, policy_pool, 
    #                                    total_timesteps=total_timesteps, 
    #                                    num_self_play_games=num_self_play_games)
    
    # best_policy.save("ppo_ptcg_self_play_model")
    
    # mean_reward, std_reward = evaluate_policy(best_policy, env, n_eval_episodes=100)
    # # wandb.log({"final_mean_reward": mean_reward, "final_std_reward": std_reward})
    # print(f"Mean reward: {mean_reward:.2f} +/- {std_reward:.2f}")

    # wandb.finish()

