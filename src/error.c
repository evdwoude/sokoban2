#include <stdio.h>
#include <stdarg.h>
#include "error.h"

skbn_err print_error(skbn_err error, ...)
{
    va_list ap;

//      char *str_1;
//     char *str_2;
//     char *str_3;
    int int_1;
    int int_2;
//     int int_3;


    va_start(ap, error);

    /* Pre-parse source code file name and source code line number for the relevant error cases. */
    /*
    switch (error)
    {
        case A:
        case B:
            filename = va_arg(ap, int);
            linenr =   va_arg(ap, int);
        default:
            break;
    }
    */

    switch (error)
    {
    case usage:
        printf("\nUsage: soko <setupfile> <game number>.\n");
        break;
    case cant_open_setupfile:
        printf("\nCan't open setupfile: \"%s\".\n", va_arg(ap, char *));
        break;
    case cant_close_setupfile:
        printf("\nCan't close setupfile: \"%s\".\n", va_arg(ap, char *));
        break;
    case game_number_is_zero:
        printf("\nPlease specify a game number other then zero.\n");
        break;
    case game_number_not_found:
        printf("\nCouldn't find the given game number.\n");
        break;
    case empty_game:
        printf("\nThe given game has no data. (':' expected after game number.)\n");
        break;
    case no_colon:
        printf("\nThere's no colon before the end of the line.\n");
        break;
    case second_man:
        int_1 = va_arg(ap, int);
        int_2 = va_arg(ap, int);
        printf("\nThere is a second man in the game on line: %d, position: %d.\n", int_1, int_2);
        break;
    case illegal_character:
        int_1 = va_arg(ap, int);
        int_2 = va_arg(ap, int);
        printf("\nIllegal character in setup file, line: %d, position: %d.\n", int_1, int_2);
        break;
    case not_enough_static_memory:
        printf("\nNot enough static memory defined to read the game setup.\n");
        break;
//         case game_pool_max:
//             int_1 = va_arg(ap, int);
//             str_1 = va_arg(ap, char *);
//             int_2 = va_arg(ap, int);
//             printf("\nNumber of games exceeds the maximum of %d, file: \"%s\", line: %d\n", int_1, str_1, int_2);
//             break;
//         case game_buf_max:
//             str_1 = va_arg(ap, char *);
//             int_1 = va_arg(ap, int);
//             int_2 = va_arg(ap, int);
//             printf("\nRequired memory exceeds the reserved maximum, file: \"%s\", line: %d\n(src line nr: %d)\n", str_1, int_1, int_2);
//             break;
    case boxes_unequal_targets:
        printf("\nThe amount of boxes does not match the amount of targets.\n");
        break;
    case no_man:
        printf("\nThere's no warehouse manager on the floor.\n");
        break;
    case move_stack_is_full:
        printf("\nMove stack is full.\n");
        break;
    case cant_allocate_memory:
        printf("\nCannot allocate memory.\n");
        break;
//         case not_enough_allocated_memory:
//             printf("\nThere is not enough allocated memory.\n");
//             break;
    case memory_not_aligned:
        int_1 = va_arg(ap, int);
        printf("\nMemory not aligned on %d bytes boundary.\n", int_1);
        break;
    case transposition_node_size:
        int_1 = va_arg(ap, int);
        printf("\nSize of transposition_node is not %d.\n", int_1);
        break;
    case transposition_leaf_size:
        int_1 = va_arg(ap, int);
        printf("\nSize of transposition_leaf is not %d.\n", int_1);
        break;
    case no_tree_memory:
        printf("\nNo tree memory.\n");
        break;
    case out_of_tree_memory:
        printf("\nOut of tree memory.\n");
        break;
    case unexpected_existing_child_node:
        int_1 = va_arg(ap, int);
        printf("\nUnexpected pre-existing child node in move tree. Walking depth %d.\n", int_1);
        break;
    case unexpected_direction_parent:
        int_1 = va_arg(ap, int);
        printf("\nUnexpected direction \"parent\". Walking depth %d.\n", int_1);
        break;
    case no_parent_move:
        int_1 = va_arg(ap, int);
        printf("\nNo parent move pointer populated. Walking depth %d.\n", int_1);
        break;
    default:
        printf("\nError in error handling (undefined error).\n");
        break;
    }

    va_end(ap);

    return error;
}
