#ifndef ERRORH
#define ERRORH

typedef enum
{
    no_error = 0,
    usage,
    cant_open_setupfile,
    cant_close_setupfile,
    game_number_is_zero,
    game_number_not_found,
    empty_game,
    no_colon,
    second_man,
    illegal_character,
    not_enough_static_memory,
    boxes_unequal_targets,
    no_man,
    move_stack_is_full,
    cant_allocate_memory,

    /* Programming errors: */
    unexpected_existing_child_node,
    unexpected_direction_parent,
    no_parent_move,
//    not_enough_allocated_memory
} skbn_err;

skbn_err print_error(skbn_err error, ...);

#endif /* ERRORH */
