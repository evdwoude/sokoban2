#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "sokoban2.h"
#include "error.h"
#include "resolve_int.h"
#include "utility.h"
#include "position.h"


/* Local defs */

/* Conditional compile switches  */

#define DBG_CREATE_BASE 1       /* Debug prints on creating the trasnposition base. */
#define DBG_ADD_TP 1            /* Prints on add_postion function                   */

/* Conditional compile 'fabric' */

#ifdef DBG_ADD_TP
#define printf_add_tp(...) printf(__VA_ARGS__);
#else
#define printf_add_tp(...)
#endif


/* Internal protos */


t_outcome_add_tp find_or_add_box_arrangement(p_game_data_t p_game_data, int search_dir, uint32_t *p_leaf);

t_outcome_add_tp find_or_add_reach(p_game_data_t p_game_data, int search_dir, uint32_t reach, uint32_t *p_leaf);

int find_reach_identifier(p_game_data_t p_game_data, p_spot johnny, int search_dir);

/* Exported data */

/* Imported data */

/* Local data */

/* Code */

void create_position_base(p_game_data_t p_game_data)
{
    struct spot* spot;
    struct spot** p_end;

    p_end = &(p_game_data->position_head);
    for (spot = &(p_game_data->spot_pool[0]); spot < p_game_data->pool_ptr; spot++)
        if ( ! is_hardnogo(spot) )
        {
            *p_end = spot;
            p_end = &(spot->position_list);
        }
    *p_end = NULL;

#ifdef DBG_CREATE_BASE
    printf("Position spots:\n");
    for(spot = p_game_data->position_head; spot; spot = spot->position_list)
        printf(" %ld", SPOT_NUMBER(spot));
    printf("\n\n");
#endif /* DBG_CREATE_BASE */
}


/* Potential outcomes:
 *  - Position is new.
 *  - Position exists already on the same search direction.
 *  - Position exists for the opposite search direction: Bingo. Solution found.
 *  - Error finding a new memory block for a position node or leaf.
 */

t_outcome_add_tp find_or_add_position(p_game_data_t p_game_data, int search_dir, int32_t **move_path)
{
    t_outcome_add_tp outcome = position_exists_on_same_direction; /* The outcome of this function. */

    uint32_t leaf = 0;    /* The new leaf. Polulate with reach and move_path. */
    uint32_t reach = 0;   /* Reach_identifier of the currents posiiton. */

    reach = find_reach_identifier(p_game_data, p_game_data->johnny, search_dir);

    if ( ! move_path ) /* Sanity check: Do not accept this pointer to be not populated. */
        exit( print_error(no_move_path_ref_ref_given) );

    /* Figure out whether this is new box arrangement. If so, add it to the tree. */
    outcome =  find_or_add_box_arrangement(p_game_data, search_dir, &leaf);

    /* Now leaf refers to a position leaf, either new or pre-existing. */
    printf_add_tp("  >  ");
    if (outcome != position_is_new)
    {
        /* Box arrangement is known.
         * Now figure out whether this is a new reach. If so, add it to the list. */
        outcome =  find_or_add_reach(p_game_data, search_dir, reach, &leaf);
        if (outcome != position_is_new)
        {
            /* The whole position is known. Either bingo or known (in same direction). */
            printf_add_tp(outcome == bingo ? "  !!" : "  --");
            *move_path = &(P_TPL(leaf)->move_path); /* Return the (opposite) move_path in case of bingo. */
            return outcome;
        }
    }
    /* Here the position is new, either due to boxes or reaches. (And leaf refers to the new leaf.)
     * Populate the new leaf with the reach identifier and the seacrh direction */
    P_TPL(leaf)->leaf_data = reach | (search_dir ? TPL_BACKWARD : TPL_FORWARD);
    printf_add_tp(" N_%03d_%d", leaf, reach);

    *move_path = &(P_TPL(leaf)->move_path); /* Return move_path reference for population by caller. */
    return position_is_new;
}


