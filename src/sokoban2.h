#ifndef SOKOBAN2_H
#define SOKOBAN2_H

#include <stdint.h>


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

#define HARDNOGO ((p_spot) -1)

#define NR_OF_SPOTS (50*50)
#define SPOT_NUMBER(p_spot) ((p_spot) - p_game_data->spot_pool)

struct game_data
{
    struct spot spot_pool[NR_OF_SPOTS];
    struct spot *pool_ptr;
    struct spot *johnny;

    /* struct spot *transposition_list;
     *      Head of linked list: the sub set of spots that are relevant for the transpositions.
     *      Consists of all spots that are nog marked as hardnogo.
     */
    struct spot *transposition_head; /* Head of the transposition base list. */

    int next_reach;   /* For exploring johnny's current reach. */
};

typedef struct game_data *p_game_data_t;


#endif /* SOKOBAN2_H */








