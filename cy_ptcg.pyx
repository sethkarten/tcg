# cy_ptcg.pyx

cimport cython
from libc.stdlib cimport malloc, free
from libc.string cimport strcpy, memcpy

cdef extern from "simulator/ptcg.h":
    ctypedef struct GameState:
        pass
    
    void init(GameState * game)
    void reset(GameState *game, const char player1_deck[20][50], const int player1_energy[11], const char player2_deck[20][50], const int player2_energy[11])
    int* get_legal_actions(GameState *game)
    int execute_action(GameState *game, int action, int target, int opponent_target)
    int* get_valid_targets(GameState *game, int action)
    int* get_valid_opponent_target(GameState *game, int action)
    float* get_observation(GameState *game)
    int is_game_over(GameState *game)
    int get_current_player(GameState *game)

cdef class CyPTCG:
    cdef GameState* game
    cdef char player1_deck[20][50]
    cdef char player2_deck[20][50]
    cdef int player1_energy[11]
    cdef int player2_energy[11]
    cdef int action_

    def __cinit__(self):
        self.game = <GameState*>malloc(sizeof(GameState))
        if self.game is NULL:
            raise MemoryError()
        init(self.game)

    def __dealloc__(self):
        if self.game is not NULL:
            free(self.game)

    def get_player(self):
        return get_current_player(self.game)

    def reset(self, player1_deck, player1_energy, player2_deck, player2_energy):
        for i in range(20):
            strcpy(self.player1_deck[i], player1_deck[i].encode('utf-8'))
            strcpy(self.player2_deck[i], player2_deck[i].encode('utf-8'))
        
        for i in range(11):
            self.player1_energy[i] = player1_energy[i]
            self.player2_energy[i] = player2_energy[i]

        reset(self.game, self.player1_deck, self.player1_energy, self.player2_deck, self.player2_energy)

    def get_actions_available(self):
        cdef int* legal_actions = get_legal_actions(self.game)
        actions = [bool(legal_actions[i]) for i in range(97)]
        free(legal_actions)
        return actions

    def get_targets(self, action):
        action_ = int(action)
        cdef int* legal_targets_ = get_valid_targets(self.game, action_)
        cdef int* legal_targets_opp_ = get_valid_opponent_target(self.game, action_)
        legal_targets = [bool(legal_targets_[i]) for i in range(8)]
        legal_targets_opp = [bool(legal_targets_opp_[i]) for i in range(8)]
        free(legal_targets_)
        free(legal_targets_opp_)
        return legal_targets, legal_targets_opp

    def step(self, action, target, opponent_target):
        reward = execute_action(self.game, action, target, opponent_target)
        return reward

    def get_observation(self):
        cdef float* obs = get_observation(self.game)
        observation = [obs[i] for i in range(91)]
        free(obs)
        return observation

    def is_game_over(self):
        return is_game_over(self.game)
