#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "sokoban2.h"
#include "error.h"
#include "resolve_int.h"
#include "utility.h"
#include "move.h"


/* Conditional compile switches and 'fabric' */

#define DBG_MOVE_MV 1 /* Prints on walking the move tree. */


#ifdef DBG_MOVE_MV
#define printf_move_mv(...) printf(__VA_ARGS__);
#else
#define printf_move_mv(...)
#endif


/* Code */
/* TODO: document. */

bool test_move(p_game_data_t p_game_data, p_spot johnny, t_mv_dir mv_dir, t_s_dir search_dir)
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
void make_move(p_game_data_t p_game_data, p_spot johnny, t_mv_dir mv_dir, t_action action, t_s_dir search_dir)
{
    p_spot spots[2]; /* spots[mv_src] is source; spots[mv_dst] is destination. */

    printf_move_mv(action==just_move?"%c: %02ld-%c":"", search_dir?'B':'F', SPOT_NO(johnny), mv_dir_name(mv_dir));

    /* Sort out which spots are source and which are destination. The search direction determines which  */
    /* spots are involved and the action determinces which of them is source and which is destimation.   */
    if (search_dir == forward)
    {
        spots[   action] = johnny->neighbour[mv_dir];
        spots[ ! action] = johnny->neighbour[mv_dir]->neighbour[mv_dir];
    }
    else
    {
        spots[   action] = johnny->neighbour[OPPOSITE_MV_DIR(mv_dir)];
        spots[ ! action] = johnny;
    }

    san_if ( ! spots[mv_src]) /* Error if source does not exist. */
        san_exit( print_error(move_obj_err, 1, search_dir, SPOT_NO(johnny), (int) mv_dir_name(mv_dir)) )

    san_if ( ! spots[mv_dst]) /* Error if destination does not exist. */
        san_exit( print_error(move_obj_err, 2, search_dir, SPOT_NO(johnny), (int) mv_dir_name(mv_dir)) )

    san_if ( spots[mv_src]->object[search_dir] ==  not_present) /* Error if source has has no object.  */
        san_exit( print_error(move_obj_err, 3, search_dir, SPOT_NO(johnny), (int) mv_dir_name(mv_dir)) )

    san_if ( spots[mv_dst]->object[search_dir] ==  present ) /* Error if destination is not empty. */
        san_exit( print_error(move_obj_err, 4, search_dir, SPOT_NO(johnny), (int) mv_dir_name(mv_dir)) )

    spots[mv_src]->object[search_dir] = not_present; /* Move the box from here ...  */
    spots[mv_dst]->object[search_dir] = present;     /*  ... to here.               */

    /* Move Johnny too. Note that this is only correct for just_make and not for take_back, but   */
    /* it is only required in the case of just_make and does not harm in the case of take_back.   */
    p_game_data->johnny = johnny->neighbour[mv_dir];
}
