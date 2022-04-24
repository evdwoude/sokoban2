#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "sokoban2.h"
#include "error.h"
#include "resolve_int.h"
#include "blockers.h"



#include "utility.h"                                            /* TODO: Remove! */
void print_medium_setup(p_game_data_t p_game_data);             /* TODO: Remove! */



/* Local defs */

#define HARDNOGO ((p_spot) -1)  /* Indicates that a spot is a "hard no go": Never put a box on it. */

#define LEFT(dir)  (((dir)+1) & down) /* Bitwise and makes it wrap from down to right. */
#define RIGHT(dir) (((dir)-1) & down) /* Bitwise and it wrap from right to down. */

/* Conditional compile switches  */

#define DBG_HARDNOGOS 1   // Prints the search result for hard no-go's.
// #define RIGHT_PRINTS
// #define RIGHT_PRINTS_WITH_SETUPS
// #define LEFT_PRINTS
// #define LEFT_PRINTS_WITH_SETUPS

/* Conditional compile 'fabric' */

#ifdef DBG_HARDNOGOS
#define printf_hardnogo(...) printf(__VA_ARGS__);
#define dbg_hardnogo_if(condition) if(condition)
#define dbg_hardnogo_for(condition) for(condition)
#else
#define printf_hardnogo(...)
#define dbg_hardnogo_if(condition)
#define dbg_hardnogo_for(condition)
#endif



/* Internal protos */

void scan_line_for_hardnogos(p_game_data_t p_game_data, struct spot* spot, int main_dir, int *new_hardnogos);
void set_as_hardnogo(struct spot* spot);

/* Right-side square cases. */
bool square_case_A(p_spot spot, t_mv_dir dir, p_game_data_t p_game_data);
bool square_case_B(p_spot spot, t_mv_dir dir, p_game_data_t p_game_data);
bool square_case_C(p_spot spot, t_mv_dir dir, p_game_data_t p_game_data);
bool square_case_D(p_spot spot, t_mv_dir dir, p_game_data_t p_game_data);
bool square_case_E(p_spot spot, t_mv_dir dir, p_game_data_t p_game_data);
bool square_case_F(p_spot spot, t_mv_dir dir, p_game_data_t p_game_data);

/* Left-side square cases. */
bool square_case_a(p_spot spot, t_mv_dir dir, p_game_data_t p_game_data);
bool square_case_b(p_spot spot, t_mv_dir dir, p_game_data_t p_game_data);
bool square_case_c(p_spot spot, t_mv_dir dir, p_game_data_t p_game_data);
bool square_case_d(p_spot spot, t_mv_dir dir, p_game_data_t p_game_data);
bool square_case_e(p_spot spot, t_mv_dir dir, p_game_data_t p_game_data);
bool square_case_f(p_spot spot, t_mv_dir dir, p_game_data_t p_game_data);


/* Exported data */

/* Imported data */

/* Local data */

/* Code */


/* void define_hardnogos(p_game_data_t p_game_data);
 *
 * Find and mark all spots where placing a box would always and immediately result in a dead end.
 * This includes the dead ends that result from individual boxes put on specific, spots, identifiable
 * from the static setup (i.e. the start position).
 * It excludes all dead ends that involve multiple boxes. These dead ends are more complex to find and
 * can't be idenfified up-front (staticallly), but only on-the-go, which is quite expensive.
 */
void define_hardnogos(p_game_data_t p_game_data)
{
    struct spot* spot;
    int new_hardnogos = 1;

    /* First find all spots that are adjecent to at least two non-opposing walls, i.e. corners. */
    printf_hardnogo("\n\nCorners:        ")
    for (spot = &(p_game_data->spot_pool[0]); spot < p_game_data->pool_ptr; spot++)
    {
        if (    !spot->object[target]
            && (    (spot->neighbour[right] == NULL  &&  spot->neighbour[up   ] == NULL)
                ||  (spot->neighbour[up   ] == NULL  &&  spot->neighbour[left ] == NULL)
                ||  (spot->neighbour[left ] == NULL  &&  spot->neighbour[down ] == NULL)
                ||  (spot->neighbour[down ] == NULL  &&  spot->neighbour[right] == NULL) ) )
        {
            set_as_hardnogo(spot);

            printf_hardnogo(" %lu", SPOT_NO(spot))
        }
    }

    /* TODO: Document: find what here ?
     * Scan the setup multiple times: new found hard nogos can trigger the finding of more hard nogos.
     * Keep re-scanning until no new hard nogos are found anymore. (The only indication that another
     * round will not find more hard no gos is when the previous round didn't find any.) */
    printf_hardnogo("\nIn betweens:    ")
    while (new_hardnogos)
    {
        new_hardnogos = 0;

        /* From every spot that is marked as hard no go, do a horizontal and vertical line scan. */
        for (spot = &(p_game_data->spot_pool[0]); spot < p_game_data->pool_ptr; spot++)
            if (is_hardnogo(spot))
            {
                scan_line_for_hardnogos(p_game_data, spot, right, &new_hardnogos); /* Horizontal line scan. */
                scan_line_for_hardnogos(p_game_data, spot,  down, &new_hardnogos); /* Vertical line scan.   */
            }
    }
    printf_hardnogo("\n")
}


