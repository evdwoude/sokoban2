#ifndef RESOLVEH
#define RESOLVEH

/* Include:
 *      sokoban2.h:
 *          struct game_data
 *          p_spot
 */

skbn_err resolve(struct game_data *p_game_data);
void explore_for_reach(struct game_data *p_game_data, p_spot johnny, int search_dir);

#endif /* RESOLVEH */
