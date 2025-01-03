import sys
from typing import Tuple
import gymnasium as gym
import numpy as np
from stable_baselines3 import PPO
from stable_baselines3.common.vec_env import DummyVecEnv
from stable_baselines3.common.evaluation import evaluate_policy
import torch as th
import torch.nn as nn
from stable_baselines3.common.torch_layers import BaseFeaturesExtractor
from transformers import GPT2Model, GPT2Config
import ptcg
import random
import wandb

# Check for CUDA availability
device = th.device("cuda" if th.cuda.is_available() else "cpu")
print(f"Using device: {device}")

class TransformerPolicy(BaseFeaturesExtractor):
    def __init__(self, observation_space: gym.spaces.Box, action_space: gym.spaces.MultiDiscrete):
        super().__init__(observation_space, features_dim=sum(action_space.nvec))  # features_dim is not used in this implementation
        n_input_channels = observation_space.shape[0]
        self.action_dims = action_space.nvec  # [97, 8, 8]
        self.vocab_size = sum(self.action_dims)
        
        config = GPT2Config(
            n_embd=64,
            n_layer=4,
            n_head=4,
            n_positions=n_input_channels,
            vocab_size=self.vocab_size,
            output_hidden_states=True
        )
        self.transformer = GPT2Model(config).to(device)
        self.action_head = nn.Linear(64, self.vocab_size).to(device)

    def forward(self, observations: th.Tensor) -> th.Tensor:
        x = observations.unsqueeze(-1).to(device)
        transformer_output = self.transformer(inputs_embeds=x)
        hidden_states = transformer_output.last_hidden_state
        logits = self.action_head(hidden_states[:, -1, :])
        return logits

    def get_action_dist(self, logits: th.Tensor, action_masks: th.Tensor) -> th.Tensor:
        action_masks = action_masks.to(device)

        action_type_logits = logits[0, :self.action_dims[0]]
        action_type_mask = action_masks[:self.action_dims[0]]
        action_type_probs = th.softmax(action_type_logits + (action_type_mask + 1e-10).log(), dim=-1)
        return action_type_probs
    
    def get_target_dist(self, logits: th.Tensor, target_mask: th.Tensor, opponent_target_mask: th.Tensor) -> Tuple[th.Tensor, th.Tensor]:
        target_mask = target_mask.to(device)
        opponent_target_mask = opponent_target_mask.to(device)
        
        target_logits = logits[0, self.action_dims[0]:self.action_dims[0]+self.action_dims[1]]
        opponent_target_logits = logits[0, self.action_dims[0]+self.action_dims[1]:]

        target_probs = th.softmax(target_logits + (target_mask + 1e-10).log(), dim=-1)
        opponent_target_probs = th.softmax(opponent_target_logits + (opponent_target_mask + 1e-10).log(), dim=-1)

        return target_probs, opponent_target_probs


class PolicyPool:
    def __init__(self, env, num_policies=5):
        self.env = env
        self.policies = [self.create_new_policy() for _ in range(num_policies)]
        self.wins = [0] * num_policies
        self.games_played = [0] * num_policies

    def create_new_policy(self):
        return PPO("MlpPolicy", self.env, verbose=0, policy_kwargs={
            "features_extractor_class": TransformerPolicy,
            "features_extractor_kwargs": {"action_space": self.env.action_space}
        }, device=device)

    def get_random_policy(self):
        return random.choice(self.policies)

    def update_policy(self, index, new_policy):
        self.policies[index] = new_policy

    def update_winrate(self, index, won):
        self.games_played[index] += 1
        if won:
            self.wins[index] += 1

    def get_winrate(self, index):
        if self.games_played[index] == 0:
            return 0
        return self.wins[index] / self.games_played[index]

def self_play_episode(env, policy1, policy2):
    obs = env.reset()
    done = False
    total_reward = 0
    
    while not done:
        if env.cy_ptcg.game.current_player == 0:
            action, _ = policy1.predict(obs, deterministic=False)
        else:
            action, _ = policy2.predict(obs, deterministic=False)
        
        obs, reward, terminated, truncated, _ = env.step(action)
        total_reward += reward
        done = terminated or truncated
    
    return total_reward


