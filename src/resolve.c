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
#include "solution.h"
#include "resolve.h"

/* Local defs */

/* Conditional compile switches  */

//#define DBG_WALK_MV 1 /* Prints on walking the move tree. */

/* Conditional compile 'fabric' */

#ifdef DBG_WALK_MV
#define printf_walk_mv(...) printf(__VA_ARGS__);
#else
#define printf_walk_mv(...)
#endif

#ifdef DBG_WALK_MV
#define printf_walk_indent(...) printf_walk_indent_f(__VA_ARGS__);
#else
#define printf_walk_indent(...)
#endif



/* Internal protos */
void init_all_move_trees(p_game_data_t p_game_data);
uint32_t init_move_tree(p_game_data_t p_game_data, t_s_dir search_dir);

void search_forward_and_backward(p_game_data_t p_game_data);

void walk_tree(p_game_data_t p_game_data, uint32_t root, int extend_at, t_s_dir search_dir, int *extention);

void descend(p_game_data_t p_game_data, uint32_t *move, int *depth, t_s_dir search_dir);
void ascend(p_game_data_t p_game_data, uint32_t *move, int *depth, t_s_dir search_dir);
void walk_lateral(p_game_data_t p_game_data, uint32_t *move, t_s_dir search_dir);

void extend_depth(p_game_data_t p_game_data, uint32_t move, t_s_dir search_dir, int *extention);
void explore_for_reach(p_game_data_t p_game_data, p_spot spot, t_s_dir search_dir);
void consider(p_game_data_t p_game_data, uint32_t parent, p_spot johnny, t_mv_dir mv_dir, t_s_dir search_dir, int *extention);
void add_move(p_game_data_t p_game_data, uint32_t parent, p_spot johnny, t_mv_dir mv_dir, uint32_t new_move);

void clean_up_ascending(uint32_t move);
void clean_up_walking_lateral(uint32_t move);

#ifdef DBG_WALK_MV
void printf_walk_indent_f(int print, int change);
#endif


/* Code */

skbn_err resolve(p_game_data_t p_game_data)
{
    /* Define all hard no go spots. Spots that we never should put a box on. */
    define_hardnogos(p_game_data);

    /* Using the defined hard nogos, create the linked list of spots that form the base of the positions. */
    create_position_base(p_game_data);

    /* Create the root node for the position tree */
    p_game_data->position_root = new_position_node(p_game_data);

    /* Initialise move trees (one tree for forward search, multiple for backward). */
    init_all_move_trees(p_game_data);

    /* Now go for it: */
     search_forward_and_backward(p_game_data);

    return no_error;
}


void init_all_move_trees(p_game_data_t p_game_data)
{
    struct spot* spot;
    uint32_t new_backward_root = 0;

    /* Move tree initialisation for forward search. */
    p_game_data->forward_move_root = init_move_tree(p_game_data, forward);
    printf_walk_mv("\n\n");

    /* Move tree initialisation for backward search.                        */
    /* For all spots that are not a target, try to add a backward position. */
    /* find_or_add_position() filters out spots from the same reach.        */
    for (spot = &(p_game_data->spot_pool[0]); spot < p_game_data->pool_ptr; spot++)
    {
        if (spot->object[backward] == not_present)
        {
            p_game_data->johnny = spot;
            new_backward_root = init_move_tree(p_game_data, backward);
            if (new_backward_root)
            {
                printf_walk_mv("  *");

                /* Add the new backward moveroot to the head of the list. */
                /* Important: Don't mark these roots as sibblings (stop conditon of walk_tree()). */
                P_MN(new_backward_root)->next.sibbling   = p_game_data->backward_move_root_list;
                p_game_data->backward_move_root_list     = new_backward_root;

            }
            printf_walk_mv("\n");
        }
    }
}


uint32_t init_move_tree(p_game_data_t p_game_data, t_s_dir search_dir)
{
    t_outcome_add_tp    outcome;
    uint32_t            new_root;
    uint32_t            *move_path = NULL;

    outcome = find_or_add_position(p_game_data, search_dir, &move_path);

    printf_walk_mv("   %2ld%c", SPOT_NO(p_game_data->johnny),
        outcome == position_exists_on_same_direction ? '-' : outcome == bingo ? '!' : '+'
    );

    /* If this is an existing position (same search direction, same reach), do nothing. */
    if (outcome == position_exists_on_same_direction)
        return 0;

    /* The position is new, either the first on this search direction, or another backward reach. */

    new_root = new_move_node(p_game_data, search_dir); /* Create new move root.                   */
    *move_path = new_root;                             /* Refer to it from the new position.      */

    /* In the root node, spot number is used for Johhny's start spot instead for the move spot.   */
    P_MN(new_root)->move_data = MV_SET_SPOT_NO( SPOT_NO(p_game_data->johnny));

    // TODO:  Bingo?

    return new_root;
}


