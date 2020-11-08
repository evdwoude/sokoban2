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


/* Code */


skbn_err resolve(p_game_data_t p_game_data)
{
//     p_spot spot = p_game_data->johnny;

    int32_t tmp_move_index; // TODO: Remove this ? Check out what to initiale the first move_path to.
    int32_t *move_index = &tmp_move_index;


    /* Define all hard no go spots. Spots that we never should put a box on. */
    define_hardnogos(p_game_data);

    /* Using the defined hard nogos, create the linked list of spots that form the base of the positions. */
    create_position_base(p_game_data);

    /* Create a root node for the position tree */
    p_game_data->position_root = new_position_node(p_game_data);

    /* Create a root node for the forward move tree */
    p_game_data->forward_move_root = new_move_node(p_game_data, forward);

    /* Add the setup position to the position tree. */
    find_or_add_position(p_game_data, forward , &move_index);

    walk_to_extend_depth(p_game_data, p_game_data->forward_move_root, 3, forward);

    dbg_print_setup(p_game_data);

    return no_error;
}


void walk_to_extend_depth(p_game_data_t p_game_data, uint32_t root, int extend_at, t_s_dir search_dir)
{
    uint32_t move;
    int depth = 0;

    move = root;

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
    move_object(p_game_data, *move, make_move, search_dir);
}


void ascend(p_game_data_t p_game_data, uint32_t *move, int *depth, t_s_dir search_dir)
{
    san_if ( ! P_MN(*move)->next.parent) /* Sanity check: Do we have a parent to ascend to? */
        san_exit( print_error(no_parent) )

    printf_walk_mv("\nAscend      ");

    /* Apply the undoing of this move to the warehouse, i.e. move the last-moved box back. */
    move_object(p_game_data, *move, take_back, search_dir);

    *move = P_MN(*move)->next.parent;       /* Ascend a level up in the move tree. */
    (*depth)--;                             /* Update current depth                */
}


void walk_lateral(p_game_data_t p_game_data, uint32_t *move, t_s_dir search_dir)
{
    san_if ( ! P_MN(*move)->next.sibbling) /* Sanity check: Do we have a sibbling to move to? */
        san_exit( print_error(no_sibbling) )

    printf_walk_mv("\nWalk lateral");

    /* Apply the undoing of the current move to the warehouse, i.e. move the last-moved box back. */
    move_object(p_game_data, *move, take_back, search_dir);

    *move =  P_MN(*move)->next.sibbling;    /* Move laterally in the move tree. */

    /* Apply the new move to the warehouse, i.e. move the now-current box. */
    move_object(p_game_data, *move, make_move, search_dir);
}


void extend_depth(p_game_data_t p_game_data, uint32_t move, t_s_dir search_dir)
{
    p_spot johnny;
    t_direction mv_dir;

    printf_walk_mv("\nExtend depth: ");

    johnny = p_game_data->johnny;

    explore_for_reach(p_game_data, johnny, search_dir);

    printf_walk_mv("\nResult: ");

    while (johnny) /* for all spots in reach list: */
    {
        for (mv_dir=right; mv_dir<=down; mv_dir++) /* for all move directions: */
        {
            if (test_move(p_game_data, johnny, mv_dir, search_dir))
            {
                printf_walk_mv("\nspot: %ld,  direction: %c",  SPOT_NO(johnny), mv_dir_name(mv_dir));

                // move object(make_move)
                // consider_move()
                // move object(take_back)
            }
        }
        johnny = johnny->explore_reach_list;
    }

    return;
}


void explore_for_reach(p_game_data_t p_game_data, p_spot johnny, t_s_dir search_dir)
{
    t_direction dir = right;
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


// void consider_move()
// {
//     outcome = find_or_add_position()
//
//     if (outcome == position_exists_on_same_direction)
//         return;
//
//     add_move();
//
//     if (outcome == bingo)
//         bingo();
// }

