#ifndef ERRORH
#define ERRORH

typedef enum
{
    no_error = 0,
    usage,
    cant_register_exit_function,
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
    memory_not_aligned,
    transposition_node_size,
    transposition_leaf_size,
    no_tree_memory,
    out_of_tree_memory,

    /* Programming errors: */
    unexpected_existing_child_node,
    unexpected_direction_parent,
    no_parent_move,
} skbn_err;

skbn_err print_error(skbn_err error, ...);

#endif /* ERRORH */
