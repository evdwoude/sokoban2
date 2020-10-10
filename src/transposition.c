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

/* Conditional compile switches  */

#define DBG_CREATE_BASE 1       /* Debug prints on creating the trasnposition base. */
#define DBG_ADD_TP 1            /* Prints on add_transpostion function              */

/* Conditional compile 'fabric' */

#ifdef DBG_ADD_TP
#define printf_add_tp(...) printf(__VA_ARGS__);
#define dbg_else_add_tp else
#else
#define printf_add_tp(...)
#define dbg_else_add_tp
#endif


/* Internal protos */


t_outcome_add_tp find_or_add_box_positioning(p_game_data_t p_game_data, int search_dir, uint32_t *p_leaf);

t_outcome_add_tp find_or_add_reach(p_game_data_t p_game_data, int search_dir, uint32_t reach, uint32_t *p_leaf);


/* Exported data */

/* Imported data */

/* Local data */

/* Code */

void create_transposition_base(p_game_data_t p_game_data)
{
    struct spot* spot;
    struct spot** p_end;

    p_end = &(p_game_data->transposition_head);
    for (spot = &(p_game_data->spot_pool[0]); spot < p_game_data->pool_ptr; spot++)
        if ( ! is_hardnogo(spot))
        {
            *p_end = spot;
            p_end = &(spot->transposition_list);
        }
    *p_end = NULL;

#ifdef DBG_CREATE_BASE
    printf("Transposition spots:\n");
    for(spot = p_game_data->transposition_head; spot; spot = spot->transposition_list)
        printf(" %ld", SPOT_NUMBER(spot));
    printf("\n\n");
#endif /* DBG_CREATE_BASE */
}


/* Potential outcomes:
 *  - Transposition is new.
 *  - Transposition exists already on the same search direction.
 *  - Transposition exists for the opposite search direction: Bingo. Solution found.
 *  - Error finding a new momery block for a transpisition node or leaf.
 */

t_outcome_add_tp find_or_add_transposition(p_game_data_t p_game_data, int search_dir, int32_t **move_index)
{
    t_outcome_add_tp outcome = transposition_exists_on_same_direction; /* The outcome of this function. */

    uint32_t leaf = 0;           /* The new leaf. Polulate with reach and move_path. */
    uint32_t reach = 0;   /* Reach_identifier of the currents transposiiton. */

    reach = SPOT_NUMBER(p_game_data->johnny); /* TODO: Call explore_for_reach_indent. */


    /* Figure out whether this is new box positioning. If so, add it to the tree. */
    outcome =  find_or_add_box_positioning(p_game_data, search_dir, &leaf);

    /* Here, leaf refers to a tranposition leaf, either new or pre-existing. */
    printf("   >  ");
    if (outcome != transposition_is_new)
    {
        /* So the box positioning is not new. Now figure out whether this is new reach.
         * If so, add it to the list. */
        outcome =  find_or_add_reach(p_game_data, search_dir, reach, &leaf);

        if (outcome != transposition_is_new)
            return outcome; /* Transpisition is known. So bingo or consider as dead end. */
    }


    /* Here the transpisition is new, either due to boxes or reaches. Leaf refers to a new leaf. */
    printf("  __%d__", leaf);
    P_TPL(leaf)->reach_ident = reach; /* Populate the new leaf with its reach identifier */

    /* Return the reference of the leaf's move index for later population by the calling context. */
    if (move_index)
        *move_index = &(P_TPL(leaf)->move_path[search_dir]);

    return transposition_is_new;
}



t_outcome_add_tp find_or_add_box_positioning(p_game_data_t p_game_data, int search_dir, uint32_t *p_leaf)
{
    t_outcome_add_tp outcome = transposition_exists_on_same_direction; /* First assume not new. */

    uint32_t node_and_leaf = p_game_data->transposition_root; /* Walks down te tree. */

    uint32_t *next;             /* Reference of the current node's child index. */
    p_spot  spot;               /* Iterates over the tranposition base. */

    for(spot = p_game_data->transposition_head; spot; spot = spot->transposition_list)
    {
        if (spot->has_box)         /* TODO add a bit of code for backward search; */
            next = &(P_TPN(node_and_leaf)->spot_has_box);
        else
            next = &(P_TPN(node_and_leaf)->spot_is_empty);

        printf_add_tp( spot->has_box ? "X" : "Z" )
        printf("[%02d]", *next);

        if ( ! *next)
        {
            outcome = transposition_is_new; /* There is no next node, so this is a new transpostion. */

            /* Create new node (or leaf), and hook it up to current node. */
            if (spot->transposition_list)
                *next = new_transposition_node(p_game_data); /* Not end of list: creata node. */
            else
                *next = new_transposition_leaf(p_game_data); /* End of list: creata leaf */

            printf_add_tp("+ ")
        }
        dbg_else_add_tp
            printf_add_tp("- ")

        node_and_leaf = *next;   /* Goto the next node. If there was no next node yet, we've just created one. */
    }

    *p_leaf = node_and_leaf; /* Return the leaf we'er at. */
    return outcome;          /* Return the outcomce.      */
}



t_outcome_add_tp find_or_add_reach(p_game_data_t p_game_data, int search_dir, uint32_t reach, uint32_t *p_leaf)
{
    uint32_t leaf = *p_leaf; /* Walks down the leaf list. */
    uint32_t last_leaf = 0;  /* Remind the leaf in order to add a new leaf it (if applicable). */

    while (leaf) /* The box positioning was existing, so look for the reach identifier now. */
    {
        printf(" L[%02d]", leaf);

        if (P_TPL(leaf)->reach_ident == reach)  /* if "known transposition" ... */
        {
            /* Sanity checks: both or none of the pointers populated is a programmong error. */
            if (P_TPL(leaf)->move_path[FORWARD] && P_TPL(leaf)->move_path[BACKWARD]) /* TODO: remove. */
                exit( print_error(both_forward_and_backward_moves) );
            if (!P_TPL(leaf)->move_path[FORWARD] && !P_TPL(leaf)->move_path[BACKWARD]) /* TODO: remove. */
                exit( print_error(no_forward_nor_backward_move) );


            if (P_TPL(leaf)->move_path[search_dir])
                {printf("  --"); return transposition_exists_on_same_direction;}
            else
                {printf("  !!"); return bingo;}
        }
        last_leaf = leaf;
        leaf = P_TPL(leaf)->next_leaf;  /* Go to the next leaf. */
    }
    printf(" ++");

    /* The reach identifier appears to be new, create a new leaf and hook up to the previous. */
    P_TPL(last_leaf)->next_leaf = new_transposition_leaf(p_game_data);

    *p_leaf = P_TPL(last_leaf)->next_leaf; /* Return the new leaf for further polulation */
    return transposition_is_new;           /* Return the outcome. */
}







