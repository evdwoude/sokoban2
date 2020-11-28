#ifndef MOVEH
#define MOVEH

bool test_move(p_game_data_t p_game_data, p_spot johnny, t_mv_dir mv_dir, t_s_dir search_dir);

void make_move(p_game_data_t p_game_data, p_spot johnny, t_mv_dir mv_dir, t_s_dir search_dir);

void take_back(p_game_data_t p_game_data, p_spot johnny, t_mv_dir mv_dir, t_s_dir search_dir);

#endif /* MOVEH */
