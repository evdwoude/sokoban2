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


// typedef enum {false = 0, true = 1} t_bool;

/* Internal protos */

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

skbn_err add_transposition(p_game_data_t p_game_data)
{
    /* Current node index. Start at the root of the forward transpoition tree. */
    uint32_t node = p_game_data->forward_transposition_root;

    uint32_t *next;         /* Reference of the current node's child index. */
    uint32_t new_node = 0;  /* New node index. */

    p_spot  spot;           /* Iterates over the tranposition base. */
    skbn_err  error;

    int new_transposition = 0;

    for(spot = p_game_data->transposition_head; spot; spot = spot->transposition_list)
    {
        if (spot->has_box)
            next = &(P_TPN(node)->spot_has_box);
        else
            next = &(P_TPN(node)->spot_is_empty);

        printf_add_tp( spot->has_box ? "B" : "E" )

        if ( ! *next)
        {
            /* So there is no next node. This means this is a new transpostion. */

            new_transposition = 1;

            /* Allocate a new node or leaf. (Note: the new node is already zero-initialised.) */
            if (spot->transposition_list)
                new_node = new_transposition_node(p_game_data);
            else
            {
                new_node = new_transposition_leaf(p_game_data);
                printf_add_tp("L");
            }

            if (!new_node)
                return print_error(out_of_tree_memory);

            /* Hook the new node up to the current node's 'next'; either spot_has_box or spot_is_empty. */
            *next = new_node;

            /* Indicate that this transposition is new. */
            /* We do not break up the loop here, because we need to make the complete transposition known. */
            printf_add_tp("+ ")
        }
        dbg_else_add_tp
            printf_add_tp("- ")

        node = *next;   /* Goto the next node. If there was no next node yet, we've just created one. */
    }
#if 0
    if (new_transposition || node->move_path.backward_path)
        *new_tp_node = node;

    return none;
#endif
}


