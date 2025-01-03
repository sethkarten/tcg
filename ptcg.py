# ptcg.py

import gymnasium as gym
from gymnasium import spaces
import numpy as np
from cy_ptcg import CyPTCG
import csv
import random

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
        
        self.action_space = spaces.MultiDiscrete([97, 8, 8])  # action, target, opponent_target
        self.observation_space = spaces.Box(low=-1, high=1, shape=(91,), dtype=np.float32)

        self.render_mode = render_mode
        self.decks, self.energies = load_decks_from_csv(decks_file)

    def sample_valid_action(self):
        # get legal actions
        legal_actions = self.cy_ptcg.get_actions_available()
        # print('legal actions')
        # print(legal_actions)
        # Sample action type
        valid_action_types = [i for i, x in enumerate(legal_actions[:97]) if x]
        action_type = np.random.choice(valid_action_types)
        
        legal_targets, legal_targets_opp = self.cy_ptcg.get_targets(action_type)
        # Sample target
        legal_targets_types = [i for i, x in enumerate(legal_targets[:8]) if x]
        if len(legal_targets_types) > 0:
            target = np.random.choice(legal_targets_types)
        else:
            target = -1
        # target = np.random.randint(8)
        
        # Sample opponent target
        legal_targets_opp_types = [i for i, x in enumerate(legal_targets_opp[:8]) if x]
        if len(legal_targets_opp_types) > 0:
            opponent_target = np.random.choice(legal_targets_opp_types)
        else:
            opponent_target = -1
        # opponent_target = np.random.randint(8)
        
        return [action_type, target, opponent_target]


    def reset(self, seed=None, options=None):
        super().reset(seed=seed)
        
        # Randomly select two decks and their corresponding energies
        deck1_index = random.randint(0, len(self.decks) - 1)
        deck2_index = random.randint(0, len(self.decks) - 1)
        
        player1_deck = self.decks[deck1_index]
        player2_deck = self.decks[deck2_index]
        player1_energy = self.energies[deck1_index]
        player2_energy = self.energies[deck2_index]

        self.cy_ptcg.reset(player1_deck, player1_energy, player2_deck, player2_energy)
        
        observation = self.cy_ptcg.get_observation()
        info = {}  # You can add any additional info here

        return np.array(observation, dtype=np.float32), info

    def step(self, action):
        action_idx, target, opponent_target = action
        reward = self.cy_ptcg.step(action_idx, target, opponent_target)
        # print('c step reward', reward)
        # input('end of step')
        
        # print('obs', observation)
        # print('game is over', self.cy_ptcg.is_game_over())
        terminated = (self.cy_ptcg.is_game_over() == 1) or (reward == -10)
        truncated = False  # You can implement turn limit if needed
        info = {}  # You can add any additional info here
        if not terminated:
            observation = self.cy_ptcg.get_observation()
        else:
            observation = None
        return np.array(observation, dtype=np.float32), reward, terminated, truncated, info

    def render(self):
        if self.render_mode == "human":
            # Implement rendering logic here
            pass

    def close(self):
        pass

    def seed(self, seed=None):
        pass

# Register the environment
gym.register(
    id='PTCG-v0',
    entry_point='ptcg:PTCGEnv',
)