#ifndef SOKOBAN2_H
#define SOKOBAN2_H

#include <stdint.h>
#include <stdbool.h>


/* The amout of memory to allocate for the move tree and position tree: */
// #define TREE_MEMORY (2048)
// #define TREE_MEMORY (0x000100000) /*  1    Mbytes. */
// #define TREE_MEMORY (0x040000000) /*  1.07 Gbytes. */
// #define TREE_MEMORY (0x080000000) /*  2.15 Gbytes. */
// #define TREE_MEMORY (0x100000000) /*  4.29 Gbytes. */
// #define TREE_MEMORY (0x180000000) /*  6.44 Gbytes. */
// #define TREE_MEMORY (0x1C0000000) /*  7.52 Gbytes. */
// #define TREE_MEMORY (0x200000000) /*  8.59 Gbytes. */
// #define TREE_MEMORY (0x240000000) /*  9.66 Gbytes. */
//#define TREE_MEMORY (0x280000000) /* 10.74 Gbytes. */
#define TREE_MEMORY (0x2C0000000) /* 11.81 Gbytes. */
// #define TREE_MEMORY (0x300000000) /* 12.88 Gbytes. */

// #define DBG_SANITY

#define NR_OF_SPOTS (50*50)
#define SPOT_NO(p_spot) ((p_spot) - p_game_data->spot_pool)

// typedef enum {false = 0, true = 1} t_bool;
typedef enum {right=0, up=1, left=2, down=3} t_mv_dir;
typedef enum {box=0, forward=0, target=1, backward=1} t_s_dir; /* Object type and search direction, tied. */
typedef enum {not_present=0, present=1} t_object_presence;     /* Whether a box or target is present.     */

struct spot
{
    struct spot *neighbour[4];       /* Connects to the neighbours, right, up, left and down.       */

    struct spot *explore_reach_list; /* For exploring johnny's current reach, when searching moves. */
    struct spot *other_reach_list;   /* For exploring johnny's current reach, all other purposes.   */
    struct spot *position_list;      /* Link to next spot in the position base list. A value of     */
                                     /* HARDNOGO indicates this is a hard no-go spot.               */

    t_object_presence object[2];     /* [0]: Whether spot has a box; [1]: Whether spot is a target. */

    uint32_t reach_mark;             /* For exploring johnny's current reach, all purposes. */
    int position;                    /* For setup parsing only.  */
};

typedef struct spot *p_spot;


struct game_data
{
    struct spot spot_pool[NR_OF_SPOTS]; /* The pool of spots. Statically allocated. */
    struct spot *pool_ptr;              /* Points just behind the last spot in use. */
    struct spot *johnny;

    /* struct spot *position_list;
     *      Head of linked list: the sub set of spots that are relevant for the positions.
     *      Consists of all spots that are nog marked as hardnogo.
     */
    struct spot *position_head;  /* Head of the position base list.          */

    uint32_t position_root;           /* Index of root node of the position tree.            */
    uint32_t forward_move_root;       /* Index of root node of the forward move tree.        */
    uint32_t backward_move_root_list; /* Index of root node of the first backward move tree. */

    uint32_t next_reach;    /* For exploring johnny's current reach. */

    char *p_memory_start;   /* Pointer to the memory allocated for the move trees and position tree.      */
    char *p_memory_bottom;  /* Pointer to free tree memory, from the bottom up.                           */
    char *p_memory_top;     /* Pointer to free tree memory, from the top down.                            */
                            /* Note: p_memory_bottom points to the first free location while p_memory_top */
                            /* points to the first non-free location after the free space.                */

    uint32_t move_pool;     /* Pointer to pool of previously used but returned move nodes. */

    /* Statistics: */
    int fw_move_count;      /* Amount of forward move  nodes used. */
    int bw_move_count;      /* Amount of backward move nodes used. */
    int tp_node_count;      /* Amount of position nodes used.      */
    int tp_leaf_count;      /* Amount of position leafs used.      */

    int cleanups;           /* Amount of move nodes that are cleaned up on the go. TODO: remove this again.    */
    int reuses;             /* Amount of move nodes that are reusedup on the go. TODO: remove this again.    */

    /* Flags: */
    int no_solution_print;  /* Omit the printing of the solution. */

    int no_true_blockers;
    int no_false_blockers;

};

typedef struct game_data *p_game_data_t;

#ifdef DBG_SANITY
#define san_if(condition) if(condition)
#define san_exit(value) exit(value);
#else
#define san_if(condition)
#define san_exit(value)
#endif


#endif /* SOKOBAN2_H */









