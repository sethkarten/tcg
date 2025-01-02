# cy_ptcg.pyx

cimport cython
from libc.stdlib cimport malloc, free
from libc.string cimport strcpy, memcpy

cdef extern from "simulator/ptcg.h":
    ctypedef struct GameState:
        pass
    
    void reset_game(GameState *game, const char player1_deck[20][50], const bint player1_energy[11], const char player2_deck[20][50], const bint player2_energy[11])
    bint* get_legal_actions(GameState *game, int *actions)
    int execute_action(GameState *game, int action, int target, int opponent_target)
    float* get_observation(GameState *game)
    bint is_game_over(GameState *game)

cdef class CyPTCG:
    cdef GameState* game
    cdef char player1_deck[20][50]
    cdef char player2_deck[20][50]
    cdef bint player1_energy[11]
    cdef bint player2_energy[11]

    def __cinit__(self):
        self.game = <GameState*>malloc(sizeof(GameState))
        if self.game is NULL:
            raise MemoryError()

    def __dealloc__(self):
        if self.game is not NULL:
            free(self.game)

    def reset(self, player1_deck, player1_energy, player2_deck, player2_energy):
        for i in range(20):
            strcpy(self.player1_deck[i], player1_deck[i].encode('utf-8'))
            strcpy(self.player2_deck[i], player2_deck[i].encode('utf-8'))
        
        for i in range(11):
            self.player1_energy[i] = player1_energy[i]
            self.player2_energy[i] = player2_energy[i]

        reset_game(self.game, self.player1_deck, self.player1_energy, self.player2_deck, self.player2_energy)

    def get_legal_actions(self):
        cdef bint* legal_actions = get_legal_actions(self.game, NULL)
        actions = [bool(legal_actions[i]) for i in range(95)]
        free(legal_actions)
        return actions

    def step(self, action, target, opponent_target):
        reward = execute_action(self.game, action, target, opponent_target)
        return reward

    def get_observation(self):
        cdef float* obs = get_observation(self.game)
        observation = [obs[i] for i in range(256)]
        free(obs)
        return observation

    def is_game_over(self):
        return is_game_over(self.game)
