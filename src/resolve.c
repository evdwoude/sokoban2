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

/* Conditional compile switches  */

#define DBG_WALK_MV 1 /* Prints on walking the move tree. */

/* Conditional compile 'fabric' */

#ifdef DBG_WALK_MV
#define printf_walk_mv(...) printf(__VA_ARGS__);
#else
#define printf_walk_mv(...)
#endif



/* Internal protos */

void walk_to_extend_depth(p_game_data_t p_game_data, uint32_t root, int extend_at, int search_dir);

void descend(p_game_data_t p_game_data, uint32_t *move, int *depth, int search_dir);
void ascend(p_game_data_t p_game_data, uint32_t *move, int *depth, int search_dir);
void walk_lateral(p_game_data_t p_game_data, uint32_t *move, int search_dir);

void move_object(p_game_data_t p_game_data, uint32_t move, t_mv_action action, int search_dir);

void extend_depth(p_game_data_t p_game_data, uint32_t move, int search_dir);

void explore_for_reach(p_game_data_t p_game_data, p_spot spot, int search_dir);


/* Exported data */

/* Imported data */

/* Local data */

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
    p_game_data->forward_move_root = new_move_node(p_game_data, FORWARD);

    /* Add the setup position to the position tree. */
    find_or_add_position(p_game_data, FORWARD , &move_index);

    walk_to_extend_depth(p_game_data, p_game_data->forward_move_root, 3, FORWARD);

    dbg_print_setup(p_game_data);

    return no_error;
}


void walk_to_extend_depth(p_game_data_t p_game_data, uint32_t root, int extend_at, int search_dir)
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


void descend(p_game_data_t p_game_data, uint32_t *move, int *depth, int search_dir)
{
    san_if ( ! P_MN(*move)->child ) /* Sanity check: Do we have a child to descend to? */
        san_exit( print_error(no_child) )

    *move = P_MN(*move)->child;             /* Descend a level deeper in the move tree. */
    (*depth)++;                             /* Update current depth                     */

    printf_walk_mv("\n        Descended   ");

    /* Apply this move to the warehouse, i.e. move the box. */
    move_object(p_game_data, *move, make_move, search_dir);
}


void ascend(p_game_data_t p_game_data, uint32_t *move, int *depth, int search_dir)
{
    san_if ( ! P_MN(*move)->next.parent) /* Sanity check: Do we have a parent to ascend to? */
        san_exit( print_error(no_parent) )

    printf_walk_mv("\n");

    /* Apply the undoing of this move to the warehouse, i.e. move the last-moved box back. */
    move_object(p_game_data, *move, take_back, search_dir);

    *move = P_MN(*move)->next.parent;       /* Ascend a level up in the move tree. */
    (*depth)--;                             /* Update current depth                */

    printf_walk_mv("   Ascended");
}


void walk_lateral(p_game_data_t p_game_data, uint32_t *move, int search_dir)
{
    san_if ( ! P_MN(*move)->next.sibbling) /* Sanity check: Do we have a sibbling to move to? */
        san_exit( print_error(no_sibbling) )

    printf_walk_mv("\n");

    /* Apply the undoing of the current move to the warehouse, i.e. move the last-moved box back. */
    move_object(p_game_data, *move, take_back, search_dir);

    *move =  P_MN(*move)->next.sibbling;    /* Move laterally in the move tree. */

    printf_walk_mv("   Moved       ");

    /* Apply the new move to the warehouse, i.e. move the now-current box. */
    move_object(p_game_data, *move, make_move, search_dir);
}

void move_object(p_game_data_t p_game_data, uint32_t move, t_mv_action action, int search_dir)
{
    p_spot spot, src, dst;

    t_direction mv_dir = MV_GET_MOVE_DIR(P_MN(move)->move_data);
    int spot_no        = MV_GET_SPOT_NO(P_MN(move)->move_data);

    printf_walk_mv("%c-MM: %02ld-%c", search_dir?'B':'F', SPOT_NO(spot), mv_dir_name(mv_dir));

    san_if ( mv_dir < right || mv_dir > down  ) /* Bounds check for move_dir */
        san_exit( print_error(move_obj_err, 1, search_dir, SPOT_NO(spot), (int) mv_dir_name(mv_dir)) )
    san_if ( &p_game_data->spot_pool[spot_no] >= p_game_data->pool_ptr ) /* Bounds check for spot_no */
        san_exit( print_error(move_obj_err, 2, search_dir, SPOT_NO(spot), (int) mv_dir_name(mv_dir)) )

    spot = &p_game_data->spot_pool[spot_no];
    src  = spot->neighbour[mv_dir];

    san_if ( ! src) /* Does the source spot (neighbour) exist? */
        san_exit( print_error(move_obj_err, 3, search_dir, SPOT_NO(spot), (int) mv_dir_name(mv_dir)) )

    if (search_dir == FORWARD)
        dst  = spot->neighbour[mv_dir]->neighbour[mv_dir];
    else
        dst  = spot;

    san_if ( ! dst ) /* Error if no destination. */
        san_exit( print_error(move_obj_err, 4, search_dir, SPOT_NO(spot), (int) mv_dir_name(mv_dir)) )
    san_if ( src->has_box == 0 ) /* Error if source has has bo object.  */
        san_exit( print_error(move_obj_err, 5, search_dir, SPOT_NO(spot), (int) mv_dir_name(mv_dir)) )
    san_if ( dst->has_box == 1 ) /* Error if destination is empty. */
        san_exit( print_error(move_obj_err, 6, search_dir, SPOT_NO(spot), (int) mv_dir_name(mv_dir)) )

    src->has_box = 0; /* Move the box from here ...   */
    dst->has_box = 1; /* ... to here.                 */
}


void extend_depth(p_game_data_t p_game_data, uint32_t move, int search_dir)
{
//     Pseudo:
//     explore_for_reach()
//     for spots on reach list:
//         for movable boxes from spot
//             add move

    printf_walk_mv("    Extend depth");

    return;
}


void explore_for_reach(p_game_data_t p_game_data, p_spot spot, int search_dir)
{
    t_direction dir = right;
    int mark;
    struct spot *neighbour;
    struct spot *end;

    mark = next_mark(p_game_data);
    spot->reach_mark  = mark;
    spot->explore_reach_list = NULL;
    end = spot;

    /* Build up a list of spots to visit, while at the same time walk down that list. */
    while (spot)
    {
        /* Insert all applicable neighbours into the list: */
        for (dir=right; dir<=down; dir++)
        {
            neighbour = spot->neighbour[dir];

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
        spot = spot->explore_reach_list;
    }
    return;
}
