#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "sokoban2.h"
#include "error.h"
#include "setup.h"

/* Debug defs */


#define PRINT_SMALL_SETUP 1
#define PRINT_MEDIUM_SETUP 1
//#define PRINT_LARGE_SETUP 1

/* Internal protos */

skbn_err parse_setup(p_game_data_t p_game_data, int chr, int target_game_number, int *done);
skbn_err add_new_spot(p_game_data_t p_game_data, int chr, int position, int line_number, int char_position);

void set_all_pointers(p_game_data_t p_game_data);

#ifdef PRINT_SMALL_SETUP
#define print_small_setup(...) printf(__VA_ARGS__);
#else
#define print_small_setup(...)
#endif

#ifdef PRINT_MEDIUM_SETUP
void print_medium_setup(p_game_data_t p_game_data);
#define print_medium_setup_def(x) print_medium_setup(x);
#else
#define print_medium_setup_def(...)
#endif

#ifdef PRINT_LARGE_SETUP
void print_large_setup(p_game_data_t p_game_data);
#define print_large_setup_def(x) print_large_setup(x);
#else
#define print_large_setup_def(...)
#endif

/* Exported data */

/* Imported data */

/* Local data */


int number_of_boxes   = 0;
int number_of_targets = 0;

/* Code */

skbn_err setup(p_game_data_t p_game_data, int argc, char *argv[])
{
    FILE *setupfile;

    int chr;
    int target_game_number;
    skbn_err error = no_error;
    int done = 0;

    if (argc < 3)
        return print_error(usage);

    /* Open the setup file. */
    setupfile = fopen(argv[1], "r");
    if (!setupfile)
        return print_error(cant_open_setupfile, argv[1]);

    target_game_number = atoi(argv[2]);
    if (target_game_number == 0)
        return print_error(game_number_is_zero);

    print_small_setup("\n");

    /* Parse the setup file. */
    do
    {
        chr = fgetc(setupfile);
        error = parse_setup(p_game_data, chr, target_game_number, &done);
    }
    while (error == no_error && !done);

    if (error)
        return error;

    if (number_of_boxes != number_of_targets)
        return print_error(boxes_unequal_targets);

    if ( ! p_game_data->johnny)
        return print_error(no_man);

    set_all_pointers(p_game_data);

    print_medium_setup_def(p_game_data)
    print_large_setup_def(p_game_data)

    /* Close the setup file. */
    if (fclose(setupfile))
        return print_error(cant_close_setupfile, argv[1]);

    return error;
}

skbn_err parse_setup(p_game_data_t p_game_data, int chr, int target_game_number, int *done)
{
    static enum {
        scanning_game_number,
        looking_for_colon,
        scanning_leading_spaces,
        scanning_other_spaces,
        scanning_data
    } state = scanning_game_number;

    static int current_game_number = 0;
    static int line_number = 1;
    static int space_counter = 0;
    static int character_position = 0;

    skbn_err error;

    character_position++;
    if ('\n' == chr)
        line_number++;

    while (1) switch (state)
    {
    case scanning_game_number:
        if (EOF == chr)
            return print_error(game_number_not_found);

        if (isdigit(chr))
        {
            current_game_number = 10 * current_game_number + (chr=='.' ? chr-'.' : chr-'0');
            return no_error;
        }
        if (target_game_number != current_game_number)
        {
            current_game_number = 0;
            return no_error;
        }

        state = looking_for_colon;
        break; // Break means re-enter state machine. The number-terminating char needs parsing too.

    case looking_for_colon:
        if (EOF == chr)
            return print_error(empty_game);

        if ('\n' == chr)
            return print_error(no_colon);

        if (':' != chr)
            return no_error;

        state = scanning_leading_spaces;
        space_counter = 0;
        character_position = 0;

        return no_error;

    case scanning_leading_spaces:
        if (' ' == chr)
        {
            print_small_setup("%c", chr);
            return no_error;
        }

        if (EOF == chr || isdigit(chr)) // A digit is a valid game-end, since it starts the successing game.
        {
            *done = 1;
            return no_error;
        }

        space_counter = 0;
        state = scanning_data;
        break; // Break means re-enter state machine. The first non-space on a line needs parsing too.

    case scanning_other_spaces:
        if (' ' == chr)
        {
            space_counter += 1;
            print_small_setup("%c", chr);
            return no_error;
        }

        if (EOF == chr || isdigit(chr)) // A digit is a valid game-end, since it starts the successing game.
        {
            *done = 1;
            return no_error;
        }

        if ('#' == chr || '.' == chr || 'X' == chr || 'H' == chr || 'O' == chr || 'o' == chr )
        {
            while (space_counter)
            {
                error = add_new_spot(p_game_data, ' ', character_position-space_counter,
                                     line_number, character_position);
                if (error)
                    return error;
                space_counter--;
            }
        }

        space_counter = 0;
        state = scanning_data;
        break; // Break means re-enter state machine. The first non-space on a line needs parsing too.

    case scanning_data:
        if (EOF == chr || isdigit(chr)) // A digit is a valid game-end, since it starts the successing game.
        {
            *done = 1;
            return no_error;
        }

        print_small_setup("%c", chr);

        switch (chr)
        {
        case ' ':
            space_counter = 1;

            state = scanning_other_spaces;
            return no_error;
        case '\n':
            character_position = 0;
            state = scanning_leading_spaces;
            return no_error;
        case '#':
            return no_error;
        case '.':
        case 'X':
        case 'H':
        case 'O':
        case 'o':
            error = add_new_spot(p_game_data, chr, character_position, line_number, character_position);
            if (error)
                return error;
            return no_error;
        default:
            return print_error(illegal_character, line_number, character_position);
            return no_error;
        }
    }
}


