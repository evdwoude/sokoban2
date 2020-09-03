#include <stdio.h>
#include <time.h>
#include "setup.h"
#include "resolve.h"
#include "sokoban2.h"


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
    game_data.man = NULL;
    game_data.number_of_boxes = 0;
    game_data.number_of_boxes_on_target = 0;

    time_t start_time, end_time;
    long diff_time;

    start_time = time(NULL);

    printf("\nSokoban2 v0.");

    if (!setup(argc, argv))
        return error;

    resolve(&game_data);

    end_time = time(NULL);
    diff_time = (long) difftime(end_time, start_time);

    printf("\nDuration: %ld:%ld:%ld.\n",
        diff_time / 3600,
        (diff_time / 60) % 60,
        diff_time % 60 );

    return 0;
}