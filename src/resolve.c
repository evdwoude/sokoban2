#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "sokoban2.h"
#include "error.h"
#include "resolve_int.h"
#include "move.h"
#include "position.h"
#include "utility.h"
#include "resolve.h"

/* Local defs */

/* Conditional compile switches  */

#define DBG_WALK_MV 1 /* Prints on walking the move tree. */

/* Conditional compile 'fabric' */

#ifdef DBG_WALK_MV
#define printf_walk_mv(...) printf(__VA_ARGS__);
#else
#define printf_walk_mv(...)
#endif



/* Internal protos */

void walk_to_extend_depth(p_game_data_t p_game_data, uint32_t root, int extend_at, t_s_dir search_dir);

void descend(p_game_data_t p_game_data, uint32_t *move, int *depth, t_s_dir search_dir);
void ascend(p_game_data_t p_game_data, uint32_t *move, int *depth, t_s_dir search_dir);
void walk_lateral(p_game_data_t p_game_data, uint32_t *move, t_s_dir search_dir);

void extend_depth(p_game_data_t p_game_data, uint32_t move, t_s_dir search_dir);
void explore_for_reach(p_game_data_t p_game_data, p_spot spot, t_s_dir search_dir);
void consider(p_game_data_t p_game_data, uint32_t parent, p_spot johnny, t_mv_dir mv_dir, t_s_dir search_dir);
void add_move(p_game_data_t p_game_data, uint32_t parent, p_spot johnny, t_mv_dir mv_dir, uint32_t new_move);

/* Code */


skbn_err resolve(p_game_data_t p_game_data)
{
    int32_t *move_index = NULL;

    /* Define all hard no go spots. Spots that we never should put a box on. */
    define_hardnogos(p_game_data);

    /* Using the defined hard nogos, create the linked list of spots that form the base of the positions. */
    create_position_base(p_game_data);

    /* Create a root node for the position tree */
    p_game_data->position_root = new_position_node(p_game_data);


    /* Forward search initialisations: */

    /* Create a root node for the forward move tree */
    p_game_data->forward_move_root = new_move_node(p_game_data, forward);

    /* Re-use the move's spot number for reinitialisation of Johhny before each consequive tree walk. */
    P_MN(p_game_data->forward_move_root)->move_data = MV_SET_SPOT_NO( SPOT_NO(p_game_data->johnny) );

    /* Add the setup position to the position tree. */
    find_or_add_position(p_game_data, forward , &move_index);
    *move_index = p_game_data->forward_move_root;


    /* Backward search initialisations: */

    /* TODO: Make this a list of roots */
    p_game_data->backward_move_base = new_move_node(p_game_data, forward);

    /* Add the setup end-position to the position tree. TODO: add multiple, for all reaches. */
    find_or_add_position(p_game_data, backward , &move_index);
    *move_index = 1; /* TODO: Populate with it's proper base. */


    /* Now go for it: */


    dbg_print_setup(p_game_data);

    return no_error;
}


void walk_to_extend_depth(p_game_data_t p_game_data, uint32_t root, int extend_at, t_s_dir search_dir)
{
    uint32_t move = root;
    int depth = 0;

    /* Re-initlialise Johnny, because  A: make_move(..., take_back, ...) doesn't do it, and            */
    /*                                 B: Johnny's position differs anyway for the various move roots. */
    p_game_data->johnny = MOVE_SPOT(p_game_data->forward_move_root);

    while (1)
    {
        while ( P_MN(move)->child )
            descend(p_game_data, &move, &depth, search_dir);

        if (depth == extend_at)
            extend_depth(p_game_data, move, search_dir);

        while ( ! MV_GET_HAS_SIBB(P_MN(move)->move_data) )
            if (move != root)
                ascend(p_game_data, &move, &depth, search_dir);
            else
                return;

        walk_lateral(p_game_data, &move, search_dir);
    }
    return;
}


void descend(p_game_data_t p_game_data, uint32_t *move, int *depth, t_s_dir search_dir)
{
    san_if ( ! P_MN(*move)->child ) /* Sanity check: Do we have a child to descend to? */
        san_exit( print_error(no_child) )

    printf_walk_mv("\nDescend     ");

    *move = P_MN(*move)->child;             /* Descend a level deeper in the move tree. */
    (*depth)++;                             /* Update current depth                     */

    /* Apply this move to the warehouse, i.e. move the box. */
    make_move(p_game_data, MOVE_SPOT(*move), MOVE_DIR(*move), just_move, search_dir);
}


void ascend(p_game_data_t p_game_data, uint32_t *move, int *depth, t_s_dir search_dir)
{
    san_if ( ! P_MN(*move)->next.parent) /* Sanity check: Do we have a parent to ascend to? */
        san_exit( print_error(no_parent) )

    printf_walk_mv("\nAscend      ");

    /* Apply the undoing of this move to the warehouse, i.e. move the last-moved box back. */
    make_move(p_game_data, MOVE_SPOT(*move), MOVE_DIR(*move), take_back, search_dir);

    *move = P_MN(*move)->next.parent;       /* Ascend a level up in the move tree. */
    (*depth)--;                             /* Update current depth                */
}


