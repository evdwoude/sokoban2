#ifndef SOKOBAN2_H
#define SOKOBAN2_H

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
    int spot_number;    /* For identifying reaches and for debug printing. */
    int position;       /* For setup parsing only.  */
};

typedef struct spot *p_spot;

#define HARDNOGO ((p_spot) -1)

#define NR_OF_SPOTS (50*50)

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

#endif /* SOKOBAN2_H */

