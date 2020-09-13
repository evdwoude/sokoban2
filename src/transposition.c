#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "sokoban2.h"
#include "error.h"
#include "resolve_int.h"
#include "utility.h"
#include "transposition.h"

/* Local defs */

// typedef enum {false = 0, true = 1} t_bool;

/* Internal protos */

/* Exported data */

/* Imported data */

/* Local data */

/* Code */

#if 0
skbn_err add_transposition(p_spot johhny)
{
    p_transposition_node  node = &first_node;
    p_transposition_node  *next;
    p_transposition_node  new_node;
    p_spot  spot;
    skbn_err  error;

    int new_transposition = 0;

    for (spot = &(game_data.spot_pool[0]); spot < game_data.pool_ptr; spot++)
    {
        if (man == spot)
            next = &(node->node.spot_has_man);
        else if (spot->has_box )
            next = &(node->node.spot_has_box);
        else
            next = &(node->node.spot_is_empty);

        dbg_printf_a( man == spot ? "M" : spot->has_box ? "B" : "E" )

        if (!*next)
        {
            /* There is no next node for this 'next' pointer.    */
            /* By definition, this means this is a new transpostion. */

            new_transposition = 1;

            error = get_new_node((void **) &new_node, sizeof(union transposition_node));
            if (error)
                return error;
            transposition_nodes++;

            /* Initialise members. */
            new_node->node.spot_has_box  = NULL;
            new_node->node.spot_has_man  = NULL;
            new_node->node.spot_is_empty = NULL;

            /* Hook the new node up to the current node's 'next' pointer, whichever that is. */
            *next = new_node;

            /* Indicate that this transposition is new. */
            /* We do not break up the loop here, because we need to make the complete transposition known. */
            dbg_printf_a("+ ")
        }
        dbg_else_a
            dbg_printf_a("- ")

        node = *next;   /* Goto the next node. (If there was no next node, we just created one.) */
    }

    if (new_transposition || node->move_path.backward_path)
        *new_tp_node = node;

    return none;
}
#endif