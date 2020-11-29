#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "sokoban2.h"
#include "error.h"
#include "setup.h"
#include "resolve.h"
#include "utility.h"

/* Internal protos */

void exit_function(int status, void *arg);

/* Exported data */

/* Imported data */

/* Local data */

/* Code */

struct game_data game_data;

int main(int argc, char *argv[])
{
    skbn_err error;

    game_data.pool_ptr                = &(game_data.spot_pool[0]);
    game_data.johnny                  = NULL;
    game_data.position_head           = NULL;
    game_data.position_root           = 0;
    game_data.forward_move_root       = 0;
    game_data.backward_move_root_list = 0;
    game_data.next_reach              = 0;
    game_data.p_memory_start          = NULL;
    game_data.p_memory_bottom         = NULL;
    game_data.p_memory_top            = NULL;


    time_t start_time, end_time;
    long diff_time;

    p_game_data_t p_game_data = &game_data;

    start_time = time(NULL);

    printf("\nSokoban2 v0.13.");

    if (on_exit(&exit_function, (void *) &game_data))
        return print_error(cant_register_exit_function);

    error = setup(&game_data, argc, argv);
    if (error)
        return error;

    error = allocate_memory(&game_data);
    if (error)
        return error;


    resolve(&game_data);

    /* TOO: move the below into the exit function. */
    end_time = time(NULL);
    diff_time = (long) difftime(end_time, start_time);

//     printf("\nDuration: %ld:%ld:%ld.\n",
//         diff_time / 3600,
//         (diff_time / 60) % 60,
//         diff_time % 60 );

    return 0;
}

/* So for the only thing to worry about is the memort allocation, as for now setup() does not use exit()
 * and will take care of closing the setup file by its own.*/
void exit_function(int status, void *arg)
{
    if (arg && ((p_game_data_t)arg)->p_memory_start)
    {
//         printf("\nExit: freeing momory. %d, %ld", status, (long int) (((p_game_data_t)arg)->p_memory_start));
        free( ((p_game_data_t)arg)->p_memory_start );
    }
}




