#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "sokoban2.h"
#include "error.h"
#include "resolve_int.h"
#include "utility.h"

/* Local defs */

/* Conditional compile switches  */

#define DBG_MARK 1          // Prints marker wrappings
#define DBG_HARDNOGOS 1     // Prints the search resul for hard no-go's.
//#define DBG_C 1           // Prints
//#define DBG_D 1           // Prints

/* Conditional compile 'fabric' */


#ifdef DBG_MARK
#define printf_mark(...) printf(__VA_ARGS__);
#else
#define printf_mark(...)
#endif

#ifdef DBG_HARDNOGOS
#define printf_hardnogo(...) printf(__VA_ARGS__);
#define dbg_hardnogo_if(condition) if(condition)
#define dbg_hardnogo_for(condition) for(condition)
#else
#define printf_hardnogo(...)
#define dbg_hardnogo_if(condition)
#define dbg_hardnogo_for(condition)
#endif


/* Internal protos */

void scan_line_for_hardnogos(p_game_data_t p_game_data, struct spot* spot, int main_dir, int *new_hardnogos);
void set_as_hardnogo(struct spot* spot);

/* Exported data */

/* Imported data */

/* Local data */

/* Code */


/*int next_mark(p_game_data_t p_game_data)
 *
 * Used for explore functions.
 *
 * Increments the global spot mark to mark spot as newly found during a current eploration. This way,
 * the spot marks do not need to be re-initialized every new eploration. The reinitilization is still
 * required (and automatically performed) when the globla spot mark wraps. Reinitilization is also
 * required before searching for the amount of domains a backward search can start from.
 */
int next_mark(p_game_data_t p_game_data)
{
    p_spot spot;

    p_game_data->next_reach++;
    if (p_game_data->next_reach > 3)
    {
        printf_mark("\nReinit marks...")

        /* If next_reach wrapped, reinitiae all spots. */
        for (spot = &(p_game_data->spot_pool[0]); spot < p_game_data->pool_ptr; spot++)
        {
            spot->reach_mark = 0;
            printf_mark(" %ld", SPOT_NUMBER(spot))
        }
        printf_mark("\n")

        /* And reinitialize next_reach as zero + 1. */
        p_game_data->next_reach = 1;
    }

    return p_game_data->next_reach;
}


skbn_err allocate_memory(p_game_data_t p_game_data)
{
    /* Sanity checks for the way we'er goona use the tree memory. */
    if (sizeof(struct transposition_node) != MEMBLOCK_SIZE)
        return print_error(transposition_node_size, MEMBLOCK_SIZE);

    if (sizeof(struct transposition_leaf) != 2*MEMBLOCK_SIZE)
        return print_error(transposition_leaf_size, 2*MEMBLOCK_SIZE);

    /* Allocate space for the trees. */
    p_game_data->p_memory_start = calloc(MEMBLOCK_COUNT, MEMBLOCK_SIZE);

    if ( ! p_game_data->p_memory_start)
        return print_error(cant_allocate_memory);

    if ((long int) p_game_data->p_memory_start & MEMBLOCK_SIZE-1) /* MEMBLOCK_SIZE is a power of 2. */
    {
        free(p_game_data->p_memory_start);
        return print_error(memory_not_aligned, MEMBLOCK_SIZE);
    }
    p_game_data->p_memory_bottom = p_game_data->p_memory_start;
    p_game_data->p_memory_top  =   p_game_data->p_memory_start + MEMBLOCK_COUNT * MEMBLOCK_SIZE;

    return no_error;
}


uint32_t new_transposition_node(p_game_data_t p_game_data)
{
    uint32_t index;

    if (p_game_data->p_memory_bottom + MEMBLOCK_SIZE > p_game_data->p_memory_top)
        exit( print_error(out_of_tree_memory) );

    index = (uint32_t) ((p_game_data->p_memory_bottom - p_game_data->p_memory_start) / MEMBLOCK_SIZE);
//     printf("[%d]", index);

    p_game_data->p_memory_bottom += MEMBLOCK_SIZE;
    return index;
}

