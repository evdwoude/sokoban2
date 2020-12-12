#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "sokoban2.h"
#include "error.h"
#include "resolve_int.h"
#include "move.h"
#include "utility.h"
#include "solution.h"


/* Conditional compile switches and 'fabric' */

//#define DBG_SOLUTION 1 /* Debug printing on reconstruction the move trees for printing. */

#ifdef DBG_SOLUTION
#define debug_solution(...) printf(__VA_ARGS__);
#else
#define debug_solution(...)
#endif


/* Local definitions */

#define LINE_LENGTH 100
#define N_LINES 3

typedef enum {is_move=0, is_step=1} t_move_type;

/* Internal protos */

void construct_forward_path(p_game_data_t p_game_data, uint32_t move, bool restore);
void construct_bckward_path(p_game_data_t p_game_data, uint32_t move, bool restore);

void print_path(p_game_data_t p_game_data);
void find_steps(p_game_data_t p_game_data, p_spot src, p_spot dst);
void print_steps(p_spot src, p_spot dst);
void print_single(t_mv_dir move_direction, t_move_type move_type);
const char *one_or_two_newlines(int line_count);


/* Code */

/* TODO: document. */
void print_solution(p_game_data_t p_game_data, uint32_t this_move, uint32_t that_move, t_s_dir search_dir)
{
    debug_solution("\n\nSolution found !   {%u, %u}.\n", this_move, that_move);

    if (search_dir == forward)
    {
        construct_forward_path(p_game_data, this_move, true );
        construct_bckward_path(p_game_data, that_move, false);

        P_MN(this_move)->child = that_move;    /* Connect paths.  */
    }
    else
    {
        construct_forward_path(p_game_data, that_move, false);
        construct_bckward_path(p_game_data, this_move, true );

        P_MN(that_move)->child = this_move;    /* Connect paths.  */
    }

    /* Restore Johhny's start spot. We need him to reconstruct the steps between the moves. */
    p_game_data->johnny = MOVE_SPOT(p_game_data->forward_move_root);

    print_path(p_game_data); /* Print the moves and the steps in between. */

    exit(no_error);
}



void construct_forward_path(p_game_data_t p_game_data, uint32_t move, bool restore)
{
    uint32_t next;

    debug_solution("\nForward moves:");

    next = move;
    while (move != p_game_data->forward_move_root)
    {
        debug_solution(" %u", move);

        if (restore)
            take_back(p_game_data, MOVE_SPOT(move), MOVE_DIR(move), forward);

        /* Find parent. */
        while ( MV_GET_HAS_SIBB(P_MN(next)->move_data) )    /* Find younghest sibbling */
            next = P_MN(next)->next.sibbling;
        next = P_MN(next)->next.parent;                     /* From there goto parent. */

        P_MN(next)->child = move; /* Link the parent directly to this move. (Orphaning older sibblings.) */

        move = next; /* Goto parent. */
    }
}


void construct_bckward_path(p_game_data_t p_game_data, uint32_t move, bool restore)
{
    uint32_t root = p_game_data->backward_move_root_list;
    uint32_t next;

    debug_solution("\nBckward roots:");

    /* Mark the backward move roots as root. (This unlinks them from each other, but that's fine.) */
    while (root)
    {
        debug_solution(" {%u}", root);
        next = P_MN(root)->next.sibbling; /* Look up the next root, while retaining this one. */

        P_MN(root)->next.parent = 0;      /* Mark this one as a root. */
        root = next;
    }

    debug_solution("\nBckward moves:");

    next = move;
    while ( P_MN(move)->next.parent != 0 ) /* While not a backward root, ... */
    {
        debug_solution(" %u", move);

        if (restore)
            take_back(p_game_data, MOVE_SPOT(move), MOVE_DIR(move), backward);

        /* Find parent. */
        while ( MV_GET_HAS_SIBB(P_MN(next)->move_data) )    /* Find youngest sibbling  */
            next = P_MN(next)->next.sibbling;
        next = P_MN(next)->next.parent;                     /* From there goto parent. */

        /* Change the backward move in to a forward one; forget about sibblings. */
        P_MN(move)->move_data = OPPOSITE_MV_DIR( MOVE_DIR(move) ) |
                                MV_SET_SPOT_NO( SPOT_NO( MOVE_SPOT(move)->neighbour[MOVE_DIR(move)] ) );

        P_MN(move)->child = next; /* Link move directly to it's parent. (Swapping the parent/child roles.) */

        move = next;        /* Goto parent. */
    }
}