skbn_err add_new_spot(p_game_data_t p_game_data, int chr, int position, int line_number, int char_position)
{
    // Fetch a new spot from the pool.
    if (p_game_data->pool_ptr >= &(p_game_data->spot_pool[NR_OF_SPOTS]))
        return print_error(not_enough_static_memory);

    // Initialise it.
    p_game_data->pool_ptr->neighbour[up   ]   = NULL;
    p_game_data->pool_ptr->neighbour[down ]   = NULL;
    p_game_data->pool_ptr->neighbour[left ]   = NULL;
    p_game_data->pool_ptr->neighbour[right]   = NULL;
    p_game_data->pool_ptr->reach_chain        = NULL;
    p_game_data->pool_ptr->transposition_list = NULL;
    p_game_data->pool_ptr->has_box            = ('X' == chr) || ('H' == chr);
    p_game_data->pool_ptr->is_target          = ('.' == chr) || ('H' == chr) || ('o' == chr);
    p_game_data->pool_ptr->reach_mark         = 0;
    p_game_data->pool_ptr->position           = position;

    if (p_game_data->pool_ptr->has_box)
        number_of_boxes++;                 /* Counted for double checking the setup. */

    if (p_game_data->pool_ptr->is_target)
        number_of_targets++;               /* Counted for double checking the setup. */

    if ('O' == chr || 'o' == chr)
    {
        if (p_game_data->johnny)
            return print_error(second_man, line_number, char_position);
        else
            p_game_data->johnny = p_game_data->pool_ptr;
    }

    p_game_data->pool_ptr++;                 // Advance to the new spot on the pool.
    return no_error;
}


void set_all_pointers(p_game_data_t p_game_data)
{
    struct spot* curr_spot;
    struct spot* next_spot;
    struct spot* next_line;

    int line_no;
    int next_line_no;

    line_no = 0;
    next_line_no = 0;
    next_line = &(p_game_data->spot_pool[0]);
    for (curr_spot = &(p_game_data->spot_pool[0]); curr_spot < p_game_data->pool_ptr; curr_spot++)
    {
        /* Vertical chaining */
        while ( next_line < p_game_data->pool_ptr &&
                ( next_line_no <= line_no ||
                  ( line_no + 1 == next_line_no  && next_line->position < curr_spot->position)))
        {
            if (next_line+1 < p_game_data->pool_ptr && next_line->position >= (next_line+1)->position)
                next_line_no++; /* line wrap. */
            next_line++;
        }
        if (next_line < p_game_data->pool_ptr &&
            line_no + 1 == next_line_no &&
            curr_spot->position == next_line->position)
        {
            /* Vertically adjecent spots. Chain together; */
            curr_spot->neighbour[down] = next_line;
            next_line->neighbour[up  ] = curr_spot;
        }

        /* Horizontal chaining. */
        next_spot = curr_spot + 1;
        if (next_spot < p_game_data->pool_ptr)
        {
            if (curr_spot->position + 1 == next_spot->position)
            {
                /* Horizontally adjecent spots. Chain together; */
                curr_spot->neighbour[right] = next_spot;
                next_spot->neighbour[left ] = curr_spot;
            }
            if (curr_spot->position >= next_spot->position)
                line_no++; /* Line wrap. */
        }
    }
}