/* TODO: Document. */
int is_hardnogo(struct spot* spot)
{
    return spot->position_list == HARDNOGO;
}


/* TODO: Document. */
void scan_line_for_hardnogos(p_game_data_t p_game_data, struct spot* spot, int main_dir, int *new_hardnogos)
{
    struct spot* scan;

    int sub_dir_a = left;
    int sub_dir_b = right;

    if (main_dir == right || main_dir == left)
    {
        sub_dir_a = up;
        sub_dir_b = down;
    }

    /* Scan line for any spot that can not (yet) be regarded as a hard no go */
    for (scan = spot->neighbour[main_dir]; scan && ! is_hardnogo(scan); scan = scan->neighbour[main_dir])
        if (scan->object[target] || (scan->neighbour[sub_dir_a] != NULL && scan->neighbour[sub_dir_b] != NULL))
            return;

    /* Apparently, there is (at least one) new hard no go on this line, so just mark them all. (Except when
     * the line length is zero (exluding start/end), in which case the for loop below will not enter and
     * thus correctly omitting the marking of new_hardnogos. */
    for (scan = spot->neighbour[main_dir]; scan && ! is_hardnogo(scan); scan = scan->neighbour[main_dir])
    {
        dbg_hardnogo_if( ! is_hardnogo(scan))
            printf_hardnogo(" %lu", SPOT_NO(scan))

        set_as_hardnogo(scan);
        *new_hardnogos  = 1;
    }
}


void set_as_hardnogo(struct spot* spot)
{
    spot->position_list = HARDNOGO;
    return;
}


/* TODO: Remove: */
#pragma GCC diagnostic ignored "-Wunused-parameter"


/* TODO: further document.
 *
 * bool is_fw_blocker(p_spot spot, t_mv_dir mv_dir, p_game_data_t p_game_data)
 *
 * Return true if the suggesed move will result in a blocking position, i.e. a position where there is:
 *   - no solution ahead,
 *   - no way to return back,
 * because al least one box is stuck on a location that is not a target.
 *
 *
 * Input condition:
 *   - Johnny points to the square behind the box he is about to push, and that square
 *       - exists, i.e. is not a wall,
 *       - is empty,
 *       - is not a hard nogo.
 *
 * is_fw_blocker() covers a limited set of cases that result in a blocked position.
 * For the below example, assume Johnny just pushed the indicated ('^') box upward.
 * The code is 'orientation agnostic' and works in all four rotations (not just the orientation of given example).
 *
 *      TODO: Swap C&D with E&F.
 *
 *      A:      B:      C:      D:      E:      F:      G:          Right side cases.
 *
 *      XX      XW      XX      XW      WX      WW      W-          QS  (Names used to refer to the locations)
 *      XX      XX      XW      XW      XX      XX      XW          PR  (     in de code comment below.      )
 *      ^       ^       ^       ^       ^       ^       ^           ^
 *
 *      a:      b:      c:      d:      e:      f:      g:          Left side cases (mirror the right side ones).
 *
 *      XX      WX      XX      WX      XW      WW      -W          UQ  (Names use to refer to the locations)
 *      XX      XX      WX      WX      XX      XX      WX          TP  (     in de code comment below.     )
 *       ^       ^       ^       ^       ^       ^       ^           ^
 *
 *      Legenda: 'X' is box, 'W' is wall, '-' is don't care.
 *
 * There is no need to check for the G-cases (and g-cases) because in these cases the left(and right)-bottom square
 * is either a hard no go or a target. The hard no go case is catched before and the target case is a false.
 *
 */

