#ifndef SOKOBAN2_H
#define SOKOBAN2_H


/* We use "move" when referring to a game move. I.e. the moving of
 * Johnny, the floor manager to an adjecent spot in the warehouse. */
typedef enum {mv_right=0, mv_up=1, mv_left=2, mv_down=3} t_move_direction;

/* We use "step" end "walk" when referring to traversing the move tree. */

struct spot
{
    struct spot *neighbour[4];
    int has_box;
    int is_target;
    int no_go;
    int has_box_bw;
    int is_target_bw;
    int position;       /* For setup parsing only.  */
    int spot_number;    /* For debug printing only. */
};

typedef struct spot *p_spot;

#define NR_OF_SPOTS (50*50)

struct game_data
{
    struct spot spot_pool[NR_OF_SPOTS];
    struct spot *pool_ptr;
    struct spot *man;
    int number_of_boxes;
    int number_of_boxes_on_target;
};

#endif /* SOKOBAN2_H */
