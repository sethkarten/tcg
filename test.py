import gymnasium as gym
import ptcg
import time
import csv

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

def measure_fps(env, num_steps=1000):
    start_time = time.time()
    
    observation, info = env.reset()
    for _ in range(num_steps):
        action = env.sample_valid_action()  # Random action
        observation, reward, terminated, truncated, info = env.step(action)
        # print('terminated', terminated)
        # print('truncated', truncated)
        if terminated or truncated:
            observation, info = env.reset()
            # break
    
    end_time = time.time()
    fps = num_steps / (end_time - start_time)
    return fps

# Load decks from CSV
decks, energies = load_decks_from_csv('decks.csv')

# Create the environment
env = gym.make('PTCG-v0')

# Set decks for both players
env.reset(options={
    'player1_deck': decks[0],
    'player1_energy': energies[0],
    'player2_deck': decks[1],
    'player2_energy': energies[1]
})

# Measure FPS
fps = measure_fps(env)

print(f"Simulator FPS: {fps:.2f}")

# Close the environment
env.close()