bool is_fw_blocker(p_spot spot, t_mv_dir dir, p_game_data_t p_game_data)
{
    /* Check for right-side square blocker ('left' means left relative to the move direction. */

    /* TODO: Add the left side cases. */

    if (spot->neighbour[dir]) {                                                             /* Is Q a spot?     */
        if (spot->neighbour[dir]->object[box]) {                                            /* Is Q a box?      */

            /* Check for A, B, C, D: */
            if (spot->neighbour[RIGHT(dir)]) {                                              /* Is R a spot?     */
                if (spot->neighbour[RIGHT(dir)]->object[box]) {                             /* Is R is a box?   */
                    if (spot->neighbour[RIGHT(dir)]->neighbour[dir]) {                      /* Is S a spot?     */
                        if (spot->neighbour[RIGHT(dir)]->neighbour[dir]->object[box]) {     /* Is S a box?      */
                            return square_case_A(spot, dir, p_game_data);                   /* Case A.          */
                        } /* else: S is empty. Check left side cases a, b, c, d. */
                    } else                                                                  /* S is a wall.     */
                        return square_case_B(spot, dir, p_game_data);                       /* Case B.          */
                } /* else: R is empty. Check left side cases a, b, c, d. */
            } else {                                                                        /* R is a wall.     */
                if (spot->neighbour[dir]->neighbour[RIGHT(dir)]) {                          /* Is S a spot?     */
                    if (spot->neighbour[dir]->neighbour[RIGHT(dir)]->object[box]) {         /* Is S a box?      */
                        return square_case_C(spot, dir, p_game_data);                       /* Case C.          */
                    } /* else: S is empty. Check left side cases a, b, c, d. */
                } else                                                                      /* S is a wall.     */
                    return square_case_D(spot, dir, p_game_data);                           /* Case D.          */
            }

            /* Check for a, b, c, d: */
            if (spot->neighbour[LEFT(dir)]) {                                               /* Is T a spot?     */
                if (spot->neighbour[LEFT(dir)]->object[box]) {                              /* Is T is a box?   */
                    if (spot->neighbour[LEFT(dir)]->neighbour[dir]) {                       /* Is U a spot?     */
                        if (spot->neighbour[LEFT(dir)]->neighbour[dir]->object[box]) {      /* Is U a box?      */
                            return square_case_a(spot, dir, p_game_data);                   /* Case a.          */
                        } /* else: U is empty. Return false below. */
                    } else                                                                  /* U is a wall.     */
                        return square_case_b(spot, dir, p_game_data);                       /* Case b.          */
                } /* else: T is empty. Return false below. */
            } else {                                                                        /* T is a wall.     */
                if (spot->neighbour[dir]->neighbour[LEFT(dir)]) {                           /* Is U a spot?     */
                    if (spot->neighbour[dir]->neighbour[LEFT(dir)]->object[box]) {          /* Is U a box?      */
                        return square_case_c(spot, dir, p_game_data);                       /* Case c.          */
                    } /* else: U is empty. Return false below. */
                } else                                                                      /* U is a wall.     */
                    return square_case_d(spot, dir, p_game_data);                           /* Case d.          */
            }

            return false; /* Some empty spot (R, S, T or U). */

        } else
            return false; /* Q is an empty spot. Return false */
    } else {                                                                                /* Q is a wall.     */

        /* Check for E, F: */
        if (spot->neighbour[RIGHT(dir)]) {                                                  /* Is R a spot?     */
            if (spot->neighbour[RIGHT(dir)]->object[box]) {                                 /* Is R is a box?   */
                if (spot->neighbour[RIGHT(dir)]->neighbour[dir]) {                          /* Is S a spot?     */
                    if (spot->neighbour[RIGHT(dir)]->neighbour[dir]->object[box]) {         /* Is S a box?      */
                        return square_case_E(spot, dir, p_game_data);                       /* Case E.          */
                    } /* else: S is empty. Check left side cases e, f. */
                } else                                                                      /* S is a wall.     */
                    return square_case_F(spot, dir, p_game_data);                           /* Case F.          */
            } /* else: R is empty. Check left side cases e, f. */
        } else
            return false; /* R is a wall (and Q is also). Case G: Return false always */

        /* Check for e, f: */
        if (spot->neighbour[LEFT(dir)]) {                                                   /* Is T a spot?     */
            if (spot->neighbour[LEFT(dir)]->object[box]) {                                  /* Is T is a box?   */
                if (spot->neighbour[LEFT(dir)]->neighbour[dir]) {                           /* Is U a spot?     */
                    if (spot->neighbour[LEFT(dir)]->neighbour[dir]->object[box]) {          /* Is U a box?      */
                        return square_case_e(spot, dir, p_game_data);                       /* Case e.          */
                    } /* else: U is empty. Return false below. */
                } else                                                                      /* U is a wall.     */
                    return square_case_f(spot, dir, p_game_data);                           /* Case f.          */
            } /* else T is empty. Return false below. */
        } else
            return false; /* T is a wall (and Q is also). Case g: Return false "always" */

        return false; /* Some empty spot (R, S, T or U). */
    }
    /* Not reachable; compiler checks this. */
}

