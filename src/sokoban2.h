#ifndef SOKOBAN2_H
#define SOKOBAN2_H

#include <stdint.h>

#define MEMBLOCK_SIZE   8    /* Code relies on MEMBLOCK_SIZE being a power of 2. */
//#define MEMBLOCK_COUNT  (2^30) /* 1G */
#define MEMBLOCK_COUNT  (20)

#define HARDNOGO ((p_spot) -1)

#define NR_OF_SPOTS (50*50)
#define SPOT_NUMBER(p_spot) ((p_spot) - p_game_data->spot_pool)



typedef enum {right=0, up=1, left=2, down=3} t_direction;

struct spot
{
    struct spot *neighbour[4];
    struct spot *reach_chain;
    struct spot *transposition_list;    /* Link to next spot in the transposition base list. */
                                        /* A value of HARDNOGO indicates this is a hard no-go spot. */
    int has_box;
    int is_target;
    int reach_mark;     /* For exploring johnny's current reach. */
    int position;       /* For setup parsing only.  */
};

typedef struct spot *p_spot;


struct game_data
{
    struct spot spot_pool[NR_OF_SPOTS];
    struct spot *pool_ptr;
    struct spot *johnny;

    /* struct spot *transposition_list;
     *      Head of linked list: the sub set of spots that are relevant for the transpositions.
     *      Consists of all spots that are nog marked as hardnogo.
     */
    struct spot *transposition_head;                /* Head of the transposition base list.        */

    uint32_t forward_transposition_root;            /* Root node of the forward transpoition tree. */

    int next_reach; /* For exploring johnny's current reach. */

    void *p_memory_start;   /* Pointer to the memory allocated for the move trees and transposition tree. */
    void *p_memory_bottom;  /* Pointer to free tree memory, from the bottom up.                           */
    void *p_memory_top;     /* Pointer to free tree memory, from the top down.                            */
                            /* Note: p_memory_bottom points to the first free location while p_memory_top */
                            /* points to the first non-free location after the free space.                */

    /* Statitics: */
    int fw_move_count;  /* Amount of forward move  nodes used. */
    int bw_move_count;  /* Amount of backward move nodes used. */
    int tp_node_count;  /* Amount of transposition nodes used. */
    int tp_leaf_count;  /* Amount of transposition leafs used. */

};

typedef struct game_data *p_game_data_t;

#endif /* SOKOBAN2_H */








