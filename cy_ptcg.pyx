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
    int execute_action(GameState *game, int action)
    float* get_observation(GameState *game)
    int is_game_over(GameState *game)
    int get_current_player(GameState *game)
    void set_seed_(int seed)
    char** get_player_hand(GameState* game, int player, int* size)
    int get_player_deck_count(GameState* game, int player)
    char* get_player_active(GameState* game, int player)
    char** get_player_bench(GameState* game, int player, int* size)
    int get_player_prizes(GameState* game, int player)

cdef class CyPTCG:
    cdef GameState* game
    cdef char player1_deck[20][50]
    cdef char player2_deck[20][50]
    cdef int player1_energy[11]
    cdef int player2_energy[11]
    cdef int action_
    cdef int seed_

    def __init__(self):
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
        actions = [bool(legal_actions[i]) for i in range(687)]
        free(legal_actions)
        return actions

    def step(self, action):
        player_idx = self.get_player()
        cdef int _reward = execute_action(self.game, action)
        reward = int(_reward)
        reward_other = 0
        if (abs(reward) == 100): reward_other = -reward
        if (player_idx == 0):
            rewards = [reward, reward_other]
        else:
            rewards = [reward_other, reward]
        return rewards

    def get_observation(self):
        cdef float* obs = get_observation(self.game)
        observation = [obs[i] for i in range(92)]
        free(obs)
        return observation

    def is_game_over(self):
        return is_game_over(self.game)

    def set_seed(self, seed):
        seed_ = int(seed)
        set_seed_(seed)

    def get_player_hand(self, player):
        cdef int size = 0
        cdef const char** hand_ptr = get_player_hand(self.game, player, &size)
        return [hand_ptr[i].decode('utf-8') for i in range(size) if hand_ptr[i] != b'']

    def get_player_deck_count(self, player):
        return get_player_deck_count(self.game, player)

    def get_player_active(self, player):
        cdef const char* active_ptr = get_player_active(self.game, player)
        active = active_ptr.decode('utf-8') if active_ptr != NULL else None
        free(active_ptr)
        return active

    def get_player_bench(self, player):
        cdef int size = 0
        cdef char** bench_ptr = get_player_bench(self.game, player, &size)
        bench = [bench_ptr[i].decode('utf-8') for i in range(size)]
        for i in range(size):
            free(bench_ptr[i])
        free(bench_ptr)
        return bench

    def get_player_prizes(self, player):
        return get_player_prizes(self.game, player)

    def __reduce__(self):
        # Return a tuple of (callable, args) that can recreate the object
        return (CyPTCG, ())

    def __setstate__(self, state):
        # Reinitialize the object after unpickling
        self.__init__()