/* Right-side cases. */

bool square_case_A(p_spot spot, t_mv_dir dir, p_game_data_t p_game_data)
{
#ifdef RIGHT_PRINTS
    static bool case_true__todo = true;
    static bool case_false_todo = true;
#endif

    bool blocker =
        !spot->object[target] ||                                                            /* P */
        !spot->neighbour[dir]->object[target] ||                                            /* Q */
        !spot->neighbour[RIGHT(dir)]->object[target] ||                                     /* R */
        !spot->neighbour[RIGHT(dir)]->neighbour[dir]->object[target];                       /* S */

#ifdef RIGHT_PRINTS
    if (blocker ? case_true__todo : case_false_todo)
    {
        if (blocker)
            case_true__todo = false;
        else
            case_false_todo = false;

        printf("\nCase A: %s.   %ld-%d.\n", blocker?"True":"False", SPOT_NO(spot), dir);
#ifdef RIGHT_PRINTS_WITH_SETUPS
        dbg_print_setup(p_game_data);
        print_medium_setup(p_game_data);
#endif
    }
#endif

    if (blocker)
        p_game_data->no_true_blockers++;
    else
        p_game_data->no_false_blockers++;

    return blocker;
}

bool square_case_B(p_spot spot, t_mv_dir dir, p_game_data_t p_game_data)
{
#ifdef RIGHT_PRINTS
    static bool case_true__todo = true;
    static bool case_false_todo = true;
#endif

    bool blocker =
        !spot->object[target] ||                                                            /* P */
        !spot->neighbour[dir]->object[target] ||                                            /* Q */
        !spot->neighbour[RIGHT(dir)]->object[target];                                       /* R */

#ifdef RIGHT_PRINTS
    if (blocker ? case_true__todo : case_false_todo)
    {
        if (blocker)
            case_true__todo = false;
        else
            case_false_todo = false;

        printf("\nCase B: %s.   %ld-%d.\n", blocker?"True":"False", SPOT_NO(spot), dir);
#ifdef RIGHT_PRINTS_WITH_SETUPS
        dbg_print_setup(p_game_data);
        print_medium_setup(p_game_data);
#endif
    }
#endif

    if (blocker)
        p_game_data->no_true_blockers++;
    else
        p_game_data->no_false_blockers++;

    return blocker;
}

bool square_case_C(p_spot spot, t_mv_dir dir, p_game_data_t p_game_data)
{
#ifdef RIGHT_PRINTS
    static bool case_true__todo = true;
    static bool case_false_todo = true;
#endif

    bool blocker =
        !spot->object[target] ||                                                            /* P */
        !spot->neighbour[dir]->object[target] ||                                            /* Q */
        !spot->neighbour[dir]->neighbour[RIGHT(dir)]->object[target];                       /* S */

#ifdef RIGHT_PRINTS
    if (blocker ? case_true__todo : case_false_todo)
    {
        if (blocker)
            case_true__todo = false;
        else
            case_false_todo = false;

        printf("\nCase C: %s.   %ld-%d.\n", blocker?"True":"False", SPOT_NO(spot), dir);
#ifdef RIGHT_PRINTS_WITH_SETUPS
        dbg_print_setup(p_game_data);
        print_medium_setup(p_game_data);
#endif
    }
#endif

    if (blocker)
        p_game_data->no_true_blockers++;
    else
        p_game_data->no_false_blockers++;

    return blocker;
}

bool square_case_D(p_spot spot, t_mv_dir dir, p_game_data_t p_game_data)
{
#ifdef RIGHT_PRINTS
    static bool case_true__todo = true;
    static bool case_false_todo = true;
#endif

    bool blocker =
        !spot->object[target] ||                                                            /* P */
        !spot->neighbour[dir]->object[target];                                              /* Q */

#ifdef RIGHT_PRINTS
    if (blocker ? case_true__todo : case_false_todo)
    {
        if (blocker)
            case_true__todo = false;
        else
            case_false_todo = false;

        printf("\nCase D: %s.   %ld-%d.\n", blocker?"True":"False", SPOT_NO(spot), dir);
#ifdef RIGHT_PRINTS_WITH_SETUPS
        dbg_print_setup(p_game_data);
        print_medium_setup(p_game_data);
#endif
    }
#endif

    if (blocker)
        p_game_data->no_true_blockers++;
    else
        p_game_data->no_false_blockers++;

    return blocker;
}

