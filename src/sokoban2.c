#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "sokoban2.h"
#include "error.h"
#include "setup.h"
#include "resolve.h"
#include "utility.h"

/* Internal protos */

/* Exported data */

/* Imported data */

/* Local data */

/* Code */

struct game_data game_data;

int main(int argc, char *argv[])
{
    skbn_err error;

    game_data.pool_ptr                   = &(game_data.spot_pool[0]);
    game_data.johnny                     = NULL;
    game_data.transposition_head         = NULL;
    game_data.forward_transposition_root = 0;
    game_data.next_reach                 = 0;
    game_data.p_memory_start             = NULL;
    game_data.p_memory_bottom            = NULL;
    game_data.p_memory_top               = NULL;


    time_t start_time, end_time;
    long diff_time;

    // int test; // TODO remove.
    uint32_t test_node = 0;
    uint32_t test_leaf = 0;
    struct game_data *p_game_data = &game_data;

    start_time = time(NULL);

    printf("\nSokoban2 v0.12.\n");

    error = setup(&game_data, argc, argv);
    if (error)
        return error;

    error = allocate_memory(&game_data);
    if (error)
        return error;

//         /* Temp: */
//         printf("\nstart:     %lX.",   (long int) game_data.p_memory_start );
//         printf("\nbottom:    %lX.",   (long int) game_data.p_memory_bottom);
//         printf("\ntop:       %lX.\n", (long int) game_data.p_memory_top   );
//
//         printf("\n\ntest:      %lX\n", (long int) P_TPL(1) );
//
//         test_node = new_transposition_node(&game_data);
//         printf("\ntest node: %d.", test_node);
//         printf("\nptr:       %lX\n", (long int) P_TPN(test_node));
//
//         test_leaf = new_transposition_leaf(&game_data);
//         printf("\ntest leaf: %d.", test_leaf);
//         printf("\nptr:       %lX\n", (long int) P_TPL(test_leaf));
//
//         test_node = new_transposition_node(&game_data);
//         printf("\ntest node: %d.", test_node);
//         printf("\nptr:       %lX\n", (long int) P_TPN(test_node));
//
//         test_leaf = new_transposition_leaf(&game_data);
//         printf("\ntest leaf: %d.", test_leaf);
//         printf("\nptr:       %lX\n", (long int) P_TPL(test_leaf));
//

    resolve(&game_data);

    free(game_data.p_memory_start);

    end_time = time(NULL);
    diff_time = (long) difftime(end_time, start_time);

//     printf("\nDuration: %ld:%ld:%ld.\n",
//         diff_time / 3600,
//         (diff_time / 60) % 60,
//         diff_time % 60 );

    return 0;
}
