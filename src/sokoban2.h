#ifndef SOKOBAN2_H
#define SOKOBAN2_H

#include <stdint.h>

/* The amout of memory to allocate for the move tree and position tree: */
// #define TREE_MEMORY (2^30) /* 1 Gbytes. */
// #define TREE_MEMORY (2^32) /* 4 Gbytes. */
#define TREE_MEMORY (2048)

#define NR_OF_SPOTS (50*50)
#define SPOT_NUMBER(p_spot) ((p_spot) - p_game_data->spot_pool)

typedef enum {false = 0, true = 1} t_bool;
typedef enum {right=0, up=1, left=2, down=3} t_direction;

struct spot
{
    struct spot *neighbour[4];
    struct spot *explore_reach_list; /* For exploring johnny's current reach, when searching moves. */
    struct spot *other_reach_list;   /* For exploring johnny's current reach, all other purposes.    */
    struct spot *position_list;      /* Link to next spot in the position base list. A value
                                      * of HARDNOGO indicates this is a hard no-go spot. */
    int has_box;
    int is_target;
    int reach_mark;     /* For exploring johnny's current reach, all purposes. */
    int position;       /* For setup parsing only.  */
};

typedef struct spot *p_spot;


struct game_data
{
    struct spot spot_pool[NR_OF_SPOTS];
    struct spot *pool_ptr;
    struct spot *johnny;

    /* struct spot *position_list;
     *      Head of linked list: the sub set of spots that are relevant for the positions.
     *      Consists of all spots that are nog marked as hardnogo.
     */
    struct spot *position_head; /* Head of the position base list.   */

    uint32_t position_root;     /* Root node of the poition tree.    */

    int next_reach;         /* For exploring johnny's current reach. */

    void *p_memory_start;   /* Pointer to the memory allocated for the move trees and position tree.      */
    void *p_memory_bottom;  /* Pointer to free tree memory, from the bottom up.                           */
    void *p_memory_top;     /* Pointer to free tree memory, from the top down.                            */
                            /* Note: p_memory_bottom points to the first free location while p_memory_top */
                            /* points to the first non-free location after the free space.                */

    /* Statistics: */
    int fw_move_count;      /* Amount of forward move  nodes used. */
    int bw_move_count;      /* Amount of backward move nodes used. */
    int tp_node_count;      /* Amount of position nodes used.      */
    int tp_leaf_count;      /* Amount of position leafs used.      */

};

typedef struct game_data *p_game_data_t;

#endif /* SOKOBAN2_H */