bool square_case_E(p_spot spot, t_mv_dir dir, p_game_data_t p_game_data)
{
#ifdef RIGHT_PRINTS
    static bool case_true__todo = true;
    static bool case_false_todo = true;
#endif

    bool blocker =
        !spot->object[target] ||                                                            /* P */
        !spot->neighbour[RIGHT(dir)]->object[target] ||                                     /* R */
        !spot->neighbour[RIGHT(dir)]->neighbour[dir]->object[target];                       /* S */

#ifdef RIGHT_PRINTS
    if (blocker ? case_true__todo : case_false_todo)
    {
        if (blocker)
            case_true__todo = false;
        else
            case_false_todo = false;

        printf("\nCase E: %s.   %ld-%d.\n", blocker?"True":"False", SPOT_NO(spot), dir);
#ifdef RIGHT_PRINTS_WITH_SETUPS
        dbg_print_setup(p_game_data);
        print_medium_setup(p_game_data);
#endif
    }
#endif

    if (blocker)
        p_game_data->no_true_blockers++;
    else
        p_game_data->no_false_blockers++;

    return blocker;
}

bool square_case_F(p_spot spot, t_mv_dir dir, p_game_data_t p_game_data)
{
#ifdef RIGHT_PRINTS
    static bool case_true__todo = true;
    static bool case_false_todo = true;
#endif

    bool blocker =
        !spot->object[target] ||                                                            /* P */
        !spot->neighbour[RIGHT(dir)]->object[target];                                       /* R */

#ifdef RIGHT_PRINTS
    if (blocker ? case_true__todo : case_false_todo)
    {
        if (blocker)
            case_true__todo = false;
        else
            case_false_todo = false;

        printf("\nCase F: %s.   %ld-%d.\n", blocker?"True":"False", SPOT_NO(spot), dir);
#ifdef RIGHT_PRINTS_WITH_SETUPS
        dbg_print_setup(p_game_data);
        print_medium_setup(p_game_data);
#endif
    }
#endif

    if (blocker)
        p_game_data->no_true_blockers++;
    else
        p_game_data->no_false_blockers++;

    return blocker;
}

/* Left-side cases. */

bool square_case_a(p_spot spot, t_mv_dir dir, p_game_data_t p_game_data)
{
#ifdef LEFT_PRINTS
    static bool case_true__todo = true;
    static bool case_false_todo = true;
#endif

    bool blocker =
        !spot->object[target] ||                                                            /* P */
        !spot->neighbour[dir]->object[target] ||                                            /* Q */
        !spot->neighbour[LEFT(dir)]->object[target] ||                                      /* T */
        !spot->neighbour[LEFT(dir)]->neighbour[dir]->object[target];                        /* U */

#ifdef LEFT_PRINTS
    if (blocker ? case_true__todo : case_false_todo)
    {
        if (blocker)
            case_true__todo = false;
        else
            case_false_todo = false;

        printf("\nCase a: %s.   %ld-%d.\n", blocker?"True":"False", SPOT_NO(spot), dir);
#ifdef LEFT_PRINTS_WITH_SETUPS
        dbg_print_setup(p_game_data);
        print_medium_setup(p_game_data);
#endif
    }
#endif

    if (blocker)
        p_game_data->no_true_blockers++;
    else
        p_game_data->no_false_blockers++;

    return blocker;
}

bool square_case_b(p_spot spot, t_mv_dir dir, p_game_data_t p_game_data)
{
#ifdef LEFT_PRINTS
    static bool case_true__todo = true;
    static bool case_false_todo = true;
#endif

    bool blocker =
        !spot->object[target] ||                                                            /* P */
        !spot->neighbour[dir]->object[target] ||                                            /* Q */
        !spot->neighbour[LEFT(dir)]->object[target];                                        /* T */

#ifdef LEFT_PRINTS
    if (blocker ? case_true__todo : case_false_todo)
    {
        if (blocker)
            case_true__todo = false;
        else
            case_false_todo = false;

        printf("\nCase b: %s.   %ld-%d.\n", blocker?"True":"False", SPOT_NO(spot), dir);
#ifdef LEFT_PRINTS_WITH_SETUPS
        dbg_print_setup(p_game_data);
        print_medium_setup(p_game_data);
#endif
    }
#endif

    if (blocker)
        p_game_data->no_true_blockers++;
    else
        p_game_data->no_false_blockers++;

    return blocker;
}