void search_forward_and_backward(p_game_data_t p_game_data)
{
    uint32_t backward_root = 0;

    // TODO: Remove the test prints: (But only after having built the removing of dead backward trees.)
    /* Test printing the forward root. */
    printf_walk_mv("\nForward root (reach):");
    printf_walk_mv(" %u", MV_GET_SPOT_NO(P_MN(p_game_data->forward_move_root)->move_data));

    /* Test printing the backward root list. */
    printf_walk_mv("\nBackward root list (reaches):");
    backward_root = p_game_data->backward_move_root_list;
    while (backward_root)
    {
        printf_walk_mv(" %u", MV_GET_SPOT_NO(P_MN(backward_root)->move_data));
        backward_root = P_MN(backward_root)->next.sibbling;
    }
    printf_walk_mv("\n\n");


    int extention = 1;
    int fw_width = 0;
    int bw_width = 0;
    int fw_depth = 0;
    int bw_depth = 0;

    print_interim_header();
    while (extention)
    {
        extention = 0;
        printf_walk_mv("\nDepth %d\n", fw_depth + bw_depth)

        if (fw_width < bw_width)
        {
            printf_walk_mv("\nForward reach: %u\n",
                            MV_GET_SPOT_NO(P_MN(p_game_data->forward_move_root)->move_data));

            walk_tree(p_game_data, p_game_data->forward_move_root,  fw_depth, forward, &extention);
            printf_walk_mv("Extended: %s\n", extention?"yes":"no")

            fw_width = extention;
            fw_depth++;
        }
        else
        {
            backward_root = p_game_data->backward_move_root_list;
            bw_width = 0;
            while (backward_root)
            {
                printf_walk_mv("\nBackward reach: %u\n",
                            MV_GET_SPOT_NO(P_MN(backward_root)->move_data));

                walk_tree(p_game_data, backward_root, bw_depth, backward, &extention);
                printf_walk_mv("Extended: %s\n", extention?"yes":"no")

                backward_root = P_MN(backward_root)->next.sibbling;
                bw_width += extention;
            }
            bw_depth++;
        }
        print_interim_stats(p_game_data, fw_depth, bw_depth, fw_width, bw_width);
    }
    dbg_print_setup(p_game_data);
}


