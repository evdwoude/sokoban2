#ifndef MOVEH
#define MOVEH

t_bool test_move(p_game_data_t p_game_data, p_spot johnny, t_direction mv_dir, t_s_dir search_dir);

void make_move(p_game_data_t p_game_data, p_spot johnny, t_direction mv_dir, t_mv_action action, t_s_dir search_dir);

#endif /* MOVEH */
