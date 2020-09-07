#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "sokoban2.h"
#include "error.h"
#include "setup.h"

/* Debug defs */

#define PRINT_LARGE_SETUP 1

#ifndef PRINT_LARGE_SETUP
#define print_cond(...) printf(__VA_ARGS__);
#define print_setup(...)
#else
#define print_cond(...)
#endif


/* Internal protos */

skbn_err parse_setup(int chr, int target_game_number, int *done);
skbn_err add_new_spot(int chr, int position, int line_number, int character_position);

void set_all_pointers(void);
char *spot_number_str(struct spot* spot, char *no_spot_str);

#ifdef PRINT_LARGE_SETUP
void print_setup(void);
#endif

/* Exported data */

/* Imported data */

extern struct game_data game_data; // TODO: Make this a parameter:

/* Local data */

int number_of_targets = 0;

/* Code */

skbn_err setup(int argc, char *argv[])
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

    print_cond("\n");

    /* Parse the setup file. */
    do
    {
        chr = fgetc(setupfile);
        error = parse_setup(chr, target_game_number, &done);
    }
    while (error == no_error && !done);

    if (error)
        return error;

    if (game_data.number_of_boxes != number_of_targets)
        return print_error(boxes_unequal_targets);

    if (!game_data.man)
        return print_error(no_man);

    set_all_pointers();

    print_setup();

    /* Close the setup file. */
    if (fclose(setupfile))
        return print_error(cant_close_setupfile, argv[1]);

    return error;
}

skbn_err parse_setup(int chr, int target_game_number, int *done)
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
            print_cond("%c", chr);
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
            print_cond("%c", chr);
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
                error = add_new_spot(' ', character_position-space_counter, line_number, character_position);
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

        print_cond("%c", chr);

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
            error = add_new_spot(chr, character_position, line_number, character_position);
            if (error)
                return error;
            return no_error;
        default:
            return print_error(illegal_character, line_number, character_position);
            return no_error;
        }
    }
}


skbn_err add_new_spot(int chr, int position, int line_number, int character_position)
{
    static int spot_number = 1;

    // Fetch a new spot from the pool.
    if (game_data.pool_ptr >= &(game_data.spot_pool[NR_OF_SPOTS]))
        return print_error(not_enough_static_memory);

    // Initialise it.
    game_data.pool_ptr->neighbour[up   ] = NULL;
    game_data.pool_ptr->neighbour[down ] = NULL;
    game_data.pool_ptr->neighbour[left ] = NULL;
    game_data.pool_ptr->neighbour[right] = NULL;
    game_data.pool_ptr->has_box             = ('X' == chr) || ('H' == chr);
    game_data.pool_ptr->is_target           = ('.' == chr) || ('H' == chr) || ('o' == chr);
    game_data.pool_ptr->no_go               = 0;
    game_data.pool_ptr->has_box_bw          = game_data.pool_ptr->is_target;
    game_data.pool_ptr->is_target_bw        = game_data.pool_ptr->has_box;

    game_data.pool_ptr->position    = position;       /* For setup parsing only.    */
    game_data.pool_ptr->spot_number = spot_number++;  /* For debug printing only.   */

    if (game_data.pool_ptr->has_box)
        game_data.number_of_boxes++;      /* Counted for reaching the final goal (and for double checking the setup). */

    if (game_data.pool_ptr->is_target)
        number_of_targets++;            /* Counted for double checking the setup only. */

    if (game_data.pool_ptr->has_box && game_data.pool_ptr->is_target)
        game_data.number_of_boxes_on_target++;

    if ('O' == chr || 'o' == chr)
    {
        if (game_data.man)
            return print_error(second_man, line_number, character_position);
        else
            game_data.man = game_data.pool_ptr;
    }

    game_data.pool_ptr++;                 // Advance to the new spot on the pool.
    return no_error;
}


