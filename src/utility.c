#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "sokoban2.h"
#include "error.h"
#include "resolve_int.h"
#include "utility.h"

/* Local defs */

#define HARDNOGO ((p_spot) -1)  /* Indicates that a spot is a hard no go: Never put a box on it. */

/* Conditional compile switches  */

//#define DBG_MARK 1        // Prints marker and marker wrappings
// #define DBG_MEM  1        // Prints memory allocations
#define DBG_HARDNOGOS 1   // Prints the search result for hard no-go's.
//#define DBG_C 1           // Prints
//#define DBG_D 1           // Prints

/* Conditional compile 'fabric' */


#ifdef DBG_MARK
#define printf_mark(...) printf(__VA_ARGS__);
#else
#define printf_mark(...)
#endif

#ifdef DBG_MEM
#define printf_mem(...) printf(__VA_ARGS__);
#else
#define printf_mem(...)
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

void reinit_mark(p_game_data_t p_game_data)
{
    p_spot spot;

    printf_mark("\nReinit marks...")

    for (spot = &(p_game_data->spot_pool[0]); spot < p_game_data->pool_ptr; spot++)
    {
        spot->reach_mark = 0;
        printf_mark(" %ld", SPOT_NO(spot))
    }
    printf_mark("\n")
}


/*int next_mark(p_game_data_t p_game_data)
 *
 * Used for explore functions.
 *
 * Increments the global spot mark to mark spot as newly found during a current eploration. This way,
 * the spot marks do not need to be re-initialized every new eploration. The reinitilization is still
 * required (and automatically performed) when the globla spot mark wraps. Reinitilization is also
 * required before searching for the amount of reaches a backward search can start from.
 */
int next_mark(p_game_data_t p_game_data)
{
    p_game_data->next_reach++;
    if (p_game_data->next_reach > 0x10000)
    {
        /* If next_reach wrapped, reiniatise all spots. */
        reinit_mark(p_game_data);

        /* And reinitialize next_reach as zero + 1. */
        p_game_data->next_reach = 1;
    }

    printf_mark("\nMark: %d",  p_game_data->next_reach)
    return p_game_data->next_reach;
}


skbn_err allocate_memory(p_game_data_t p_game_data)
{
     /* Sanity checks for the way we'er gonna use the tree memory. */
     if (sizeof(struct position_node) != TP_NODE_SIZE)
         return print_error(position_node_size, TP_NODE_SIZE);

     if (sizeof(struct position_leaf) != TP_LEAF_SIZE)
         return print_error(position_leaf_size, TP_LEAF_SIZE);

     if (sizeof(struct move_node) != MV_NODE_SIZE)
         return print_error(move_node_size, MV_NODE_SIZE);

    /* Allocate space for the trees. */
    p_game_data->p_memory_start = calloc(MEM_UNIT_COUNT, MEM_REF_UNIT);

    if ( ! p_game_data->p_memory_start)
        return print_error(cant_allocate_memory);

    /* Check whether the base of the memory is MEM_REF_UNIT aligned. */
    if ((long int) p_game_data->p_memory_start & MEM_REF_UNIT-1) /* (MEM_REF_UNIT is a power of 2.) */
    {
        free(p_game_data->p_memory_start);
        return print_error(memory_not_aligned, MEM_REF_UNIT);
    }
    p_game_data->p_memory_bottom = p_game_data->p_memory_start;
    p_game_data->p_memory_top  =   p_game_data->p_memory_start + MEM_UNIT_COUNT * MEM_REF_UNIT;

    return no_error;
}


uint32_t new_position_node(p_game_data_t p_game_data)
{
    uint32_t index;

    if (p_game_data->p_memory_bottom + TP_NODE_SIZE > p_game_data->p_memory_top)
        exit( print_error(out_of_tree_memory) );

    index = (uint32_t) ((p_game_data->p_memory_bottom - p_game_data->p_memory_start) / MEM_REF_UNIT);
        printf_mem("{N%d}", index)

    p_game_data->p_memory_bottom += TP_NODE_SIZE;
    p_game_data->tp_node_count++;
    return index;
}

uint32_t new_position_leaf(p_game_data_t p_game_data)
{
    uint32_t index;

    if (p_game_data->p_memory_bottom + TP_LEAF_SIZE > p_game_data->p_memory_top)
        exit( print_error(out_of_tree_memory) );

    index = (uint32_t) ((p_game_data->p_memory_bottom - p_game_data->p_memory_start) / MEM_REF_UNIT);
        printf_mem("{L%d}", index)

    p_game_data->p_memory_bottom += TP_LEAF_SIZE;
    p_game_data->tp_leaf_count++;
    return index;
}

