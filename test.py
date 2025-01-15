import gymnasium as gym
import torch
import numpy as np
from algorithms import LeaguePPO
from ptcg import PTCGEnv
from train import PTCGPolicy, constructor
from stable_baselines3.common.utils import get_linear_fn

# Load the environment
env = gym.make('PTCG-v0')

# Load the trained model
model_path = "trained_models/ma/MA0_left_0.pt"
loaded_model = torch.load(model_path, map_location=torch.device('cpu'))

# Create a new LeaguePPO instance
# args = type('Args', (), {'num_env': 1})()  # Create a simple object to mimic args
lr_schedule = get_linear_fn(
        start=3e-4,
        end=1e-5,
        end_fraction=0.1
    )
model = LeaguePPO(
        "left",
        PTCGPolicy, 
        env,
        device="cpu", 
        verbose=1,
        n_steps=512,
        batch_size=512, # 512,
        n_epochs=4,
        gamma=0.94,
        learning_rate=1e-4, # lr_schedule,
        clip_range=0.1, # clip_range_schedule,
        tensorboard_log=None,
        seed=1,
        other_learning_rate=1e-4, # other_lr_schedule,
    )
# Load the state dict into the model
model.set_parameters(loaded_model['kwargs']['agent_dict'])

# Function to evaluate the model
def evaluate_model(model: PTCGPolicy, env: PTCGEnv, num_episodes=1):
    total_rewards = []
    for _ in range(num_episodes):
        observation = env.reset()
        done = False
        episode_reward = 0
        while not done:
            player = env.get_player()
            if player == 0:
                action_masks = env.get_action_masks()
                action_masks = torch.as_tensor(action_masks, dtype=bool, device=model.device)
                observation = torch.tensor([observation])
                action, _ = model.policy.predict(observation, action_masks, deterministic=False)
                action = [action.item(), -1]
                # action = env.sample_valid_action()
                # action = [action, -1]
            else:
                action_p2 = env.sample_valid_action()
                action = [-1, action_p2]
            observation, (reward1, reward2), terminated, truncated, _ = env.step(action)
            done = terminated or truncated
            episode_reward += reward1
            env.visualize_game_state()
            input()
        total_rewards.append(episode_reward)
    
    mean_reward = np.mean(total_rewards)
    std_reward = np.std(total_rewards)
    return mean_reward, std_reward

# Evaluate the loaded model
mean_reward, std_reward = evaluate_model(model, env)

print(f"Mean reward: {mean_reward:.2f} +/- {std_reward:.2f}")

# Close the environment
env.close()
