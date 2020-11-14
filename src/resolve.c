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


        //  Use in combination with setup-test number 20.          Start:          On J:
        //
        //          A               000-001-002-003-004-005        ########        ########
        //         / \               |                   |         #  X  .#        #    X.#
        //        /   \             006                 007        # #### #        # ####X#
        //       B--C--D             |                   |         #  .O  #        #  .O  #
        //         / \              008-009-010-011-012-013        ####  X#        ####   #
        //        /   \                          |   |   |         #. X   #        #. X   #
        //       E-----F                        014-015-016        ########        ########
        //      /     / \                        |   |   |
        //     /     /   \          017-018-019-020-021-022
        //    G     H-----I
        //               /
        //              /
        //             J
        //

        uint32_t mv_A, mv_B, mv_C, mv_D, mv_E, mv_F, mv_G, mv_H, mv_I, mv_J;

        mv_A = p_game_data->forward_move_root;
        mv_B = new_move_node(p_game_data, forward);
        mv_C = new_move_node(p_game_data, forward);
        mv_D = new_move_node(p_game_data, forward);
        mv_E = new_move_node(p_game_data, forward);
        mv_F = new_move_node(p_game_data, forward);
        mv_G = new_move_node(p_game_data, forward);
        mv_H = new_move_node(p_game_data, forward);
        mv_I = new_move_node(p_game_data, forward);
        mv_J = new_move_node(p_game_data, forward);

        P_MN(mv_A)->move_data     =  0; /* Represents start position; no move done yet. */
        P_MN(mv_A)->child         =  mv_B;

        P_MN(mv_B)->move_data     =  left  | MV_SET_SPOT_NO(3);
        P_MN(mv_B)->move_data     |= MV_SET_HAS_SIBB;
        P_MN(mv_B)->next.sibbling =  mv_C;

        P_MN(mv_C)->move_data     =  right | MV_SET_SPOT_NO(1);
        P_MN(mv_C)->move_data     |= MV_SET_HAS_SIBB;
        P_MN(mv_C)->child         =  mv_E;
        P_MN(mv_C)->next.sibbling =  mv_D;

        P_MN(mv_D)->move_data     =  right | MV_SET_SPOT_NO(18);
        P_MN(mv_D)->next.parent   =  mv_A;

        P_MN(mv_E)->move_data     =  down | MV_SET_SPOT_NO(13);
        P_MN(mv_E)->move_data     |= MV_SET_HAS_SIBB;
        P_MN(mv_E)->child         =  mv_G;
        P_MN(mv_E)->next.sibbling =  mv_F;

        P_MN(mv_F)->move_data     =  up | MV_SET_SPOT_NO(22);
        P_MN(mv_F)->child         =  mv_H;
        P_MN(mv_F)->next.parent   =  mv_C;

        P_MN(mv_G)->move_data     =  right | MV_SET_SPOT_NO(2);
        P_MN(mv_G)->next.parent   =  mv_E;

        P_MN(mv_H)->move_data     =  left | MV_SET_SPOT_NO(4);
        P_MN(mv_H)->move_data     |= MV_SET_HAS_SIBB;
        P_MN(mv_H)->next.sibbling =  mv_I;

        P_MN(mv_I)->move_data     =  right | MV_SET_SPOT_NO(2);
        P_MN(mv_I)->child         =  mv_J;
        P_MN(mv_I)->next.parent   =  mv_F;

        P_MN(mv_J)->move_data     =  up | MV_SET_SPOT_NO(16);
        P_MN(mv_J)->next.parent   =  mv_I;

    walk_to_extend_depth(p_game_data, p_game_data->forward_move_root, 3, forward);

    dbg_print_setup(p_game_data);

        printf_walk_mv("\n\nBackward\n")

        //  Now use test-setup 20 again for backward checks.       Start:          On D:
        //
        //          A               000-001-002-003-004-005        ########        ########
        //         / \               |                   |         #  X  .#        #  X   #
        //        /   \             006                 007        # #### #        # ####.#
        //       B--C--D             |                   |         #  .O  #        #  .O  #
        //                          008-009-010-011-012-013        ####  X#        ####  X#
        //                                       |   |   |         #. X   #        #. X   #
        //                                      014-015-016        ########        ########
        //                                       |   |   |
        //                          017-018-019-020-021-022
        //

        // Reuse the tree with some modifications:

        P_MN(mv_B)->move_data     =  left  | MV_SET_SPOT_NO(4);
        P_MN(mv_B)->move_data     |= MV_SET_HAS_SIBB;

        P_MN(mv_C)->move_data     =  down | MV_SET_SPOT_NO(7);
        P_MN(mv_C)->move_data     |= MV_SET_HAS_SIBB;
        P_MN(mv_C)->child         =  0;

        P_MN(mv_D)->move_data     =  left | MV_SET_SPOT_NO(4);

        walk_to_extend_depth(p_game_data, p_game_data->forward_move_root, 3, backward);

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
    make_move(p_game_data, MOVE_SPOT(*move), MOVE_DIR(*move), just_make, search_dir);
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
    make_move(p_game_data, MOVE_SPOT(*move), MOVE_DIR(*move), just_make, search_dir);
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

                // make move
                // consider_move()
                // take move back
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