uint32_t new_transposition_leaf(p_game_data_t p_game_data)
{
    uint32_t index;

    if (p_game_data->p_memory_bottom + 2 * MEMBLOCK_SIZE > p_game_data->p_memory_top)
        exit( print_error(out_of_tree_memory) );

    index = (uint32_t) ((p_game_data->p_memory_bottom - p_game_data->p_memory_start) / MEMBLOCK_SIZE);
//     printf("[[%d]]", index);

    p_game_data->p_memory_bottom += 2 * MEMBLOCK_SIZE;
    return index;
}


/* void define_hardnogos(p_game_data_t p_game_data);
 *
 * Find and mark all spots where placing a box would always and immediately result in a dead end.
 * This includes the dead ends that result from individual boxes put on specific, spots, identifiable
 * from the static setup (i.e. the start position).
 * It excludes all dead ends that involve multiple boxes. These dead ends are more complex to find and
 * can't be idenfified up-front (staticallly), but only on-the-go, which is quite expensive.
 */
void define_hardnogos(p_game_data_t p_game_data)
{
    struct spot* spot;
    int new_hardnogos = 1;

    /* First find all spots that are adjent to minimal two non-oppsing walls. */
    printf_hardnogo("\n\nCorners:\n")
    for (spot = &(p_game_data->spot_pool[0]); spot < p_game_data->pool_ptr; spot++)
    {
        if (    !spot->is_target
            && (    (spot->neighbour[right] == NULL  &&  spot->neighbour[up   ] == NULL)
                ||  (spot->neighbour[up   ] == NULL  &&  spot->neighbour[left ] == NULL)
                ||  (spot->neighbour[left ] == NULL  &&  spot->neighbour[down ] == NULL)
                ||  (spot->neighbour[down ] == NULL  &&  spot->neighbour[right] == NULL) ) )
        {
            set_as_hardnogo(spot);

            printf_hardnogo(" %ld", SPOT_NUMBER(spot))
        }
    }

    /* TODO: find what here ?
     * Scan the setup multiple times: new found hard nogos can trigger the finding of more hard nogos.
     * Keep re-scanning until no new hard nogos are found. (The only indication that another round will
     * not find more hard no gos is when the previous round did not find any) */
    printf_hardnogo("\nIn betweens:")
    while (new_hardnogos)
    {
        new_hardnogos = 0;
        printf_hardnogo("\n")

        /* From every spot that is marked as hard nogo, do a horizontal and vertical line scan. */
        for (spot = &(p_game_data->spot_pool[0]); spot < p_game_data->pool_ptr; spot++)
            if (is_hardnogo(spot))
            {
                scan_line_for_hardnogos(p_game_data, spot, right, &new_hardnogos); /* Horizontal line scan. */
                scan_line_for_hardnogos(p_game_data, spot,  down, &new_hardnogos); /* Vertical line scan.   */
            }
    }
}


/* TODO: Document. */
int is_hardnogo(struct spot* spot)
{
    return spot->transposition_list == HARDNOGO;
}


void scan_line_for_hardnogos(p_game_data_t p_game_data, struct spot* spot, int main_dir, int *new_hardnogos)
{
    struct spot* scan;

    int sub_dir_a = left;
    int sub_dir_b = right;

    if (main_dir == right || main_dir == left)
    {
        sub_dir_a = up;
        sub_dir_b = down;
    }

    /* Scan line for any spot that can not (yet) be regarded as a hard no go */
    for (scan = spot->neighbour[main_dir]; scan && ! is_hardnogo(scan); scan = scan->neighbour[main_dir])
        if (scan->is_target || (scan->neighbour[sub_dir_a] != NULL && scan->neighbour[sub_dir_b] != NULL))
            return;

    /* Apparently, there is (at least one) new hard no go on this line, so just mark them all. (Except when
     * the line length is zero (exluding start/end), in which case the for loop below will not enter and
     * thus correctly omitting the marking of new_hardnogos. */
    for (scan = spot->neighbour[main_dir]; scan && ! is_hardnogo(scan); scan = scan->neighbour[main_dir])
    {
        dbg_hardnogo_if( ! is_hardnogo(scan))
            printf_hardnogo(" %ld", SPOT_NUMBER(scan))

        set_as_hardnogo(scan);
        *new_hardnogos  = 1;
    }
}


void set_as_hardnogo(struct spot* spot)
{
    spot->transposition_list = HARDNOGO;
    return;
}






