# ptcg.py

import Cython
import gymnasium as gym
from gymnasium import spaces
import numpy as np
from cy_ptcg import CyPTCG
import csv
import random
from copy import copy

def load_decks_from_csv(file_path):
    decks = []
    energies = []
    with open(file_path, 'r') as csvfile:
        reader = csv.reader(csvfile)
        for row in reader:
            if len(row) == 31:  # 20 cards + 11 energy types
                decks.append(row[:20])
                energies.append([bool(int(e)) for e in row[20:]])
    return decks, energies


class PTCGEnv(gym.Env):
    metadata = {'render_modes': ['human']}

    def __init__(self, render_mode=None, decks_file='decks.csv'):
        self.cy_ptcg = CyPTCG()
        
        self.action_space = spaces.Discrete(696)  # action, target, opponent_target
        self.observation_space = spaces.Box(low=-5, high=5, shape=(92,), dtype=np.float32)

        self.render_mode = render_mode
        self.decks, self.energies = load_decks_from_csv(decks_file)

    def sample_valid_action(self):
        # get legal actions
        legal_actions = self.cy_ptcg.get_actions_available()
        # print('legal actions')
        # print(legal_actions)
        # Sample action type
        valid_action_types = [i for i, x in enumerate(legal_actions[:696]) if x]
        if len(valid_action_types) > 0:
            action_type = np.random.choice(valid_action_types)
            # print("valid actions",valid_action_types, action_type)
        else:
            # print("no valid actions", legal_actions, flush=True)
            action_type = 686
        return action_type
        
        # legal_targets, legal_targets_opp = self.cy_ptcg.get_targets(action_type)
        # # Sample target
        # legal_targets_types = [i for i, x in enumerate(legal_targets[:8]) if x]
        # # print('targets',legal_targets_types)
        # if len(legal_targets_types) > 0:
        #     target = np.random.choice(legal_targets_types)
        # else:
        #     target = -1
        # # target = np.random.randint(8)
        
        # # Sample opponent target
        # legal_targets_opp_types = [i for i, x in enumerate(legal_targets_opp[:8]) if x]
        # if len(legal_targets_opp_types) > 0:
        #     opponent_target = np.random.choice(legal_targets_opp_types)
        # else:
        #     opponent_target = -1
        # # opponent_target = np.random.randint(8)
        
        # return [action_type, target, opponent_target]


    def reset(self, seed=None, options=None):
        super().reset(seed=seed)
        # print('reseting')
        
        # Randomly select two decks and their corresponding energies
        deck1_index = random.randint(0, len(self.decks) - 1)
        deck2_index = random.randint(0, len(self.decks) - 1)
        
        player1_deck = self.decks[deck1_index]
        player2_deck = self.decks[deck2_index]
        player1_energy = self.energies[deck1_index]
        player2_energy = self.energies[deck2_index]
        # print("Loaded player decks",player1_deck, player2_deck)

        self.cy_ptcg.reset(copy(player1_deck), copy(player1_energy), copy(player2_deck), copy(player2_energy))
        observation = self.cy_ptcg.get_observation()
        # print("", flush=True)
        info = {}  # You can add any additional info here
        observation = np.array(observation, dtype=np.float32)
        # print("successfully received observation", flush=True)
        # print(observation, info)
        return observation
    
    def get_player(self):
        return self.cy_ptcg.get_player()

    def step(self, action):
        # print('entering step', action, flush=True)
        action_idx = action[self.get_player()]
        # print(action_idx)
        rewards = self.cy_ptcg.step(action_idx)
        # reward_p1 = rewards[0]
        # reward_p2 = rewards[1]
        # print('c step reward', reward)
        # input('end of step')
        
        # print('obs', observation)
        # print('game is over', self.cy_ptcg.is_game_over())
        terminated = (self.cy_ptcg.is_game_over() == 1)
        truncated = False  # You can implement turn limit if needed
        info = {}  # You can add any additional info here
        observation = self.cy_ptcg.get_observation()
        # print('leaving step', rewards)
        return np.array(observation, dtype=np.float32), (rewards[0], rewards[1]), terminated, truncated, info

    def render(self):
        if self.render_mode == "human":
            # Implement rendering logic here
            pass

    def close(self):
        pass

    def seed(self, _seed=None):
        self._seed = _seed
        if _seed is not None:
            self.cy_ptcg.set_seed(_seed)

    def get_action_masks(self):
        legal_actions = self.cy_ptcg.get_actions_available()
        action_mask = np.zeros((696), dtype=np.float32)
        for i in range(len(legal_actions)):
            action_mask[i] = legal_actions[i]        
        return action_mask
    
    def action_masks(self):
        return self.get_action_masks()
    
    def get_spaces(self):
        return self.observation_space, self.action_space
    

# Register the environment
gym.register(
    id='PTCG-v0',
    entry_point='ptcg:PTCGEnv',
)