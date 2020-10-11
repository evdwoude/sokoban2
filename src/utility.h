#ifndef UTILITYH
#define UTILITYH


/* In order to intermix 8- and 12-byte memory blocks and address them with a unified index, we using unified a
 * reference unit for indxing of 4 bytes.
 * With 32-bit wide indexes, this gives a total af 16 Gbyte addressability.  (2^32 * 4).
 * This seems enough for now.
 *
 * (Should we want to expand, we could move away from intermixing different sizes and use larger units of
 * reference, as intermixing forces to use a common denomitor of the sizes.)
 */

#define MEM_REF_UNIT 4         /* Code relies on MEM_REF_UNIT being a power of 2. */
#define MEM_UNIT_COUNT (TREE_MEMORY / MEM_REF_UNIT)

#define MV_NODE_SIZE (3*MEM_REF_UNIT)
#define TP_NODE_SIZE (2*MEM_REF_UNIT)
#define TP_LEAF_SIZE (3*MEM_REF_UNIT)


/* Get the reference (address i.e. pointer value) of a transposition node. */
#define P_TPN(i) ((struct transposition_node *) (p_game_data->p_memory_start + (i) * MEM_REF_UNIT))

/* Get the reference (address i.e. pointer value) of a transposition leaf. */
/* Note that, although the leafs are twice as big as te nodes, their indices are  both based on the */
/* original blocksize: the leaf nodes are guaranteed to be 8 bytes aligned, not 16.                 */
#define P_TPL(i) ((struct transposition_leaf *) (p_game_data->p_memory_start + (i) * MEM_REF_UNIT))


int next_mark(p_game_data_t p_game_data);

skbn_err allocate_memory(p_game_data_t p_game_data);

uint32_t new_transposition_node(p_game_data_t p_game_data);

uint32_t new_transposition_leaf(p_game_data_t p_game_data);

void define_hardnogos(p_game_data_t p_game_data);

int is_hardnogo(struct spot* spot);

#endif /* UTILITYH */
