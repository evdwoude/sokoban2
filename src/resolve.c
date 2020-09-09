#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "sokoban2.h"
#include "error.h"
#include "resolve_int.h"
#include "forward.h"
#include "resolve.h"

/* Local defs */

// typedef enum {false = 0, true = 1} t_bool;

/* Internal protos */

/* Exported data */

/* Imported data */

/* Local data */

/* Code */

void  resolve(struct game_data *p_game_data)
{
    p_spot johnny = p_game_data->johnny;

    explore_reach(p_game_data, johnny);

    printf("\nChained (2):");

    /* test print: */
    while (johnny)
    {
        printf("  %d", johnny->spot_number );
        johnny = johnny->reach_chain;
    }

    return;
}
