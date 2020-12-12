#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "sokoban2.h"
#include "error.h"
#include "resolve_int.h"
#include "utility.h"
#include "move.h"


/* Conditional compile switches and 'fabric' */

//#define DBG_MOVE_MV 1 /* Prints on walking the move tree. */


#ifdef DBG_MOVE_MV
#define printf_move_mv(...) printf(__VA_ARGS__);
#else
#define printf_move_mv(...)
#endif


/* Code */
/* TODO: document. */

bool test_move(p_spot johnny, t_mv_dir mv_dir, t_s_dir search_dir)
{
    return
        search_dir == forward ?
                johnny->neighbour[mv_dir]
            &&  johnny->neighbour[mv_dir]->object[box]
            &&  johnny->neighbour[mv_dir]->neighbour[mv_dir]
            && !johnny->neighbour[mv_dir]->neighbour[mv_dir]->object[box]
            && !is_hardnogo(johnny->neighbour[mv_dir]->neighbour[mv_dir])
        :
                johnny->neighbour[mv_dir]
            && !johnny->neighbour[mv_dir]->object[target]
            &&  johnny->neighbour[OPPOSITE_MV_DIR(mv_dir)]
            &&  johnny->neighbour[OPPOSITE_MV_DIR(mv_dir)]->object[target]
        ;
}


/* TODO: document. */
void make_move(p_game_data_t p_game_data, p_spot johnny, t_mv_dir mv_dir, t_s_dir search_dir)
{
    p_spot src, dst; /* src is source; dst is destination. */

    printf_move_mv("%c: %02lu-%c", search_dir?'B':'F', SPOT_NO(johnny), mv_dir_name(mv_dir))

    /* Sort out which spots are source and which are destination. */
    /* The search direction determines which spots are involved.  */
    /* Note that the main difference between make_move() and take_back() is that src and dst are swapped. */
    if (search_dir == forward)
    {
        src = johnny->neighbour[mv_dir];  /* action */
        dst = johnny->neighbour[mv_dir]->neighbour[mv_dir];  /* action */
    }
    else
    {
        src = johnny->neighbour[OPPOSITE_MV_DIR(mv_dir)];  /* action */
        dst = johnny;  /* action */
    }

    san_if ( ! src) /* Error if source does not exist. */
        san_exit( print_error(move_obj_err, 1, search_dir, SPOT_NO(johnny), (int) mv_dir_name(mv_dir)) )

    san_if ( ! dst) /* Error if destination does not exist. */
        san_exit( print_error(move_obj_err, 2, search_dir, SPOT_NO(johnny), (int) mv_dir_name(mv_dir)) )

    san_if ( src->object[search_dir] ==  not_present) /* Error if source has has no object.  */
        san_exit( print_error(move_obj_err, 3, search_dir, SPOT_NO(johnny), (int) mv_dir_name(mv_dir)) )

    san_if ( dst->object[search_dir] ==  present ) /* Error if destination is not empty. */
        san_exit( print_error(move_obj_err, 4, search_dir, SPOT_NO(johnny), (int) mv_dir_name(mv_dir)) )

    src->object[search_dir] = not_present; /* Move the box from here ...  */
    dst->object[search_dir] = present;     /*  ... to here.               */

    /* Move Johnny too. */
    p_game_data->johnny = johnny->neighbour[mv_dir];
}


/* TODO: document. */

/* Parameter p_game_data_t p_game_data is only used when DBG_SANITY is defined. */
#pragma GCC diagnostic ignored "-Wunused-parameter"

void take_back(p_game_data_t p_game_data, p_spot johnny, t_mv_dir mv_dir, t_s_dir search_dir)
{
    p_spot src, dst; /* src is source; dst is destination. */

//     printf_move_mv("%c-T: %02lu-%c", search_dir?'B':'F', SPOT_NO(johnny), mv_dir_name(mv_dir))

    /* Sort out which spots are source and which are destination. */
    /* The search direction determines which spots are involved.  */
    /* Note that the main difference between make_move() and take_back() is that src and dst are swapped. */
    if (search_dir == forward)
    {
        src = johnny->neighbour[mv_dir]->neighbour[mv_dir];
        dst = johnny->neighbour[mv_dir];
    }
    else
    {
        src = johnny;
        dst = johnny->neighbour[OPPOSITE_MV_DIR(mv_dir)];
    }

    san_if ( ! src) /* Error if source does not exist. */
        san_exit( print_error(move_obj_err, 1, search_dir, SPOT_NO(johnny), (int) mv_dir_name(mv_dir)) )

    san_if ( ! dst) /* Error if destination does not exist. */
        san_exit( print_error(move_obj_err, 2, search_dir, SPOT_NO(johnny), (int) mv_dir_name(mv_dir)) )

    san_if ( src->object[search_dir] ==  not_present) /* Error if source has has no object.  */
        san_exit( print_error(move_obj_err, 3, search_dir, SPOT_NO(johnny), (int) mv_dir_name(mv_dir)) )

    san_if ( dst->object[search_dir] ==  present ) /* Error if destination is not empty. */
        san_exit( print_error(move_obj_err, 4, search_dir, SPOT_NO(johnny), (int) mv_dir_name(mv_dir)) )

    src->object[search_dir] = not_present; /* Move the box from here ...  */
    dst->object[search_dir] = present;     /*  ... to here.               */

    /* Note: moving Johnny is neither correct nor necessary for take_back(). */
}