#ifdef PRINT_MEDIUM_SETUP
void print_medium_setup(p_game_data_t p_game_data)
{
    struct spot* curr_spot;
    struct spot* next_spot;
    struct spot* line_start;

    int line_pos = 0;         // For printing space where there is nothing.
    int line_rep = 1;

    int left_most;

    /* Find the left -most position. */
    curr_spot = &(p_game_data->spot_pool[0]);
    left_most = curr_spot->position;
    while (curr_spot < p_game_data->pool_ptr)
    {
        if (left_most > curr_spot->position)
            left_most = curr_spot->position;
        curr_spot++;
    }

    /* Shift spots left in order to left-align the whole 'store'. */
    left_most--; /* Leave a margin */
    curr_spot = &(p_game_data->spot_pool[0]);
    while (curr_spot < p_game_data->pool_ptr)
    {
        curr_spot->position -= left_most;
        curr_spot++;
    }

    /* Print it */
    curr_spot = &(p_game_data->spot_pool[0]);
    line_start = curr_spot;
    printf("\n");
    while (curr_spot < p_game_data->pool_ptr)
    {
        while (curr_spot->position > (line_pos))
        {
            printf("    "); /* Empty spaces. */
            line_pos++;
        }
        switch (line_rep)
        {
            case 1:
                printf("  %s", curr_spot->neighbour[up] ? "| " : "  ");
                break;
            case 2:
                printf("%s", curr_spot->neighbour[left] ? "-" : " ");
                printf("%03ld", SPOT_NUMBER(curr_spot));
                break;
            default /* Should not happen */:
                printf("/n/t????\n\n");
                break;
        }
        line_pos++;

        next_spot = curr_spot + 1;
        if ( next_spot >= p_game_data->pool_ptr ||
            (next_spot < p_game_data->pool_ptr && curr_spot->position >= next_spot->position) )
        {
            if (line_rep++ >= 2)
            {
                line_rep = 1;
                line_start = next_spot;
            }
            next_spot = line_start;

            line_pos = 0;
            printf("\n");
        }
        curr_spot = next_spot;
    }
}
#endif /* PRINT_MEDIUM_SETUP */


#ifdef PRINT_LARGE_SETUP
char *spot_number_str(p_game_data_t p_game_data, struct spot* spot, char *no_spot_str);

char *spot_number_str(p_game_data_t p_game_data, struct spot* spot, char *no_spot_str)
{
    static char spot_numberstring[3];

    if (spot)
        sprintf(spot_numberstring, "%02ld", SPOT_NUMBER(spot));
    else
        return no_spot_str;

    return spot_numberstring;
}


void print_large_setup(p_game_data_t p_game_data)
{
    struct spot* curr_spot;
    struct spot* next_spot;
    struct spot* line_start;

    int line_pos = 0;         // For printing space where there is nothing.
    int line_rep = 1;

    int left_most;

    /* Find the left -most position. */
    curr_spot = &(p_game_data->spot_pool[0]);
    left_most = curr_spot->position;
    while (curr_spot < p_game_data->pool_ptr)
    {
        if (left_most > curr_spot->position)
            left_most = curr_spot->position;
        curr_spot++;
    }

    /* Shift spots left in order to left-align the whole 'store'. */
    left_most--; /* Leave a margin */
    curr_spot = &(p_game_data->spot_pool[0]);
    while (curr_spot < p_game_data->pool_ptr)
    {
        curr_spot->position -= left_most;
        curr_spot++;
    }

    /* Print it */
    curr_spot = &(p_game_data->spot_pool[0]);
    line_start = curr_spot;
    printf("\n");
    while (curr_spot < p_game_data->pool_ptr)
    {
        while (curr_spot->position > (line_pos))
        {
            printf("         "); /* Empty spaces. */
            line_pos++;
        }
        switch (line_rep)
        {
            case 1:
                printf("    %s   ", spot_number_str(p_game_data, curr_spot->neighbour[up], "||"));
                break;
            case 2:
                printf(" %s", spot_number_str(p_game_data, curr_spot->neighbour[left], "  "));
                printf("=%02ld=", SPOT_NUMBER(curr_spot));
                printf("%s", spot_number_str(p_game_data, curr_spot->neighbour[right], "  "));
                break;
            case 3:
                printf("    %s   ", spot_number_str(p_game_data, curr_spot->neighbour[down], "||"));
                break;
            default /* Should not happen */:
                printf("/n/t????\n\n");
                break;
        }
        line_pos++;

        next_spot = curr_spot + 1;
        if ( next_spot >= p_game_data->pool_ptr ||
            (next_spot < p_game_data->pool_ptr && curr_spot->position >= next_spot->position) )
        {
            if (line_rep++ >= 3)
            {
                line_rep = 1;
                line_start = next_spot;
                printf("\n"); /* Add extra blank line per game line. */
            }
            next_spot = line_start;

            line_pos = 0;
            printf("\n");
        }
        curr_spot = next_spot;
    }
}
#endif /* PRINT_LARGE_SETUP */










