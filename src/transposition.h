#ifndef TRANSPOSITIONH
#define TRANSPOSITIONH

typedef enum {
    transposition_exists_on_same_direction,
    transposition_is_new,
    bingo
} t_outcome_add_tp;

void create_transposition_base(p_game_data_t p_game_data);

t_outcome_add_tp find_or_add_transposition(p_game_data_t p_game_data, int search_dir, int32_t **move_index);


#endif /* TRANSPOSITIONH */