bool square_case_c(p_spot spot, t_mv_dir dir, p_game_data_t p_game_data)
{
#ifdef LEFT_PRINTS
    static bool case_true__todo = true;
    static bool case_false_todo = true;
#endif

    bool blocker =
        !spot->object[target] ||                                                            /* P */
        !spot->neighbour[dir]->object[target] ||                                            /* Q */
        !spot->neighbour[dir]->neighbour[LEFT(dir)]->object[target];                        /* U */

#ifdef LEFT_PRINTS
    if (blocker ? case_true__todo : case_false_todo)
    {
        if (blocker)
            case_true__todo = false;
        else
            case_false_todo = false;

        printf("\nCase c: %s.   %ld-%d.\n", blocker?"True":"False", SPOT_NO(spot), dir);
#ifdef LEFT_PRINTS_WITH_SETUPS
        dbg_print_setup(p_game_data);
        print_medium_setup(p_game_data);
#endif
    }
#endif

    if (blocker)
        p_game_data->no_true_blockers++;
    else
        p_game_data->no_false_blockers++;

    return blocker;
}

bool square_case_d(p_spot spot, t_mv_dir dir, p_game_data_t p_game_data)
{
#ifdef LEFT_PRINTS
    static bool case_true__todo = true;
    static bool case_false_todo = true;
#endif

    bool blocker =
        !spot->object[target] ||                                                            /* P */
        !spot->neighbour[dir]->object[target];                                              /* Q */

#ifdef LEFT_PRINTS
    if (blocker ? case_true__todo : case_false_todo)
    {
        if (blocker)
            case_true__todo = false;
        else
            case_false_todo = false;

        printf("\nCase d: %s.   %ld-%d.\n", blocker?"True":"False", SPOT_NO(spot), dir);
#ifdef LEFT_PRINTS_WITH_SETUPS
        dbg_print_setup(p_game_data);
        print_medium_setup(p_game_data);
#endif
    }
#endif

    if (blocker)
        p_game_data->no_true_blockers++;
    else
        p_game_data->no_false_blockers++;

    return blocker;
}

bool square_case_e(p_spot spot, t_mv_dir dir, p_game_data_t p_game_data)
{
#ifdef LEFT_PRINTS
    static bool case_true__todo = true;
    static bool case_false_todo = true;
#endif

    bool blocker =
        !spot->object[target] ||                                                            /* P */
        !spot->neighbour[LEFT(dir)]->object[target] ||                                      /* T */
        !spot->neighbour[LEFT(dir)]->neighbour[dir]->object[target];                        /* U */

#ifdef LEFT_PRINTS
    if (blocker ? case_true__todo : case_false_todo)
    {
        if (blocker)
            case_true__todo = false;
        else
            case_false_todo = false;

        printf("\nCase e: %s.   %ld-%d.\n", blocker?"True":"False", SPOT_NO(spot), dir);
#ifdef LEFT_PRINTS_WITH_SETUPS
        dbg_print_setup(p_game_data);
        print_medium_setup(p_game_data);
#endif
    }
#endif

    if (blocker)
        p_game_data->no_true_blockers++;
    else
        p_game_data->no_false_blockers++;

    return blocker;
}

bool square_case_f(p_spot spot, t_mv_dir dir, p_game_data_t p_game_data)
{
#ifdef LEFT_PRINTS
    static bool case_true__todo = true;
    static bool case_false_todo = true;
#endif

    bool blocker =
        !spot->object[target] ||                                                            /* P */
        !spot->neighbour[LEFT(dir)]->object[target];                                        /* T */

#ifdef LEFT_PRINTS
    if (blocker ? case_true__todo : case_false_todo)
    {
        if (blocker)
            case_true__todo = false;
        else
            case_false_todo = false;

        printf("\nCase f: %s.   %ld-%d.\n", blocker?"True":"False", SPOT_NO(spot), dir);
#ifdef LEFT_PRINTS_WITH_SETUPS
        dbg_print_setup(p_game_data);
        print_medium_setup(p_game_data);
#endif
    }
#endif

    if (blocker)
        p_game_data->no_true_blockers++;
    else
        p_game_data->no_false_blockers++;

    return blocker;
}
