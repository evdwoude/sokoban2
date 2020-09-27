#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "sokoban2.h"
#include "error.h"
#include "resolve_int.h"
#include "transposition.h"
#include "utility.h"
#include "resolve.h"

/* Local defs */

// typedef enum {false = 0, true = 1} t_bool;

/* Internal protos */

/* Exported data */

/* Imported data */

/* Local data */

/* Code */

void  resolve(struct game_data *p_game_data)
{
    p_spot johnny = p_game_data->johnny;

    /* First, define all "no go spots", and chain up the rest for creating transpositions. */
    define_hardnogos(p_game_data);

    /* Then, using the defined hard nogos, create the linked list of spots that are used for the creation
     * of the as transpositions. */
    create_transposition_base(p_game_data);

    // walk_to_extend_depth(p_game_data, johnny);

    return;
}

// t_got_it walk_to_extend_depth(struct game_data *p_game_data, p_spot johnny, move_stack)
// {
//     p_spot johnny start_position = johnny;
//
//     do
//     {
//         while (child available)
//             descend();
//
//         if (no dead end yet)
//             if (extend_depth() == got_it)
//                 return got_it;
//
//         if (sibbling)
//             go to sibbling
//         else
//             ascend();
//     }
//     while (johnny is not back at start_position yet)
//
//     return not_yet;
// }
//
// t_got_it extend_depth()
// {
//     Pseudo:
//     explore_for_reach()
//     for spots on reach chain:
//         for movable boxes from spot
//             add move
// }

void explore_for_reach(struct game_data *p_game_data, p_spot johnny)
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
                end->reach_chain = NULL;            /* Keep the chain end up to date, as we're
                                                        using the chain while building it up. */
            }
        }

         /* Move down the chain until done: */
        johnny = johnny->reach_chain;
    }

    return;
}
