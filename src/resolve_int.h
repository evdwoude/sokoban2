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


/* TODO:
 *
 * Remove the below text and merge into documentation
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
    uint32_t child;
    union {
        uint32_t sibbling;
        uint32_t parent;
    } next;

    /* unt32_t move_data is a packed set of data fields:
     *  spot,           29 bits: The spot where Johnny is right before the Move (either backward or forward).
     *  move_direction,  2 bits: The diretion of the move: Right, Up, Left or Down.
     *  has_sibbling,    1 bit:  Tells us whether next refers to the next sibbling ot the parent;
     *
     *  We're not using a it to indicate whether the child of NULl indicates a dead end or is just undiscovered
     * due to the current search depth; We use a separate, global depth indicator for that/
     */
    uint32_t move_data;
};


struct transposition_node
{
    uint32_t spot_has_box;
    uint32_t spot_is_empty;
};

struct transposition_leaf
{
    uint32_t reach_ident;  /* Completes the identification of the this transposition. */
    uint32_t next_leaf;    /* Linked list of different transposition for the sae box positioning. */
    uint32_t move_path[2]; /* Refer back to the move nodes for forward- and backward search. */
};

#endif /* RESOLVE_INTH */