void print_path(p_game_data_t p_game_data)
{
    uint32_t move = P_MN(p_game_data->forward_move_root)->child;

    printf("\n");
    dbg_print_setup(p_game_data);

    printf("\n\nSolution:");
    printf("%s", one_or_two_newlines(0) );

    move = P_MN(p_game_data->forward_move_root)->child;
    while ( P_MN(move)->next.parent != 0 )
    {
        find_steps(p_game_data, p_game_data->johnny, MOVE_SPOT(move));
        print_single(MOVE_DIR(move), is_move);

        make_move(p_game_data, MOVE_SPOT(move), MOVE_DIR(move), forward);
        move = P_MN(move)->child;
    }

    debug_solution("\n");
}


void find_steps(p_game_data_t p_game_data, p_spot src, p_spot dst)
{
    uint32_t mark = 0;
    p_spot johnny = dst;
    p_spot end = johnny;

    t_mv_dir dir;
    p_spot neighbour;

    reinit_mark(p_game_data, UINT32_MAX); /* Mark all spots as unexplored. */

    johnny->other_reach_list = NULL;
    johnny->reach_mark  = mark;

    if (dst == src)
        return;

    /* Starting from dst, find src, thereby marking visited spots with increasing "distance". */
    /* (Build up a list of spots to visit, while at the same time walk down that list.) */
    while (johnny)
    {
        mark = johnny->reach_mark + 1;

        /* Insert all applicable neighbours into the list: */
        for (dir=right; dir<=down; dir++)
        {
            neighbour = johnny->neighbour[dir];

            if (    neighbour                             /* Neighbour spot exists? */
                &&  !neighbour->object[forward]           /* Is that spot empty?    */
                &&  neighbour->reach_mark == UINT32_MAX ) /* Not yet explored?      */
            {
                neighbour->reach_mark  = mark;            /* Mark it: */

                if (neighbour == src)
                {
                    print_steps(src, dst);
                    return;
                }

                /* Add it to end of list: */
                end->other_reach_list = neighbour;
                end                   = neighbour;
                end->other_reach_list = NULL;
            }
        }
        johnny = johnny->other_reach_list;                /* Move down the list until done: */
    }
}


void print_steps(p_spot src, p_spot dst)
{
    t_mv_dir dir, dir_to_lowest;
    uint32_t lowest_mark;
    p_spot neighbour;

    dir_to_lowest = right;
    while (src && src != dst)
    {
        lowest_mark = src->reach_mark;

        for (dir=right; dir<=down; dir++)
        {
            neighbour = src->neighbour[dir];

            if (    neighbour                             /* Neighbour spot exists? */
                &&  !neighbour->object[forward] )         /* Is that spot empty?    */
            {
                if (neighbour->reach_mark < lowest_mark)
                {
                    lowest_mark   = neighbour->reach_mark;
                    dir_to_lowest = dir;
                }
            }
        }
        print_single(dir_to_lowest, is_step);

        /* Goto the lowest neighbour and continue. */
        src = src->neighbour[dir_to_lowest];
    }
}


void print_single(t_mv_dir move_direction, t_move_type move_type)
{
    static int line_length = 0;
    static int n_lines = 0;
    static int last_is_move = 0;


    if (move_type == is_step)
    {
        if (line_length >= LINE_LENGTH)
        {
            printf("%s%s", last_is_move ? "" : "-", one_or_two_newlines(++n_lines));
            line_length = 0;
        }
        else if (last_is_move)
        {
            if (line_length >= LINE_LENGTH-1)
            {
                printf("%s", one_or_two_newlines(++n_lines) );
                line_length = 0;
            }
            else
            {
                printf(" ");
                line_length++;
            }
        }

        printf("%c", mv_dir_name(move_direction));
        line_length++;

        last_is_move = 0;
    }
    else
    {
        if (last_is_move)
        {
            if (line_length >= LINE_LENGTH)
            {
                printf("%s", one_or_two_newlines(++n_lines) );
                line_length = 0;
            }
            else
            {
                printf(" ");
                line_length++;
            }
        }

        printf("%c", mv_dir_name(move_direction));
        line_length++;

        last_is_move  = 1;
    }
}


const char *one_or_two_newlines(int line_count)
{
    if (line_count % N_LINES == 0)
        return "\n\n  ";
    else
        return "\n  ";
}