void walk_tree(p_game_data_t p_game_data, uint32_t root, int extend_at, t_s_dir search_dir, int *extention)
{
    uint32_t move = root;
    int depth = 0;

    /* Reset Johnny to the start position of this move tree. */
    p_game_data->johnny = MOVE_SPOT(root);

    while (1)
    {
        while ( P_MN(move)->child )
            descend(p_game_data, &move, &depth, search_dir);

        if (depth == extend_at)
            extend_depth(p_game_data, move, search_dir, extention);

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

    printf_walk_indent(1, 1)

    *move = P_MN(*move)->child;             /* Descend a level deeper in the move tree. */
    (*depth)++;                             /* Update current depth                     */

    /* Apply this move to the warehouse, i.e. move the box. */
    make_move(p_game_data, MOVE_SPOT(*move), MOVE_DIR(*move), search_dir);

    printf_walk_mv("\n");
}


void ascend(p_game_data_t p_game_data, uint32_t *move, int *depth, t_s_dir search_dir)
{
//     uint32_t cleanup = *move;
//
    san_if ( ! P_MN(*move)->next.parent) /* Sanity check: Do we have a parent to ascend to? */
        san_exit( print_error(no_parent) )

    printf_walk_indent(0, -1)

    /* Apply the undoing of this move to the warehouse, i.e. take the last-moved box back. */
    take_back(p_game_data, MOVE_SPOT(*move), MOVE_DIR(*move), search_dir);
    *move = P_MN(*move)->next.parent;       /* Ascend a level up in the move tree. */
    (*depth)--;                             /* Update current depth                */

//     /* Clean up the move we just left if it has no children. */
//     if ( ! P_MN(*move)->child)
//         clean_up_ascending(cleanup, move);
}


void walk_lateral(p_game_data_t p_game_data, uint32_t *move, t_s_dir search_dir)
{
    san_if ( ! P_MN(*move)->next.sibbling) /* Sanity check: Do we have a sibbling to move to? */
        san_exit( print_error(no_sibbling) )

    printf_walk_indent(1, 0)

    /* Apply the undoing of the current move to the warehouse, i.e. take the last-moved box back. */
    take_back(p_game_data, MOVE_SPOT(*move), MOVE_DIR(*move), search_dir);

    *move =  P_MN(*move)->next.sibbling;    /* Move laterally in the move tree. */

    /* Apply the new move to the warehouse, i.e. move the now-current box. */
    make_move(p_game_data, MOVE_SPOT(*move), MOVE_DIR(*move), search_dir);

    printf_walk_mv("\n")

//     /* Clean up the move we just left if it has no children. */
//     if ( ! P_MN(*move)->child)
//         clean_up_walking_lateral(cleanup, move);
}


void extend_depth(p_game_data_t p_game_data, uint32_t move, t_s_dir search_dir, int *extention)
{
    p_spot johnny;
    t_mv_dir mv_dir;

    johnny = p_game_data->johnny;

    explore_for_reach(p_game_data, johnny, search_dir);

    printf_walk_indent(0, 1)

    while (johnny) /* For all spots within reach: */
    {
        for (mv_dir=right; mv_dir<=down; mv_dir++) /* For all move directions: */
        {
            if (test_move(johnny, mv_dir, search_dir))
            {
                printf_walk_indent(1, 0)

                make_move(p_game_data, johnny, mv_dir, search_dir);
                consider(p_game_data, move, johnny, mv_dir, search_dir, extention);
                take_back(p_game_data, johnny, mv_dir, search_dir);
            }
        }
        johnny = johnny->explore_reach_list;
    }

    printf_walk_indent(0, -1)

    return;
}


void explore_for_reach(p_game_data_t p_game_data, p_spot johnny, t_s_dir search_dir)
{
    t_mv_dir dir = right;
    uint32_t mark;
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


void consider(p_game_data_t p_game_data, uint32_t parent, p_spot johnny, t_mv_dir mv_dir, t_s_dir search_dir, int *extention)
{
    t_outcome_add_tp outcome;
    uint32_t new_move, *move_path;

    outcome = find_or_add_position(p_game_data, search_dir, &move_path);

    printf_walk_mv("%s", outcome==position_is_new ? "new" : outcome==bingo ? "bingo" : "exists\n")

    /* If the new move leads to a prevously existing position, do nothing. */
    if (outcome == position_exists_on_same_direction)
        return;

    /* The move appears to be relevant. */
    new_move = new_move_node(p_game_data, search_dir);       /* Create new move node.               */
    add_move(p_game_data, parent, johnny, mv_dir, new_move); /* Add the new move to the move tree.  */

    /* If the new move leads to the solution, call it out and quit. */
    if (outcome == bingo)
        print_solution(p_game_data, new_move, *move_path, search_dir);

    /* So no bingo; just a new position. */
    *move_path = new_move;  /* Refer to the new move from the new position. */
    *extention += 1;        /* Notify that the move tree was extended by at least one new move, and count extentions. */

    printf_walk_mv("\n");
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

    P_MN(sibbling)->move_data     |= MV_SET_HAS_SIBB;    /* The up-to-now youngest child gets a sibbling. */
    P_MN(sibbling)->next.sibbling  = new_move;           /* Link the new child up to his older sibblings. */
    return;
}


// /*    pre-assured:
//  *      move exists
//  *      parent exists and is move's parent
//  *      move has no sibbling
// */
// void clean_up_ascending(uint32_t move, uint32_t parent);
// {
//     uint32_t sibbling;
//
//     if ( P_MN(parent)->child == move)       /* If parent'child is our cleanup move              */
//         P_MN(parent)->child = 0;            /*      then all there is it no longer has one.     */
//     else
//     {
//         sibbling = P_MN(parent)->child;                 /* First goto the oldest sibbling.      */
//         while (P_MN(sibbling)->next.sibbling != move)   /* and find our next-older sibbling     */
//             sibbling = P_MN(sibbling)->next.sibbling;
//
//                                                         /* He is now the youngest, so:          */
//         P_MN(sibbling)->move_data &= ~MV_SET_HAS_SIBB;  /*      - has no sibbling any more      */
//         P_MN(sibbling)->next.parent == parent;          /*      - he now refers to his parent   */
//     }
//
//     //    clean_up_move(move);
// }
//
//
//     while ( MV_GET_HAS_SIBB(P_MN(sibbling)->move_data) ) /* Then goto the youngest sibbling. */
//         sibbling = P_MN(sibbling)->next.sibbling;
//
//     P_MN(sibbling)->move_data     |= MV_SET_HAS_SIBB;    /* The up-to-now youngest child gets a sibbling. */
//     P_MN(sibbling)->next.sibbling  = new_move;           /* Link the new child up to his older sibblings. */
//     return;
//
//
//
// /*    pre-assured:
//  *      move exists
//  *      sibbling exists and  is move's next younger older sibbling
// */
// void clean_up_walking_lateral(uint32_t move, uint32_t sibbling);
// {
//     /* find parent */
//     while (sibbling has sibbilg
//         sibbing is next sibblinh
//     parent = sibbling's parent;
//
//     if (parent.child == move)
//     {
//         parent.child = sibbling;
//     }
//     else
//     {
//         goto oldest sibbling
//         while sibbling.next != move
//             goto next sibbling
//
//         sibbling.next == sibbling
//     }
//
//     clean_up_move(move);
// }


#ifdef DBG_WALK_MV
void printf_walk_indent_f(int print, int change)
{
    static int indent = 0;
    int i;

    indent += change;

    if (print)
        for (i=0; i<indent; i++)
            printf(".   ");
}
#endif















