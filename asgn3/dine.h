#ifndef DINE_H
#define DINE_H

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <stdbool.h>

#ifndef NUM_PHILOSOPHERS
#define NUM_PHILOSOPHERS (5)
#endif

#define IS_VALID_ARGC(x)    (x <= 2)
#define NUM_CYCLE_ARGS      (2)
#define STARTING_CHAR       ('A')
#define IS_EVEN(x)          ((x & 1) == 0)
#define GET_PHIL_IND(x)     (x - STARTING_CHAR)
#define GET_R_FORK_IND(x)   ((x + 1) % NUM_PHILOSOPHERS)
#define GET_L_FORK_IND(x)   (x)
#define FORK_TO_CHAR(x)     (x + 48)
#define PADDING             (8)
#define LEFT_HEADER_PAD     ((NUM_PHILOSOPHERS + 8) / 2)
#define RIGHT_HEADER_PAD    ((NUM_PHILOSOPHERS + 8) / 2 - 1)
#define MIN_PHILS           (2)


typedef enum PHIL_STATES_t {
    EATING,
    THINKING,
    CHANGING,
} PHIL_STATES_t;


typedef enum FORK_ACTION_t {
    PICKUP,
    PUTDOWN,
} FORK_ACTION_t;


typedef enum FORK_SIDE_t {
    LEFT,
    RIGHT,
} FORK_SIDE_t;


typedef struct Phil_t {
    pthread_t thread;
    char id;
    PHIL_STATES_t state;
    bool has_right_fork;
    bool has_left_fork;
} Phil_t;


void* dine(void* arg);
void update_phil_fork(size_t phil_ind, FORK_ACTION_t action, FORK_SIDE_t side);
void fork_action(size_t phil_ind, FORK_ACTION_t action, bool is_first_fork);
void pickup_forks(size_t phil_ind);
void putdown_forks(size_t phil_ind);

void dawdle();

#endif