void walk_lateral(p_game_data_t p_game_data, uint32_t *move, t_s_dir search_dir)
{
    san_if ( ! P_MN(*move)->next.sibbling) /* Sanity check: Do we have a sibbling to move to? */
        san_exit( print_error(no_sibbling) )

    printf_walk_mv("\nWalk lateral");

    /* Apply the undoing of the current move to the warehouse, i.e. move the last-moved box back. */
    make_move(p_game_data, MOVE_SPOT(*move), MOVE_DIR(*move), take_back, search_dir);

    *move =  P_MN(*move)->next.sibbling;    /* Move laterally in the move tree. */

    /* Apply the new move to the warehouse, i.e. move the now-current box. */
    make_move(p_game_data, MOVE_SPOT(*move), MOVE_DIR(*move), just_move, search_dir);
}


void extend_depth(p_game_data_t p_game_data, uint32_t move, t_s_dir search_dir)
{
    p_spot johnny;
    t_mv_dir mv_dir;

    printf_walk_mv("\nExtend depth: ");

    johnny = p_game_data->johnny;

    explore_for_reach(p_game_data, johnny, search_dir);

    while (johnny) /* For all spots in reach list: */
    {
        for (mv_dir=right; mv_dir<=down; mv_dir++) /* For all move directions: */
        {
            if (test_move(p_game_data, johnny, mv_dir, search_dir))
            {
                printf_walk_mv("\nConsider spot %ld, dir %c",  SPOT_NO(johnny), mv_dir_name(mv_dir));

                make_move(p_game_data, johnny, mv_dir, just_move, search_dir);
                consider(p_game_data, move, johnny, mv_dir, search_dir);
                make_move(p_game_data, johnny, mv_dir, take_back, search_dir);
            }
        }
        johnny = johnny->explore_reach_list;
    }

    return;
}


void explore_for_reach(p_game_data_t p_game_data, p_spot johnny, t_s_dir search_dir)
{
    t_mv_dir dir = right;
    int mark;
    struct spot *neighbour;
    struct spot *end;

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

            if (    neighbour                       /* Does Johnny's neighbour exists? */
                &&  !neighbour->object[search_dir]  /* Is that spot empty?             */
                &&  neighbour->reach_mark < mark )  /* Not yet explored?               */
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


void consider(p_game_data_t p_game_data, uint32_t parent, p_spot johnny, t_mv_dir mv_dir, t_s_dir search_dir)
{
    t_outcome_add_tp outcome;
    int32_t new_move, *move_path;

    outcome = find_or_add_position(p_game_data, search_dir, &move_path);

    printf_walk_mv("    Foap: %s\n", outcome==position_is_new ? "new" : outcome==bingo ? "bingo" : "exists");

    /* If the new move leads to a prevously existing position, do nothing. */
    if (outcome == position_exists_on_same_direction)
         return;

    /* The move appears to be relevant. */
    new_move = new_move_node(p_game_data, search_dir);       /* Create new move node.                        */
    *move_path = new_move;                                   /* Refer to the new move from the new position. */
    add_move(p_game_data, parent, johnny, mv_dir, new_move); /* Add the new move to the move tree.           */

    // if (outcome == bingo)  /* If the new move leads to the solution, call it out. (The new move was        */
    //     bingo();           /* deliberately added to move tree first, so it's now avaible in the solution.) */
}


void add_move(p_game_data_t p_game_data, uint32_t parent, p_spot johnny, t_mv_dir mv_dir, uint32_t new_move)
{
    uint32_t sibbling;

    /* Polulate move direction and spot number. */
    P_MN(new_move)->move_data   = mv_dir | MV_SET_SPOT_NO( SPOT_NO(johnny) );

    /* Link back (uplink) to parent, regardless whether this new child is the only one or the youngest one. */
    P_MN(new_move)->next.parent = parent;

    if ( ! P_MN(parent)->child )        /* If parent has no child yet...                    */
    {
        P_MN(parent)->child = new_move; /* ... link the new child to the parent (downlink). */
        return;
    }
                                        /* ... else, add the new child as a sibbling:        */

    sibbling = P_MN(parent)->child;                      /* First goto parent's first child. */

    while ( MV_GET_HAS_SIBB(P_MN(sibbling)->move_data) ) /* Then goto the youngest sibbling. */
        sibbling = P_MN(sibbling)->next.sibbling;

    P_MN(sibbling)->move_data     |= MV_SET_HAS_SIBB;   /* The previously youngest child has a sibbling now. */
    P_MN(sibbling)->next.sibbling  = new_move;          /* Link the new child up to his older sibblings.     */
    return;
}















