#ifndef POSITIONH
#define POSITIONH

typedef enum {
    position_exists_on_same_direction,
    position_is_new,
    bingo
} t_outcome_add_tp;

void create_position_base(p_game_data_t p_game_data);

t_outcome_add_tp find_or_add_position(p_game_data_t p_game_data, t_s_dir search_dir, uint32_t **move_path);


#endif /* POSITIONH */