def train_with_self_play(env, policy_pool, total_timesteps=1000000, num_self_play_games=100):
    wins = [0] * len(policy_pool.policies)
    games_played = [0] * len(policy_pool.policies)

    for i in range(total_timesteps // num_self_play_games):
        policy1_index = random.randint(0, len(policy_pool.policies) - 1)
        policy2_index = random.randint(0, len(policy_pool.policies) - 1)
        policy1 = policy_pool.policies[policy1_index]
        policy2 = policy_pool.policies[policy2_index]
        
        episode_rewards = []
        for _ in range(num_self_play_games):
            print('new game', flush=True)
            observation, info = env.reset()
            print('reset', flush=True)
            done = False
            while not done:
                if env.cy_ptcg.get_player() == 0:
                    action_mask = env.get_action_mask()
                    action = select_masked_action(policy1, observation, action_mask, env.get_action_mask_target)
                else:
                    action_mask = env.get_action_mask()
                    action = select_masked_action(policy2, observation, action_mask, env.get_action_mask_target)
                
                observation, reward, terminated, truncated, _ = env.step(action)
                done = terminated or truncated
                print(done)
            print('finished while')
            episode_rewards.append(reward)
            games_played[policy1_index] += 1
            games_played[policy2_index] += 1
            if reward > 0:
                wins[policy1_index] += 1
            else:
                wins[policy2_index] += 1

        policy1.learn(total_timesteps=num_self_play_games)
        policy2.learn(total_timesteps=num_self_play_games)
        
        policy_pool.update_policy(policy1_index, policy1)
        policy_pool.update_policy(policy2_index, policy2)
        
        if i % 10 == 0:
            mean_reward = np.mean(episode_rewards)
            winrates = [wins[j] / games_played[j] if games_played[j] > 0 else 0 for j in range(len(policy_pool.policies))]
            wandb.log({
                "step": i * num_self_play_games,
                "mean_reward": mean_reward,
                "avg_episode_reward": np.mean(episode_rewards),
                "max_episode_reward": np.max(episode_rewards),
                "min_episode_reward": np.min(episode_rewards),
                "policy_winrates": wandb.Histogram(winrates)
            })
            print(f"Step: {i * num_self_play_games}, Mean reward: {mean_reward}")
            for j, winrate in enumerate(winrates):
                print(f"Policy {j} winrate: {winrate:.2f}")

    winrates = [wins[j] / games_played[j] if games_played[j] > 0 else 0 for j in range(len(policy_pool.policies))]
    best_policy_index = max(range(len(winrates)), key=winrates.__getitem__)
    best_policy = policy_pool.policies[best_policy_index]
    
    print(f"\nTraining completed. Best policy: {best_policy_index} with winrate: {winrates[best_policy_index]:.2f}")
    return best_policy

def select_masked_action(policy, observation, action_mask, get_action_mask_target):
    with th.no_grad():
        logits = policy.policy.features_extractor(th.tensor(observation).reshape(1,-1))
        action_mask = th.tensor(action_mask)
        # action_type_probs, target_probs, opponent_target_probs = policy.policy.features_extractor.get_action_dist(logits, action_mask)
        action_type_probs = policy.policy.features_extractor.get_action_dist(logits, action_mask)

    action_type = th.multinomial(action_type_probs, 1).item()

    target_mask, opponent_mask = get_action_mask_target(action_type)
    target_mask = th.tensor(target_mask)
    opponent_mask = th.tensor(opponent_mask)
    target_probs, opponent_target_probs = policy.policy.features_extractor.get_target_dist(logits, target_mask, opponent_mask)
    if (target_mask == 0).all(): target = -1
    else: target = th.multinomial(target_probs, 1).item()
    if (opponent_mask == 0).all(): opponent_target = -1
    else: opponent_target = th.multinomial(opponent_target_probs, 1).item()

    print(action_type, target, opponent_target)

    return int(action_type), int(target), int(opponent_target)


if __name__ == "__main__":
    wandb.init(project="ptcg-self-play", config={
        "num_policies": 5,
        "total_timesteps": 1000000,
        "num_self_play_games": 100
    })

    env = gym.make('PTCG-v0')
    
    policy_pool = PolicyPool(env, num_policies=wandb.config.num_policies)
    
    best_policy = train_with_self_play(env, policy_pool, 
                                       total_timesteps=wandb.config.total_timesteps, 
                                       num_self_play_games=wandb.config.num_self_play_games)
    
    best_policy.save("ppo_ptcg_self_play_model")
    
    mean_reward, std_reward = evaluate_policy(best_policy, env, n_eval_episodes=100)
    wandb.log({"final_mean_reward": mean_reward, "final_std_reward": std_reward})
    print(f"Mean reward: {mean_reward:.2f} +/- {std_reward:.2f}")

    wandb.finish()

