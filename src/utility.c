#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "sokoban2.h"
#include "error.h"
#include "resolve_int.h"
#include "utility.h"

/* Local defs */

/* Conditional compile switches  */

#define DBG_MARK 1    // Prints marker wrappings
#define DBG_NOGO 1    // Prints everything related to no-go's.
//#define DBG_C 1       // Prints
//#define DBG_D 1       // Prints

/* Conditional compile 'fabric' */


#ifdef DBG_MARK
#define printf_mark(...) printf(__VA_ARGS__);
#else
#define printf_mark(...)
#endif

#ifdef DBG_NOGO
#define printf_nogo(...) printf(__VA_ARGS__);
#define if_nogo(condition) if(condition)
#else
#define printf_nogo(...)
#define if_nogo(condition)
#endif


/* Internal protos */

/* Exported data */

/* Imported data */

/* Local data */

/* Code */


/* int next_mark(struct game_data *p_game_data)
 *
 * Used for explore functions.
 *
 * Increments the global spot mark to mark spot as newly found during a current eploration. This way,
 * the spot marks do not need to be re-initialized every new eploration. The reinitilization is still
 * required (and automatically performed) when the globla spot mark wraps. Reinitilization is also
 * required before searching for the amount of domains a backward search can start from.
 */
int next_mark(struct game_data *p_game_data)
{
    p_spot spot;

    p_game_data->next_reach++;
    if (p_game_data->next_reach > 3)
    {
        printf_mark("\nReinit marks...")

        /* If next_reach wrapped, reinitiae all spots. */
        for (spot = &(p_game_data->spot_pool[0]); spot < p_game_data->pool_ptr; spot++)
        {
            spot->reach_mark = 0;
            printf_mark(" %d", spot->spot_number)
        }
        printf_mark("\n")

        /* And reinitialize next_reach as zero + 1. */
        p_game_data->next_reach = 1;
    }

    return p_game_data->next_reach;
}


/* void  define_no_go_s(struct game_data *p_game_data)
 *
 * Find and mark all spots where placing a box would always and immediately result in a dead end.
 * This includes the dead ends that result from individual boxes put on specific, spots, identifiable
 * from the static setup (i.e. the start position).
 * It excludes all dead ends that involve multiple boxes. These dead ends are more complex to find and
 * can't be idenfified up-front (staticallly), but only on-the-go, which is quite expensive.
 */
void define_no_go_s(struct game_data *p_game_data)
{
    struct spot* spot;
    struct spot* scan;
    int on_the_edge;

    printf_nogo("\n\nCorners:")

    for (spot = &(p_game_data->spot_pool[0]); spot < p_game_data->pool_ptr; spot++)
    {
        if (    !spot->is_target
            && (    (spot->neighbour[right] == NULL  &&  spot->neighbour[up   ] == NULL)
                ||  (spot->neighbour[up   ] == NULL  &&  spot->neighbour[left ] == NULL)
                ||  (spot->neighbour[left ] == NULL  &&  spot->neighbour[down ] == NULL)
                ||  (spot->neighbour[down ] == NULL  &&  spot->neighbour[right] == NULL) ) )
        {
            spot->no_go = 1;

            printf_nogo(" %d", spot->spot_number)
        }
    }

    printf_nogo("\nEdges:  ")

    for (spot = &(p_game_data->spot_pool[0]); spot < p_game_data->pool_ptr; spot++)
    {
        if (spot->neighbour[left] == NULL)
        {
            on_the_edge = 1;

            for (scan = spot; scan && on_the_edge; scan = scan->neighbour[right])
                if (scan->is_target || (scan->neighbour[up] != NULL && scan->neighbour[down] != NULL))
                    on_the_edge = 0;

            if (on_the_edge)
                for (scan = spot; scan; scan = scan->neighbour[right])
                {
                    if_nogo(!scan->no_go)
                        printf_nogo(" %d", scan->spot_number)

                    scan->no_go = 1;

                }
        }
        if (spot->neighbour[up] == NULL)
        {
            on_the_edge = 1;

            for (scan = spot; scan && on_the_edge; scan = scan->neighbour[down])
                if (scan->is_target || (scan->neighbour[right] != NULL && scan->neighbour[left] != NULL))
                    on_the_edge = 0;

            if (on_the_edge)
                for (scan = spot; scan; scan = scan->neighbour[down])
                {
                    if_nogo(!scan->no_go)
                        printf_nogo(" %d", scan->spot_number)

                    scan->no_go = 1;

                }
        }
    }

    printf_nogo("\n\n")
}
