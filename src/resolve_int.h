#ifndef RESOLVE_INTH
#define RESOLVE_INTH


/* General definitions */

#define FORWARD  0
#define BACKWARD 1

#define MAX_DEPTH 400
#define WALKING_STACK_SIZE  (MAX_DEPTH + 2)  /* In fact "+ 1" should be enough, but hey... */

/* Bit fields for move node's member move_data: */
#define SPOT_INDEX      0x1FFFFFFF
#define MOVE_DIR        0x60000000
#define HAS_SIBBLING    0x80000000


/* TODO (Document):
 *
 * Remove the below text and merge into documentation
/* We use "move" when referring to a game move. I.e. the moving of Johhny,
 * the floor manager, or the boxes, to an adjecent spot in the warehouse.
 */

/* We use "walk" when referring to traversing the move tree. I.e
 * when walking the move pointer though the move tree, simulteneously
 * moving the man and the boxes across the warehouse floor to keep
 * the position synchronised with the position in the move tree.
 */

struct move_node
{
    uint32_t child;
    union {
        uint32_t sibbling;
        uint32_t parent;
    } next;

    /* unt32_t move_data is a packed set of data fields:
     *  spot,           29 bits: The spot where Johnny is right before the Move (either backward or forward).
     *  move_direction,  2 bits: The direction of the move: Right, Up, Left or Down.
     *  has_sibbling,    1 bit:  Tells us whether next refers to the next sibbling ot the parent;
     *
     *  We're not indicating whether the a child value of 0 indicates a dead end or undiscovered moves due to
     *  the current search depth; We use a separate, global depth indicator for that.
     */
    uint32_t move_data;
};


struct position_node
{
    uint32_t spot_has_box;
    uint32_t spot_is_empty;
};

struct position_leaf
{
    /* unt32_t leaf_data is a packed set of data fields:
     *  reach_ident,      31 bits: Completes the identification of the this position.
     *  search_direction,  1 bits: The search direction on with this position has been found, so it
     *                             indicates whether move_path refers to a forward move node or a backward
     *                             mode node. Values: 0: FORWARD, 1: BACKWARD.
     */
    uint32_t leaf_data;  /* Completes the identification of the this position.                              */
    uint32_t next_leaf;  /* Linked list of different position for the same box arrangement.                 */
    uint32_t move_path;  /* Refer back to the originating move node for either forward- or backward search. */
};

/* Defines for leaf_data manipulations: */
#define TPL_REACH_MASK          0x7FFFFFFF             /* Mask to read the reach identifier.            */
#define TPL_SEARCH_DIR_MASK     0x80000000             /* Mask to read the search direction.            */
#define TPL_FORWARD             0                      /* Mask to write the search direction forward.   */
#define TPL_BACKWARD            TPL_SEARCH_DIR_MASK    /* Mask to write the search direction backward.  */
#define TPL_REACH(data)         ((data) & TPL_REACH_MASK)                            /* Get reach ident */
#define TPL_SEARCH_DIR(data)    ((data) & TPL_SEARCH_DIR_MASK ? BACKWARD : FORWARD)  /* Get search dir  */


#endif /* RESOLVE_INTH */




