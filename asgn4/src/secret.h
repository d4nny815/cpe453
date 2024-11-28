#ifndef SECRET_H
#define SECRET_H

#include <minix/drivers.h>
#include <minix/driver.h>
#include <stdio.h>
#include <stdlib.h>
#include <minix/ds.h>
#include <sys/ioc_secret.h>

#define SECRET_MSG ("I am a secret hehe")

#ifndef SECRET_SIZE
#define SECRET_SIZE (8192)
#endif

typedef struct SecretState_t {
    int fd_open_counter;
    int is_readable;
    int owned;
    uid_t owner_uid;
    int read_pos;
    int write_pos;
    char buffer[SECRET_SIZE];
} SecretState_t;
#define SECRET_STATE_NAME ("secret_state")

#endif /* secret.h */
