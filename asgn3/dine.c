#include "dine.h"

sem_t mutex_print;
sem_t forks[NUM_PHILOSOPHERS];
Phil_t phils[NUM_PHILOSOPHERS];
size_t num_cycles = 1; // default 

void print_header();
void print_borderline();
void print_tings();


int main(int argc, char** argv) {
    int i; // WHY MUST I DO THIS, I JUST WANT C99 BACK
    
    // read cmd line args
    if (!IS_VALID_ARGC(argc)) {
        perror("Incorrect Usage");
        exit(EXIT_FAILURE);
    }

    if (argc == NUM_CYCLE_ARGS) {
        char *endptr;
        long tmp = strtol(argv[1], &endptr, 10);

        if (*endptr != '\0') {
            perror("Incorrect usage. Enter a valid integer");
            exit(EXIT_FAILURE);
        }

        if (tmp <= 0 || tmp > INT_MAX) {
            perror("Incorrect usage. Enter a valid integer");            
            exit(EXIT_FAILURE);
        }

        num_cycles = (int)tmp; 
    }

    if (NUM_PHILOSOPHERS < MIN_PHILS) {
        perror("Not enough philosophers to discuss");
        exit(EXIT_FAILURE);
    }

    // init semaphores
    for (i = 0; i < NUM_PHILOSOPHERS; i++) {
        if (sem_init(&forks[i], 0, 1) == -1) {
            perror("Failed to init semaphores");
            exit(EXIT_FAILURE);
        }
    }

    if (sem_init(&mutex_print, 0, 1) == -1) {
        perror("Failed to init semaphores");
        exit(EXIT_FAILURE);
    }

    // assign the philosophers
    for (i = 0; i < NUM_PHILOSOPHERS; i++) {
        phils[i].id = 'A' + i;
        phils[i].state = CHANGING;
        phils[i].has_right_fork = false;
        phils[i].has_left_fork = false;
        // ? do i need to default assign threads?
    }

    print_header();

    // create pthreads
    for (i = 0; i < NUM_PHILOSOPHERS; i++) {
        if (pthread_create(&phils[i].thread, NULL, 
                dine, (void*)(&phils[i])) == -1) {
            perror("Couldnt create thread");
            exit(EXIT_FAILURE);
        }
    }

    // wait for the philosophers 
    for (i = 0; i < NUM_PHILOSOPHERS; i++){
        if (pthread_join(phils[i].thread, NULL) != 0){
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
    }

    print_borderline();

    // clean up on aisle here
    // destroy semaphores
    for (i = 0; i < NUM_PHILOSOPHERS; i++){
        if (sem_destroy(&forks[i]) == -1){
            perror("sem_destroy\n");
        }
    }
    
    // destroy mutex
    if (sem_destroy(&mutex_print) == -1){
        perror("sem_destroy\n");
    }

    return EXIT_SUCCESS;
}


void* dine(void* arg) {
    int i;
    Phil_t* phil = (Phil_t*) arg;

    print_tings();
    size_t phil_ind = GET_PHIL_IND(phil->id);
    for (i = 0; i < num_cycles; i++) {
        // Eating
        phils[phil_ind].state = CHANGING;
        print_tings();

        pickup_forks(phil_ind);
        print_tings();

        dawdle();
        print_tings();
    
        // Pondering
        phils[phil_ind].state = CHANGING;
        print_tings();

        putdown_forks(phil_ind);
        print_tings();

        dawdle();
        print_tings();    
    }

    // all done
    phils[phil_ind].state = CHANGING; 
    print_tings();

    pthread_exit(NULL);
    return NULL;
}


void pickup_forks(size_t phil_ind) {
    fork_action(phil_ind, PICKUP, true);
    fork_action(phil_ind, PICKUP, false);

    phils[phil_ind].state = EATING;
    print_tings();

    return;
}


void putdown_forks(size_t phil_ind) {
    fork_action(phil_ind, PUTDOWN, true);
    fork_action(phil_ind, PUTDOWN, false);

    phils[phil_ind].state = THINKING;
    print_tings();

    return;
}


void fork_action(size_t phil_ind, FORK_ACTION_t action, bool is_first_fork) {
    size_t fork_ind;
    FORK_SIDE_t side;

    if (is_first_fork) {
        if (IS_EVEN(phil_ind)) {
            fork_ind = GET_R_FORK_IND(phil_ind);
            side = RIGHT;
        } else {
            fork_ind = GET_L_FORK_IND(phil_ind);
            side = LEFT;
        }
    } else {
        if (IS_EVEN(phil_ind)) {
            fork_ind = GET_L_FORK_IND(phil_ind);
            side = LEFT;
        } else {
            fork_ind = GET_R_FORK_IND(phil_ind);
            side = RIGHT;
        }
    }

    switch (action) {
    case PICKUP:
        sem_wait(&forks[fork_ind]);
        update_phil_fork(fork_ind, action, side);
        break;
    case PUTDOWN:
        update_phil_fork(fork_ind, action, side);
        sem_post(&forks[fork_ind]);
        break;
    }

    print_tings();

    return;
}


void update_phil_fork(size_t phil_ind, FORK_ACTION_t action, FORK_SIDE_t side) {
    switch (action) {
    case PICKUP:
        if (side == RIGHT) {
            phils[phil_ind].has_right_fork = true;
        } else {
            phils[phil_ind].has_left_fork = true;
        }
        return;
    case PUTDOWN:
        if (side == RIGHT) {
            phils[phil_ind].has_right_fork = false;
        } else {
            phils[phil_ind].has_left_fork = false;
        }
        return;
    }
}


// given by Nico
void dawdle() {
    struct timespec tv;
    int msec = (int)(((double)random() / RAND_MAX) * 1000);

    tv.tv_sec = 0;
    tv.tv_nsec = 1000000 * msec;
    if (nanosleep(&tv, NULL) == -1){
        perror("nanosleep");
    }

    return;
}


void print_borderline() {
    int i, j;
    printf("|");
    for (i = 0; i < NUM_PHILOSOPHERS; i++){
        for (j = 0; j < NUM_PHILOSOPHERS + PADDING; j++) {
            printf("=");
        }
        printf("|"); 
    }
    printf("\n");
}


void print_header() {
    int i, j;

    print_borderline();
    
    printf("|");
    for (i = 0; i < NUM_PHILOSOPHERS; i++){
        for (j = 0; j < LEFT_HEADER_PAD; j++) {
            printf(" ");
        }
        printf("%c", phils[i].id);
        for (j = 0; j < RIGHT_HEADER_PAD; j++) {
            printf(" ");
        }
        printf("|");

    }
    printf("\n");
    
    print_borderline();

    return;
}


void print_tings() {
    int i, j;
    
    sem_wait(&mutex_print);
    printf("|");

    // Loop over philosophers
    for (i = 0; i < NUM_PHILOSOPHERS; i++) {
        printf(" ");
        
        // Print fork statuses for each philosopher
        for (j = 0; j < NUM_PHILOSOPHERS; j++) {
            if (GET_L_FORK_IND(i) == j) { 
                printf("%c", phils[i].has_left_fork ? FORK_TO_CHAR(j) : '-');
            } else if (GET_R_FORK_IND(i) == j) {
                printf("%c", phils[j].has_right_fork ? FORK_TO_CHAR(j) : '-');
            } else {
                printf("-");
            }
        }

        printf(" ");
        switch (phils[i].state) {
        case CHANGING:
            printf("      |");
            break;
        case EATING:
            printf("   Eat|");
            break;
        case THINKING:
            printf(" Think|");
            break;
        }
    }
    
    printf("\n");
    sem_post(&mutex_print);
}


