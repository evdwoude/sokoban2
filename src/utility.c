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

// #define DBG_MARK 1        // Prints marker and marker wrappings
// #define DBG_MEM  1        // Prints memory allocations
// #define DBG_MOVE  1        // Prints memory allocations for moves only.
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

#ifdef DBG_MOVE
#define printf_move(...) printf(__VA_ARGS__);
#else
#define printf_move(...)
#endif


/* Internal protos */

void printlong(unsigned long this, unsigned long minlen);

/* Exported data */

/* Imported data */

/* Local data */

/* Code */

void reinit_mark(p_game_data_t p_game_data, uint32_t initialiser)
{
    p_spot spot;

    printf_mark("\nReinit marks...")

    for (spot = &(p_game_data->spot_pool[0]); spot < p_game_data->pool_ptr; spot++)
    {
        spot->reach_mark = initialiser;
        printf_mark(" %ld", SPOT_NO(spot))
    }

    /* And reinitialize next_reach as zero + 1. */
    p_game_data->next_reach = 1;

    printf_mark("\n")
}


/* uint32_t next_mark(p_game_data_t p_game_data)
 *
 * Used for explore functions.
 *
 * Increments the global spot mark to mark spot as newly found during a current eploration. This way,
 * the spot marks do not need to be re-initialized every new eploration. The reinitilization is still
 * required (and automatically performed) when the globla spot mark wraps. Reinitilization is also
 * required before searching for the amount of reaches a backward search can start from.
 */
uint32_t next_mark(p_game_data_t p_game_data)
{
    p_game_data->next_reach++;

    /* If next_reach wrapped, reiniatise all spots. */
    if (p_game_data->next_reach > 0x10000000)
        reinit_mark(p_game_data, 0);

    printf_mark("\nMark: %u",  p_game_data->next_reach)
    return p_game_data->next_reach;
}


skbn_err allocate_memory(p_game_data_t p_game_data)
{
//     int n=1;

     /* Sanity checks for the way we'er gonna use the tree memory. */
     if (sizeof(struct position_node) != TP_NODE_SIZE)
         return print_error(position_node_size, TP_NODE_SIZE);

     if (sizeof(struct position_leaf) != TP_LEAF_SIZE)
         return print_error(position_leaf_size, TP_LEAF_SIZE);

     if (sizeof(struct move_node) != MV_NODE_SIZE)
         return print_error(move_node_size, MV_NODE_SIZE);

    /* Allocate space for the trees. */
//     while (1)
//     {
        p_game_data->p_memory_start = calloc(MEM_UNIT_COUNT, MEM_REF_UNIT);
//         printlong( (long unsigned int)(p_game_data->p_memory_start), 24);
//         printlong( (long unsigned int)(MEM_UNIT_COUNT) * (long unsigned int)(MEM_REF_UNIT), 24);
//         printf("    %d\n", n++);

        if ( ! p_game_data->p_memory_start)
            return print_error(cant_allocate_memory);
//     }


    /* Check whether the base of the memory is MEM_REF_UNIT aligned. */
    if ((long int) p_game_data->p_memory_start & (MEM_REF_UNIT-1)) /* (MEM_REF_UNIT is a power of 2.) */
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
    printf_mem("{N%u}", index)

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
    printf_mem("{L%u}", index)

    p_game_data->p_memory_bottom += TP_LEAF_SIZE;
    p_game_data->tp_leaf_count++;
    return index;
}

uint32_t new_move_node(p_game_data_t p_game_data, t_s_dir search_dir)
{
    uint32_t index;

    /* Check if we have previously used moves in the recycle pool. */
    if (p_game_data->move_pool)
    {
        /* Use a node from the recycle pool. */
        index = p_game_data->move_pool;
        p_game_data->move_pool = P_MN(index)->child;

        memset((void *)(P_MN(index)), 0,  MV_NODE_SIZE);

        p_game_data->reuses += 1;
    }
    else
    {
        /* Use a new node from the allocated pool. */
        if (p_game_data->p_memory_bottom + MV_NODE_SIZE > p_game_data->p_memory_top)
            exit( print_error(out_of_tree_memory) );

        index = (uint32_t) ((p_game_data->p_memory_bottom - p_game_data->p_memory_start) / MEM_REF_UNIT);
        printf_mem("{M%u}", index)
        printf_move(" {%u}", index)

        p_game_data->p_memory_bottom += MV_NODE_SIZE;
    }

    if (search_dir == forward)
        p_game_data->fw_move_count++;
    else
        p_game_data->bw_move_count++;

    return index;
}


void return_move_node(p_game_data_t p_game_data, uint32_t index)
{
    /* Add the used move to the pool of move nodes (begining of list). */

    P_MN(index)->child = p_game_data->move_pool; /* If move_pool was 0, child will indicate end of list. */
    p_game_data->move_pool = index;

    p_game_data->cleanups += 1; /* statistics. The amount of move nodes returned to the pool. */
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

void print_interim_header(void)
{
    printf("\n");
    printf("  Fw:  Bw:      Fw width:      Bw width:      FW nodes:      BW nodes:      Cleanups:        Reused:           Memory:\n");
}

void print_interim_stats(p_game_data_t p_game_data, int move_count_fw, int move_count_bw, int fw_width, int bw_width)
{
    unsigned long bottom_section, top_section;

    printf(" %4d %4d", move_count_fw, move_count_bw);

    printlong(fw_width, 16);
    printlong(bw_width, 16);

    printlong(p_game_data->fw_move_count, 16);
    printlong(p_game_data->bw_move_count, 16);

    printlong(p_game_data->cleanups, 16);
    printlong(p_game_data->reuses, 16);

    bottom_section  = p_game_data->p_memory_bottom - p_game_data->p_memory_start;
    top_section     = p_game_data->p_memory_start + MEM_UNIT_COUNT * MEM_REF_UNIT - p_game_data->p_memory_top;

    printlong(bottom_section + top_section, 19);
    printf("\n");
}

void print_header(void)
{
    printf("  Total:       Moves:  Steps:                 FW nodes:      BW nodes:           Memory:\n");
}

void print_stats(p_game_data_t p_game_data, int move_count, int step_count)
{
    unsigned long bottom_section, top_section;

    printf("    %4d         %4d    %4d           ", move_count + step_count, move_count, step_count );

    printlong(p_game_data->fw_move_count, 16);
    printlong(p_game_data->bw_move_count, 16);

    bottom_section  = p_game_data->p_memory_bottom - p_game_data->p_memory_start;
    top_section     = p_game_data->p_memory_start + MEM_UNIT_COUNT * MEM_REF_UNIT - p_game_data->p_memory_top;

    printlong(bottom_section + top_section, 19);
    printf("\n");

    printf("\nBlockers (T/F): %d / %d.\n", p_game_data->no_true_blockers, p_game_data->no_false_blockers);
}

#define MAXLONGLEN 30

void printlong(unsigned long this, unsigned long minlen)
{
    char chars[MAXLONGLEN];
    char *pchar = &chars[MAXLONGLEN];
    int comma = 0;

    *--pchar = 0;
    if (!this)
        *--pchar = '0';

    while (this)
    {
        *--pchar = this % 10 + '0';
        this /= 10;
        comma = (comma + 1) % 3;
        if (!comma)
            *--pchar = ',';
    }
    if (*pchar == ',')
        pchar++;

    if (minlen < MAXLONGLEN)
        while ( pchar > &chars[MAXLONGLEN-minlen] )
            *--pchar = ' ';

    printf("%s", pchar);
}
