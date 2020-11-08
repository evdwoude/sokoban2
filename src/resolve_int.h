#ifndef RESOLVE_INTH
#define RESOLVE_INTH


/* General definitions */

#define MAX_DEPTH 400
#define WALKING_STACK_SIZE  (MAX_DEPTH + 2)  /* In fact "+ 1" should be enough, but hey... */

/* Bit fields for move node's member move_data: */
#define SPOT_INDEX      0x1FFFFFFF
#define MOVE_DIR        0x60000000
#define HAS_SIBBLING    0x80000000


/* Change the search direction to the opposite. I.e, forward becomes backward v.v. */
// #define OPPOSITE_SDIR(search_dir) (1 - (search_dir))

/* Take the opposte of the move direction. I.e, "right" for "left", "up" for "down" and both v.v. */
#define OPPOSITE_MV_DIR(move_dir) ((move_dir) ^ 2)

/* TODO (Document):
 *
 * Remove the below text and merge into documentation
/* We use "move" when referring to a game move. I.e. the moving of Johhny,
 * the floor manager, or the boxes, to an adjecent spot in the warehouse.
 */


// TODO: move the below to "documentation"
/* We use "walk" when referring to traversing the move tree. I.e
 * when walking the move pointer though the move tree, simulteneously
 * moving the man and the boxes across the warehouse floor to keep
 * the position synchronised with the position in the move tree.
 */


//  TODO: Document
struct position_node
{
    uint32_t spot_has_object;
    uint32_t spot_is_empty;
};


//  TODO: Document
struct position_leaf
{
    /* unt32_t leaf_data is a packed set of data fields:
     *  reach_ident,      31 bits: Completes the identification of the this position.
     *  search_direction,  1 bits: The search direction on with this position has been found, so it
     *                             indicates whether move_path refers to a forward move node or a backward
     *                             mode node. Values: forward, backward.
     */
    uint32_t leaf_data;  /* Completes the identification of the this position.                              */
    uint32_t next_leaf;  /* Linked list of different position for the same box arrangement.                 */
    uint32_t move_path;  /* Refer back to the originating move node for either forward- or backward search. */
};

/* Defines and macros for position leaf's leaf_data manipulations: */
/* Basic masks: */
#define TPL_REACH_MASK          0x7FFFFFFF             /* Mask for the reach identifier.                */
#define TPL_SEARCH_DIR_MASK     0x80000000             /* Mask for the search direction.                */
/* To write: */
#define TPL_FORWARD             0                      /* Value to write the search direction forward.  */
#define TPL_BACKWARD            TPL_SEARCH_DIR_MASK    /* Value to write the search direction backward. */
/* To Read: */
#define TPL_REACH(data)         ((data)&TPL_REACH_MASK)                            /* Get reach ident   */
#define TPL_SEARCH_DIR(data)    ((data)&TPL_SEARCH_DIR_MASK ? backward : forward)  /* Get search dir    */


struct move_node
{
    uint32_t child;
    union {
        uint32_t sibbling;
        uint32_t parent;
    } next;

    /* unt32_t move_data is a packed set of data fields:
     *  move_direction,  2 bits: The direction of the move: Right, Up, Left or Down.
     *  spot number,    29 bits: The spot where Johnny is before the move is made (F/B).
     *  has_sibbling,    1 bit:  Whether 'next' refers to the next sibbling. Refers to parent otherwise.
     *
     *  We're not indicating whether the a child value of 0 indicates a dead end or undiscovered moves due to
     *  the current search depth; We use a separate, global depth indicator for that.
     */
    uint32_t move_data;
};

/* Defines and macros for move node's move_data manipulations: */
/* Basic masks: */
#define MV_MOVE_DIR_MASK        0x00000003             /* Mask for the move direction.                    */
#define MV_SPOT_NO_MASK         0x7FFFFFFC             /* Mask for the spot number.                       */
#define MV_HAS_SIBB_MASK        0x80000000             /* Mask for the idicaation of a present sibbling.  */
/* Well, eh, util?: */
#define MV_SPOT_NO_SHIFT        2                      /* Amount of bits to shift the spot nunber (L/R).  */
/* To write: */
#define MV_SET_SPOT_NO(spotno)  ((spotno) << MV_SPOT_NO_SHIFT) /* Used to set the spot number.            */
#define MV_SET_HAS_SIBB         MV_HAS_SIBB_MASK               /* Used to set the has sibbling indicator. */
/* To Read: */
#define MV_GET_MOVE_DIR(data)    ((data)&MV_MOVE_DIR_MASK)                     /* Get the move direction. */
#define MV_GET_SPOT_NO(data)    (((data)&MV_SPOT_NO_MASK) >> MV_SPOT_NO_SHIFT) /* Get the spot number.    */
#define MV_GET_HAS_SIBB(data)    ((data)&MV_HAS_SIBB_MASK ? 1 : 0)             /* Has a sibbling?         */


#endif /* RESOLVE_INTH */




