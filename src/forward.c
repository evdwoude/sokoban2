#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "sokoban2.h"
#include "error.h"
#include "resolve_int.h"
#include "forward.h"

/* Local defs */

// typedef enum {false = 0, true = 1} t_bool;

/* Internal protos */

int next_mark(struct game_data *p_game_data);

/* Exported data */

/* Imported data */

/* Local data */

/* Code */

void explore_reach(struct game_data *p_game_data, p_spot johnny)
{
    skbn_err error = no_error;
    t_direction dir = right;
    int mark;
    struct spot *neighbour;
    struct spot *end;

     if (!johnny)
         return;

    mark = next_mark(p_game_data);
    johnny->reach_mark  = mark;
    johnny->reach_chain = NULL;
    end = johnny;

    while (johnny)
    {
        /* Insert all applicable neighbours into the chain: */
        for (dir=right; dir<=down; dir++)
        {
            neighbour = johnny->neighbour[dir];

            if (    neighbour                       /* Is there a neighbouring spot?        */
                &&  !neighbour->has_box             /* Is it empty, i.e. is there no box?   */
                &&  neighbour->reach_mark < mark )  /* Not explored already this time?      */
            {
                /* Mark it: */
                neighbour->reach_mark  = mark;

                /* Chain it: */
                end->reach_chain = neighbour;
                end              = neighbour;
                end->reach_chain = NULL;
            }
        }

         /* Move down the chain until done: */
        johnny = johnny->reach_chain;
    }

    return;
}

int next_mark(struct game_data *p_game_data)
{
    p_spot spot;

    p_game_data->next_reach++;
    if (p_game_data->next_reach > 3)
    {
        printf("\nReinit ...");

        /* If next_reach wrapped, reinitiae all spots. */
        for (spot = &(p_game_data->spot_pool[0]); spot < p_game_data->pool_ptr; spot++)
        {
            spot->reach_mark = 0;
            printf(" %d", spot->spot_number);
        }
        printf("\n");

        /* And reinitialize next_reach as zero + 1. */
        p_game_data->next_reach = 1;
    }

    return p_game_data->next_reach;
}
