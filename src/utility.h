#ifndef UTILITYH
#define UTILITYH

/* Include:
 *      sokoban2.h
            struct game_data
 */


typedef enum {not_yet = 0, got_it = 1} t_got_it;
typedef enum {false = 0, true = 1} t_bool;

/* int next_mark(struct game_data *p_game_data)
 *
 * Used for explore functions.
 *
 * Increments the global spot mark to mark spot as newly found during a current eploration. This way,
 * the spot marks do not need to be re-initialized every new eploration. The reinitilization is still
 * required (and automatically performed) when the globla spot mark wraps. Reinitilization is also
 * required before searching for the amount of domains a backward search can start from.
 */
int next_mark(struct game_data *p_game_data);

/* void  define_no_go_s(struct game_data *p_game_data)
 *
 * Find and mark all spots where placing a box would allways and immediately result in a dead end.
 * This includes dead ends that result from individual boxes put on specific spots. These spots are
 * identifyable from the static setup (i.e. the start position).
 * It excludes all dead ends that involve multiple boxes. These dead ends are much more complex to
 * define up-front and finding them on-the-go is very expensive.
 */
void define_hardnogos(struct game_data *p_game_data);


int is_hardnogo(struct spot* spot);

#endif /* UTILITYH */
