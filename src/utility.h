#ifndef UTILITYH
#define UTILITYH


/* In order to intermix 8- and 12-byte memory blocks and address them with a unified index, we using unified a
 * reference unit for indxing of 4 bytes.
 * With 32-bit wide indexes, this gives a total af 16 Gbyte addressability.  (2^32 * 4,  "^": power.)
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


/* Get the reference (address i.e. pointer value) of a position node. */
#define P_TPN(i) ((struct position_node *) (p_game_data->p_memory_start + ((uint64_t)(i)) * ((uint64_t)MEM_REF_UNIT)))

/* Get the reference (address i.e. pointer value) of a position leaf. */
#define P_TPL(i) ((struct position_leaf *) (p_game_data->p_memory_start + ((uint64_t)(i)) * ((uint64_t)MEM_REF_UNIT)))

/* Get the reference (address i.e. pointer value) of a move node. */
#define P_MN(i) ((struct move_node *) (p_game_data->p_memory_start + ((uint64_t)(i)) * ((uint64_t)MEM_REF_UNIT)))

void reinit_mark(p_game_data_t p_game_data, uint32_t initialiser);

uint32_t next_mark(p_game_data_t p_game_data);

skbn_err allocate_memory(p_game_data_t p_game_data);

uint32_t new_position_node(p_game_data_t p_game_data);

uint32_t new_position_leaf(p_game_data_t p_game_data);

uint32_t new_move_node(p_game_data_t p_game_data, t_s_dir search_dir);

void define_hardnogos(p_game_data_t p_game_data);

int is_hardnogo(struct spot* spot);

char mv_dir_name(t_mv_dir move_direction);

void dbg_print_setup(p_game_data_t p_game_data);

void print_stats(p_game_data_t p_game_data);

#endif /* UTILITYH */
