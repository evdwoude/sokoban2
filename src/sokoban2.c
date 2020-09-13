#include <stdio.h>
#include <time.h>
#include "sokoban2.h"
#include "error.h"
#include "setup.h"
#include "resolve.h"

/* Internal protos */

/* Exported data */

/* Imported data */

/* Local data */

/* Code */

struct game_data game_data;


int main(int argc, char *argv[])
{
    skbn_err error;
    game_data.pool_ptr = &(game_data.spot_pool[0]);
    game_data.johnny = NULL;
    game_data.next_reach = 0;

    time_t start_time, end_time;
    long diff_time;

    start_time = time(NULL);

    printf("\nSokoban2 v0.8.\n");

    if (setup(argc, argv) != no_error)
        return error;

    resolve(&game_data);

    end_time = time(NULL);
    diff_time = (long) difftime(end_time, start_time);

//     printf("\nDuration: %ld:%ld:%ld.\n",
//         diff_time / 3600,
//         (diff_time / 60) % 60,
//         diff_time % 60 );

    return 0;
}
