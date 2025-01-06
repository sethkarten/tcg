# Pokémon Trading Card Game Simulator

This repository contains a Pokémon Trading Card Game (PTCG) simulator implemented in C with Python bindings. The project aims to provide a comprehensive simulation environment for the Pokémon Trading Card Game, allowing for AI training and gameplay analysis.

## Features

- **C-based Game Engine**: Core game logic implemented in C for high performance.
- **Python Bindings**: Cython-based Python interface for easy integration with machine learning frameworks.
- **Gymnasium Environment**: Implements the OpenAI Gymnasium interface for reinforcement learning.
- **Self-Play Training**: Supports training AI agents through self-play using PPO algorithm.
- **Custom Transformer Policy**: Utilizes a transformer-based policy for action selection.
- **Deck Building**: Supports custom deck building and loading from CSV files.
- **Extensible Card Database**: JSON-based card database for easy addition of new cards and sets.

## Setup

### Prerequisites

- C compiler (GCC recommended)
- Python 3.7+
- pip (Python package manager)

### Installation

1. Clone the repository:

```
git clone https://github.com/sethkarten/tcg.git
cd tcg
```

2. Install the required Python packages:

```
pip install -r requirements.txt
```

3. Compile the C extension:

```
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/path/to/your/repo/simulator
chmod +x compile.sh
./compile.sh
```

## Usage

### Running the Simulator

To run a basic simulation:

```
import gymnasium as gym
import ptcg env = gym.make('PTCG-v0')
observation, info = env.reset() for _ in range(1000):
action = env.action_space.sample() # Replace with your agent's action
observation, reward, terminated, truncated, info = env.step(action)

if terminated or truncated:
    observation, info = env.reset()
```

### Training an AI Agent

To train an AI agent using self-play:

```
python train.py
```

This script uses the PPO algorithm with a custom transformer policy for self-play training.

## Project Structure

- `simulator/`: C source files for the core game engine
- `cy_ptcg.pyx`: Cython interface between C and Python
- `ptcg.py`: Python wrapper implementing the Gymnasium interface
- `train.py`: Script for training AI agents using self-play
- `test.py`: Test script for measuring simulator performance
- `setup.py`: Build script for compiling the C extension

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

This code builds on the work from two important papers in the field of competitive Pokémon AI and multi-agent reinforcement learning:

1. PokéChamp: an expert-level minimax language agent for competitive Pokémon [@karten2024pokechamp]
2. FightLadder: A Benchmark for Competitive Multi-Agent Reinforcement Learning [@li2024fightladder]

These papers provide the foundation for the advanced AI techniques and competitive frameworks used in this project.

```
@inproceedings{karten2024pokechamp,
  title={Pok{\'e}Champ: an Expert-level Minimax Language Agent for Competitive Pok{\'e}mon},
  author={Karten, Seth and Nguyen, Andy Luu and Jin, Chi},
  booktitle={Language Gamification-NeurIPS 2024 Workshop}
}
```

```
@article{li2024fightladder,
  title={FightLadder: A Benchmark for Competitive Multi-Agent Reinforcement Learning},
  author={Li, Wenzhe and Ding, Zihan and Karten, Seth and Jin, Chi},
  journal={arXiv preprint arXiv:2406.02081},
  year={2024}
}
```

