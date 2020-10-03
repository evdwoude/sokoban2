#ifndef UTILITYH
#define UTILITYH

/* Include:
 *      sokoban2.h
            struct game_data
 */



/* Get the reference (address i.e. pointer value) of a transposition node. */
#define P_TPN(i) ((struct transposition_node *) (p_game_data->p_memory_start + ((i)-1) * MEMBLOCK_SIZE))

/* Get the reference (address i.e. pointer value) of a transposition leaf. */
/* Note that, although the leafs are twice as big as te nodes, their indices are  both based on the */
/* original blocksize: the leaf nodes are guaranteed to be 8 bytes aligned, not 16.                 */
#define P_TPL(i) ((struct transposition_leaf *) (p_game_data->p_memory_start + ((i)-1) * MEMBLOCK_SIZE))


typedef enum {not_yet = 0, got_it = 1} t_got_it;
typedef enum {false = 0, true = 1} t_bool;

int next_mark(p_game_data_t p_game_data);

skbn_err allocate_memory(p_game_data_t p_game_data);

uint32_t new_transposition_node(p_game_data_t p_game_data);

uint32_t new_transposition_leaf(p_game_data_t p_game_data);

void define_hardnogos(p_game_data_t p_game_data);

int is_hardnogo(struct spot* spot);

#endif /* UTILITYH */
