#ifndef RESOLVE_INTH
#define RESOLVE_INTH

/* General definitions */

#define MAX_DEPTH 400
#define WALKING_STACK_SIZE  (MAX_DEPTH + 2)  /* In fact "+ 1" should be enough, but hey... */


/* We use "move" when referring to a game move. I.e. the moving of Johhny,
 * the floor manager, or the boxes, to an adjecent spot in the warehouse.
 */

/* We use "walk" when referring to traversing the move tree. I.e
 * when walking the move pointer though the move tree, simulteneously
 * moving the man and the boxes across the warehouse floor to keep
 * the transposition synchronised with the position in the move tree.
 */

struct move_node
{
    struct move_node *links[5];
//    t_walk_direction arriving_direction; //     t_move_direction arriving_direction;
    int pushed_a_box;
};
typedef struct move_node *p_move_node;

struct bwmove_node
{
    struct bwmove_node *links[9];
//    t_bwwalk_direction arriving_direction; //     t_bwmove_direction arriving_direction;
    int pulled_a_box;   // TODO: may not be required; in the backward search case, the direction indicates this.
};
typedef struct bwmove_node *p_bwmove_node;

struct ll_root_bwmove_node
{
    struct ll_root_bwmove_node *next;
    struct spot* start_position;
    struct bwmove_node *bwroot_node;
    int found_new;
};
typedef struct ll_root_bwmove_node *p_ll_root_bwmove_node;

union transposition_node
{
    struct
    {
        union transposition_node *spot_has_box;
        union transposition_node *spot_has_man;
        union transposition_node *spot_is_empty;
    } node;
    struct
    {
        p_move_node forward_path;
        p_bwmove_node backward_path;
    } move_path;
};
typedef union transposition_node *p_transposition_node;

#endif /* RESOLVE_INTH */
