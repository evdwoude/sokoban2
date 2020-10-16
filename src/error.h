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
    move_node_size,
    position_node_size,
    position_leaf_size,
    out_of_tree_memory,
    move_path_not_polulated,
    no_move_path_ref_ref_given,

} skbn_err;

skbn_err print_error(skbn_err error, ...);

#endif /* ERRORH */
