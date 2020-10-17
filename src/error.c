#include <stdio.h>
#include <stdarg.h>
#include "error.h"

skbn_err print_error(skbn_err error, ...)
{
    va_list ap;

    int int_1;
    int int_2;
    int int_3;
    int int_4;

    char * char_1;

    va_start(ap, error);

    switch (error)
    {
    case usage:
        printf("\nUsage: soko <setupfile> <game number>.\n");
        break;
    case cant_register_exit_function:
        printf("\nCan't register exit function\n.");
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
    case memory_not_aligned:
        int_1 = va_arg(ap, int);
        printf("\nMemory not aligned on %d bytes boundary.\n", int_1);
        break;
    case move_node_size:
        int_1 = va_arg(ap, int);
        printf("\nSize of move_node is not %d.\n", int_1);
        break;
    case position_node_size:
        int_1 = va_arg(ap, int);
        printf("\nSize of position_node is not %d.\n", int_1);
        break;
    case position_leaf_size:
        int_1 = va_arg(ap, int);
        printf("\nSize of position_leaf is not %d.\n", int_1);
        break;
    case out_of_tree_memory:
        printf("\nOut of tree memory.\n");
        break;
    case move_path_not_polulated:
        printf("\nMove path not polulated.\n");
        break;
    case no_move_path_ref_ref_given:
        printf("\nNo reference to the move path reference given.\n");
        break;
    case no_child:
        printf("\nNo child reference in this move node.\n");
        break;
    case no_parent:
        printf("\nNo parent reference in this move node.\n");
        break;
    case no_sibbling:
        printf("\nNo sibbling reference in this move node.\n");
        break;
    case move_obj_err:
        int_1 = va_arg(ap, int);
        int_2 = va_arg(ap, int);
        int_3 = va_arg(ap, int);
        int_4 = va_arg(ap, int);
        printf("\nMake move error %d, %s. Spot no: %d. Move dir: %c.\n",
               int_1, int_2 ? "backward" : "forward", int_3, (char) int_4);
        break;
    default:
        printf("\nError in error handling (undefined error).\n");
        break;
    }

    va_end(ap);

    return error;
}
