#ifndef BLOCKERSH
#define BLOCKERSH

void define_hardnogos(p_game_data_t p_game_data);
int is_hardnogo(struct spot* spot);
// bool is_fw_blocker(p_spot spot, t_mv_dir dir);
bool is_fw_blocker(p_spot spot, t_mv_dir dir, p_game_data_t p_game_data);

#endif /* BLOCKERSH */
