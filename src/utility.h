#ifndef UTILITYH
#define UTILITYH

/* Include:
 *      sokoban2.h
            struct game_data
 */

typedef enum {not_yet = 0, got_it = 1} t_got_it;
typedef enum {false = 0, true = 1} t_bool;

int next_mark(p_game_data_t p_game_data);

void define_hardnogos(p_game_data_t p_game_data);

int is_hardnogo(struct spot* spot);

#endif /* UTILITYH */