uint32_t new_move_node(p_game_data_t p_game_data, t_s_dir search_dir)
{
    uint32_t index;

    if (p_game_data->p_memory_bottom + MV_NODE_SIZE > p_game_data->p_memory_top)
        exit( print_error(out_of_tree_memory) );

    index = (uint32_t) ((p_game_data->p_memory_bottom - p_game_data->p_memory_start) / MEM_REF_UNIT);
        printf_mem("{M%d}", index)

    p_game_data->p_memory_bottom += MV_NODE_SIZE;
    if (search_dir == forward)
        p_game_data->fw_move_count++;
    else
        p_game_data->bw_move_count++;
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

    /* First find all spots that are adjecent to at least two non-opposing walls, i.e. corners. */
    printf_hardnogo("\n\nCorners:\n")
    for (spot = &(p_game_data->spot_pool[0]); spot < p_game_data->pool_ptr; spot++)
    {
        if (    !spot->object[target]
            && (    (spot->neighbour[right] == NULL  &&  spot->neighbour[up   ] == NULL)
                ||  (spot->neighbour[up   ] == NULL  &&  spot->neighbour[left ] == NULL)
                ||  (spot->neighbour[left ] == NULL  &&  spot->neighbour[down ] == NULL)
                ||  (spot->neighbour[down ] == NULL  &&  spot->neighbour[right] == NULL) ) )
        {
            set_as_hardnogo(spot);

            printf_hardnogo(" %ld", SPOT_NO(spot))
        }
    }

    /* TODO: Document: find what here ?
     * Scan the setup multiple times: new found hard nogos can trigger the finding of more hard nogos.
     * Keep re-scanning until no new hard nogos are found anymore. (The only indication that another
     * round will not find more hard no gos is when the previous round didn't find any.) */
    printf_hardnogo("\nIn betweens:")
    while (new_hardnogos)
    {
        new_hardnogos = 0;
        printf_hardnogo("\n")

        /* From every spot that is marked as hard no go, do a horizontal and vertical line scan. */
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
    return spot->position_list == HARDNOGO;
}


/* TODO: Document. */
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
        if (scan->object[target] || (scan->neighbour[sub_dir_a] != NULL && scan->neighbour[sub_dir_b] != NULL))
            return;

    /* Apparently, there is (at least one) new hard no go on this line, so just mark them all. (Except when
     * the line length is zero (exluding start/end), in which case the for loop below will not enter and
     * thus correctly omitting the marking of new_hardnogos. */
    for (scan = spot->neighbour[main_dir]; scan && ! is_hardnogo(scan); scan = scan->neighbour[main_dir])
    {
        dbg_hardnogo_if( ! is_hardnogo(scan))
            printf_hardnogo(" %ld", SPOT_NO(scan))

        set_as_hardnogo(scan);
        *new_hardnogos  = 1;
    }
}


void set_as_hardnogo(struct spot* spot)
{
    spot->position_list = HARDNOGO;
    return;
}


char mv_dir_name(t_mv_dir move_direction)
{
    switch (move_direction)
    {
    case 0:  return 'R';
    case 1:  return 'U';
    case 2:  return 'L';
    case 3:  return 'D';
    default: return '?';
    }
}


void dbg_print_setup(p_game_data_t p_game_data)
{
    struct spot* curr_spot;
    struct spot* next_spot;
    struct spot* line_start;

    int line_pos = 0;         // For printing space where there is nothing.
    int c;

    int left_most;
    int right_most;

    /* Find the left -most position. */
    curr_spot  = &(p_game_data->spot_pool[0]);
    left_most  = curr_spot->position;
    right_most = curr_spot->position;
    while (curr_spot < p_game_data->pool_ptr)
    {
        if (left_most > curr_spot->position)
            left_most = curr_spot->position;
        if (right_most < curr_spot->position)
            right_most = curr_spot->position;
        curr_spot++;
    }

    /* Print it */
    curr_spot = &(p_game_data->spot_pool[0]);
    line_start = curr_spot;

    printf("\n    #");
    for (c=left_most; c<=right_most; c++)
        printf("#");

    printf("#\n    ");
    while (curr_spot < p_game_data->pool_ptr)
    {
        while (curr_spot->position > (line_pos))
        {
            printf("#");
            line_pos++;
        }

        if (curr_spot->object[box] && curr_spot->object[target])
            printf("H");
        else if (curr_spot->object[box])
            printf("X");
        else if (curr_spot->object[target] && curr_spot == p_game_data->johnny )
            printf("o");
        else if (curr_spot->object[target])
            printf(".");
        else if (curr_spot == p_game_data->johnny )
            printf("O");
        else
            printf(" ");

        line_pos++;

        next_spot = curr_spot + 1;
        if ( next_spot >= p_game_data->pool_ptr ||
            (next_spot < p_game_data->pool_ptr && curr_spot->position >= next_spot->position) )
        {
            while (line_pos++ <= right_most)
                printf("#");
            printf("#\n    ");

            line_pos = 0;
        }
        curr_spot = next_spot;
    }

    printf("#");
    for (c=left_most; c<=right_most; c++)
        printf("#");

    printf("#");
}


