#include <minix/drivers.h>
#include <minix/chardriver.h>
#include <stdio.h>
#include <stdlib.h>
#include <minix/ds.h>
#include "secret.h"


/* Function prototypes for secret driver */
// FORWARD _PROTOTYPE( char* secret_name, (void) );
// FORWARD _PROTOTYPE( int secret_open, (struct driver* d, message* m) );
// FORWARD _PROTOTYPE( int secret_close, (struct driver* d, message* m) );
// FORWARD _PROTOTYPE( struct device* secret_prepare, (int device) );
// FORWARD _PROTOTYPE( int secret_transfer, (int procnr, int opcode,
//                     u64_t position, iovec_t* iov, unsigned nr_req) );
// FORWARD _PROTOTYPE( void secret_geometry, (struct partition* entry) );
// FORWARD _PROTOTYPE( int secret_ioctl, (struct driver* d, message* m) );
char* secret_name(void);
int secret_open(struct driver* d, message* m);
int secret_close(struct driver* d, message* m);
struct device* secret_prepare(int device);
int secret_transfer(int procnr, int opcode,
                   u64_t position, iovec_t* iov, unsigned nr_req);
void secret_geometry(struct partition* entry);
int secret_ioctl(struct driver* d, message* m);

/* SEF functions */
// FORWARD _PROTOTYPE( void sef_local_startup, (void) );
// FORWARD _PROTOTYPE( int sef_cb_init, (int type, sef_init_info_t *info) );
// FORWARD _PROTOTYPE( int sef_cb_lu_state_save, (int) );
// FORWARD _PROTOTYPE( int lu_state_restore, (void) );
void sef_local_startup(void);
int sef_cb_init(int type, sef_init_info_t* info);
int sef_cb_lu_state_save(int);
int lu_state_restore(void);

/* Entry points to the hello driver */
PRIVATE struct driver secret_tab = {
    secret_name,
    secret_open,
    secret_close,
    secret_ioctl,
    secret_prepare,
    secret_transfer,
    nop_cleanup,
    secret_geometry,
    nop_alarm,
    nop_cancel,
    nop_select,
    do_nop,
};

/* the device */
PRIVATE struct device secret_device;

/* state variables */
PRIVATE int fd_open_counter;
PRIVATE int is_readable; // ? check if needed?
PRIVATE int owned;
PRIVATE uid_t owner_uid;
PRIVATE int read_pos;
PRIVATE int write_pos;
PRIVATE char buffer[SECRET_SIZE];

/* gets the name of the device */
char* secret_name(void) {
    printf("Secret driver starting\n");
    return "secret";
} 


/* opens the device if free */
int secret_open(struct driver* d, message* m) {
    struct ucred caller_process;
    int reading, writing;

    getnucred(m->IOENDPT, &caller_process);

    reading = m->COUNT & R_BIT;
    writing = m->COUNT & W_BIT; 
    if (!owned) {
        if (reading && writing) {
            printf("[OPEN] tring to RW while free\n");
            return EACCES;
        } 
        else if (reading) {
            is_readable = FALSE;
        }
        else if (writing) {
            is_readable = TRUE;
        }
        owner = caller_process.id;
        owned = TRUE;
    }
    else if (owned) {
        if (writing) {
            printf("[OPEN] tring to write while owned\n");
            return EACCES;
        }
        else if (reading) { // ? check if needed?
            if (owner != caller_process.id) {
                printf("[OPEN] tring to read while owned by someone else\n");
                return EACCES;
            }
            is_readable = FALSE;
        }
    }

    fd_open_counter++;
    return OK;
}

/* closes the device */
int secret_close(struct driver* d, message* m) {
    struct ucred caller_process;

    getnucred(m->IOENDPT, &caller_process);

    fd_open_counter--;
    if (fd_open_counter == 0 && !is_readable) {
        owner = -1;
        owned = FALSE;
        memset(buffer, 0, SECRET_SIZE);
        write_pos = 0;
        read_pos = 0;
    }

    return OK;
}




