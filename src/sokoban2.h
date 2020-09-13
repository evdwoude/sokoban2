#ifndef SOKOBAN2_H
#define SOKOBAN2_H

typedef enum {right=0, up=1, left=2, down=3} t_direction;

struct spot
{
    struct spot *neighbour[4];
    struct spot *reach_chain;
    int has_box;
    int is_target;
    int no_go;
    int reach_mark;     /* For exploring johnny's current reach. */
    int position;       /* For setup parsing only.  */
    int spot_number;    /* For debug printing only. */
};

typedef struct spot *p_spot;

#define NR_OF_SPOTS (50*50)

struct game_data
{
    struct spot spot_pool[NR_OF_SPOTS];
    struct spot *pool_ptr;
    struct spot *johnny;
    int next_reach;   /* For exploring johnny's current reach. */
};

#endif /* SOKOBAN2_H */