void set_all_pointers(void)
{
    struct spot* curr_spot;
    struct spot* next_spot;
    struct spot* next_line;

    int line_no;
    int next_line_no;

    line_no = 0;
    next_line_no = 0;
    next_line = &(game_data.spot_pool[0]);
    for (curr_spot = &(game_data.spot_pool[0]); curr_spot < game_data.pool_ptr; curr_spot++)
    {
        /* Vertical chaining */
        while ( next_line < game_data.pool_ptr &&
                ( next_line_no <= line_no ||
                  ( line_no + 1 == next_line_no  && next_line->position < curr_spot->position)))
        {
            if (next_line+1 < game_data.pool_ptr && next_line->position >= (next_line+1)->position)
                next_line_no++; /* line wrap. */
            next_line++;
        }
        if (next_line < game_data.pool_ptr &&
            line_no + 1 == next_line_no &&
            curr_spot->position == next_line->position)
        {
            /* Vertically adjecent spots. Chain together; */
            curr_spot->neighbour[down] = next_line;
            next_line->neighbour[up  ] = curr_spot;
        }

        /* Horizontal chaining. */
        next_spot = curr_spot + 1;
        if (next_spot < game_data.pool_ptr)
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


char *spot_number_str(struct spot* spot, char *no_spot_str)
{
    static char spot_numberstring[3];

    if (spot)
        sprintf(spot_numberstring, "%02d", spot->spot_number);
    else
        return no_spot_str;

    return spot_numberstring;
}

#ifdef PRINT_LARGE_SETUP
void print_setup(void)
{
    struct spot* curr_spot;
    struct spot* next_spot;
    struct spot* line_start;

    int line_pos = 0;         // For printing space where there is nothing.
    int line_rep = 1;

    int left_most;

    /* Find the left -most position. */
    curr_spot = &(game_data.spot_pool[0]);
    left_most = curr_spot->position;
    while (curr_spot < game_data.pool_ptr)
    {
        if (left_most > curr_spot->position)
            left_most = curr_spot->position;
        curr_spot++;
    }

    /* Shift spots left in order to left-align the whole 'store'. */
    left_most--; /* Leave a margin */
    curr_spot = &(game_data.spot_pool[0]);
    while (curr_spot < game_data.pool_ptr)
    {
        curr_spot->position -= left_most;
        curr_spot++;
    }

    /* Print it */
    curr_spot = &(game_data.spot_pool[0]);
    line_start = curr_spot;
    printf("\n\n");
    while (curr_spot < game_data.pool_ptr)
    {
        while (curr_spot->position > (line_pos))
        {
            printf("         "); /* Empty spaces. */
            line_pos++;
        }
        switch (line_rep)
        {
            case 1:
                printf("    %s   ", spot_number_str(curr_spot->neighbour[up], "||"));
                break;
            case 2:
                printf(" %s", spot_number_str(curr_spot->neighbour[left], "  "));
                printf("=%02d=", curr_spot->spot_number);
                printf("%s", spot_number_str(curr_spot->neighbour[right], "  "));
                break;
            case 3:
                printf("    %s   ", spot_number_str(curr_spot->neighbour[down], "||"));
                break;
            default /* Should not happen */:
                printf("/n/t????\n\n");
                break;
        }
        line_pos++;

        next_spot = curr_spot + 1;
        if ( next_spot >= game_data.pool_ptr ||
            (next_spot < game_data.pool_ptr && curr_spot->position >= next_spot->position) )
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
#else /* PRINT_LARGE_SETUP */
#if 0
void print_setup(void)
{
    struct spot* curr_spot;
    struct spot* next_spot;
    int line_pos = 0;         // For printing space where there is nothing.
    int left_most;
    int line_len = 0;         // line length
    int start_of_line = 1;
    int first_line = 1;

    /* Find the left -most position. */
    curr_spot = &(game_data.spot_pool[0]);
    left_most = curr_spot->position;
    while (curr_spot < game_data.pool_ptr)
    {
        if (left_most > curr_spot->position)
            left_most = curr_spot->position;
        curr_spot++;
    }

    /* Shift spots left in order to left-align the whole 'store'. */
    curr_spot = &(game_data.spot_pool[0]);
    while (curr_spot < game_data.pool_ptr)
    {
        curr_spot->position -= left_most;
        curr_spot++;
    }

    /* Print it */
    curr_spot = &(game_data.spot_pool[0]);
    printf("\n\n ");
    while (curr_spot < game_data.pool_ptr)
    {
        line_len++;
        while (curr_spot->position > (line_pos))
        {
            if (start_of_line)
                printf(" "); /* Empty spaces. */
            else
                printf("#"); /* Empty spaces. */
            line_pos++;
        }
        if (start_of_line)
            printf("#"); /* Empty spaces. */

        if (first_line)
            printf("#");
        else if (curr_spot == game_data.man)
            printf("O");
        else if (curr_spot->has_box)
            printf("X");
        else if (curr_spot->is_target)
            printf(".");
        else
            printf(" ");

        line_pos++;
        start_of_line = 0;
        line_len = line_pos;

        next_spot = curr_spot + 1;
        if ( next_spot >= game_data.pool_ptr ||
            (next_spot < game_data.pool_ptr && curr_spot->position >= next_spot->position) )
        {
            line_pos = 0;
            start_of_line = 1;
            if (first_line)
            {
                /* Repeat first line, but now pront content instead of hashes. */
                next_spot = &(game_data.spot_pool[0]);
                first_line = 0;
            }
            printf("#\n ");
        }
        curr_spot = next_spot;
    }

    /* Print last line */
    line_len++;
    while (line_len--)
        printf("#");
    printf("\n ");
}
#endif /* 0 */
#endif /* PRINT_LARGE_SETUP */















