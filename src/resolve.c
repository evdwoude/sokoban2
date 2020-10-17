#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "sokoban2.h"
#include "error.h"
#include "resolve_int.h"
#include "position.h"
#include "utility.h"
#include "resolve.h"

/* Local defs */

/* Internal protos */

/* Exported data */

/* Imported data */

/* Local data */

/* Code */


skbn_err resolve(p_game_data_t p_game_data)
{
    p_spot johnny = p_game_data->johnny;

    int32_t tmp_move_index; // TODO: Remove this ? Check out what to initiale the first move_path to.
    int32_t *move_index = &tmp_move_index;

    /* Define all hard no go spots. Spots that we never should put a box on. */
    define_hardnogos(p_game_data);

    /* Using the defined hard nogos, create the linked list of spots that form the base of the positions. */
    create_position_base(p_game_data);

    /* Create a root node for the forward position tree */
    p_game_data->position_root = new_position_node(p_game_data);

    /* Add the setup position to the position tree. */
    find_or_add_position(p_game_data, FORWARD , &move_index);

    // walk_to_extend_depth(p_game_data, johnny);
    return no_error;
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
//     for spots on reach list:
//         for movable boxes from spot
//             add move
// }

void explore_for_reach(p_game_data_t p_game_data, p_spot johnny, int search_dir)
{
    t_direction dir = right;
    int mark;
    struct spot *neighbour;
    struct spot *end;

    if (!johnny)
        return;

    mark = next_mark(p_game_data);
    johnny->reach_mark  = mark;
    johnny->explore_reach_list = NULL;
    end = johnny;

    /* Build up a list of spots to visit, while at the same time walk down that list. */
    while (johnny)
    {
        /* Insert all applicable neighbours into the list: */
        for (dir=right; dir<=down; dir++)
        {
            neighbour = johnny->neighbour[dir];

            if (    neighbour                                                 /* Neighbour spot exists? */
                &&  !(search_dir ? neighbour->is_target : neighbour->has_box) /* Is that spot empty?    */
                &&  neighbour->reach_mark < mark )                            /* Not yet explored?      */
            {
                /* Mark it: */
                neighbour->reach_mark  = mark;

                /* Add it to end of list: */
                end->explore_reach_list = neighbour;
                end                     = neighbour;
                end->explore_reach_list = NULL;
            }
        }
        /* Move down the list until done: */
        johnny = johnny->explore_reach_list;
    }
    return;
}