t_outcome_add_tp find_or_add_box_arrangement(p_game_data_t p_game_data, int search_dir, uint32_t *p_leaf)
{
    t_outcome_add_tp outcome = position_exists_on_same_direction; /* First assume not new. */
    uint32_t node_or_leaf    = p_game_data->position_root;        /* Walks down the tree. */

    uint32_t *next; /* Reference of the current node's child reference. */
    p_spot  spot;   /* Iterates over the position base.                 */

    for(spot = p_game_data->position_head; spot; spot = spot->position_list)
    {
        /* Check whether the spot is populated or not and follow/populate the related next pointer.
         * Check for boxes of targets depending on the search direction. Forward: check for boxes. */
        if (search_dir ? spot->is_target : spot->has_box)
            next = &(P_TPN(node_or_leaf)->spot_has_box);
        else
            next = &(P_TPN(node_or_leaf)->spot_is_empty);

        printf_add_tp("%c_%03d%c ",
                spot->has_box ?  (spot->is_target ? 'H' : 'X') : (spot->is_target ? '.' : 'Z'),
                node_or_leaf, *next ? '-' : '+');

        if ( ! *next )
        {
            outcome = position_is_new; /* There is no next node, so this is a new postion. */

            /* Create new node (or leaf), and hook it up to current node. */
            if (spot->position_list)
                *next = new_position_node(p_game_data); /* Not end of spot list: create node. */
            else
                *next = new_position_leaf(p_game_data); /* End of spot list: create leaf */
        }
        node_or_leaf = *next; /* Goto the next node. If there was no next node yet, we've just created one. */
    }

    *p_leaf = node_or_leaf; /* Return the leaf we're at, whether it's new or not. */
    return outcome;
}


t_outcome_add_tp find_or_add_reach(p_game_data_t p_game_data, int search_dir, uint32_t reach, uint32_t *p_leaf)
{
    uint32_t leaf = *p_leaf;   /* Walks down the leaf list, starting at the given leaf. */
    uint32_t last_leaf = leaf; /* Remind the last leaf for adding a new leaf (if applicable). */

    /* Walk down the leaf list until we find a matching reach identifier. */
    while (leaf) /* Note: this loops enters at least once. */
    {
        printf_add_tp(" L_%03d", leaf);

        if ( reach == TPL_REACH(P_TPL(leaf)->leaf_data) )  /* Is position known? */
        {
            if ( ! P_TPL(leaf)->move_path ) /* Sanity check. */
                exit( print_error(move_path_not_polulated) );

            *p_leaf = leaf; /* Return the found leaf. */

            if ( search_dir != TPL_SEARCH_DIR(P_TPL(leaf)->leaf_data) ) /* Opposing search direction? */
                return bingo;
            else
                return position_exists_on_same_direction;
        }
        last_leaf = leaf;
        leaf = P_TPL(leaf)->next_leaf; /* Go to the next leaf. */
    }
    /* The reach appears to be new: create a new leaf for this reach and hook it up to the list. */
    P_TPL(last_leaf)->next_leaf = new_position_leaf(p_game_data);

    *p_leaf = P_TPL(last_leaf)->next_leaf; /* Return the new leaf for further polulation. */
    return position_is_new;
}


int find_reach_identifier(p_game_data_t p_game_data, p_spot johnny, int search_dir)
{
    t_direction dir = right;
    int reach_identifier;
    int mark;
    struct spot *neighbour;
    struct spot *end;

    if (!johnny)
        return 0;

    reach_identifier = SPOT_NUMBER(johnny);
    mark = next_mark(p_game_data);
    johnny->reach_mark  = mark;
    johnny->other_reach_list = NULL;
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
                /* Mind the lowest spot number we found so far. */
                if (SPOT_NUMBER(neighbour) < reach_identifier)
                    reach_identifier = SPOT_NUMBER(neighbour);

                /* Mark it: */
                neighbour->reach_mark  = mark;

                /* Add it to end of list: */
                end->other_reach_list = neighbour;
                end                   = neighbour;
                end->other_reach_list = NULL;
            }
        }
        /* Move down the list until done: */
        johnny = johnny->other_reach_list;
    }
    return reach_identifier;
}





