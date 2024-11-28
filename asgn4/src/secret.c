#include "secret.h"


/* Function prototypes for secret driver */
PRIVATE char* secret_name(void);
PRIVATE int secret_open(struct driver* d, message* m);
PRIVATE int secret_close(struct driver* d, message* m);
PRIVATE struct device* secret_prepare(int device);
PRIVATE int secret_transfer(int procnr, int opcode,
                   u64_t position, iovec_t* iov, unsigned nr_req);
PRIVATE void secret_geometry(struct partition* entry);
PRIVATE int secret_ioctl(struct driver* d, message* m);

/* SEF functions */
PRIVATE void sef_local_startup(void);
PRIVATE int sef_cb_init(int type, sef_init_info_t* info);
PRIVATE int sef_cb_lu_state_save(int);
PRIVATE int lu_state_restore(void);

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
PRIVATE int is_readable;
PRIVATE int owned;
PRIVATE uid_t owner_uid;
PRIVATE int read_pos;
PRIVATE int write_pos;
PRIVATE char buffer[SECRET_SIZE];

/* gets the name of the device */
PRIVATE char* secret_name(void) {
    return "secret";
} 


/* opens the device if free */
PRIVATE int secret_open(struct driver* d, message* m) {
    struct ucred caller_process;
    int reading, writing;
    /*
    In brightest code, in darkest compile, no relic of C89 shall beguile. 
    Let those who cling to its ancient style, beware 
    the future—it’s worth the trial.
    
    ( courtesy of chatgpt for this poem but seriously i hate c89, 
        c99 for life. let me declare variables anywhere)
    */

    getnucred(m->IO_ENDPT, &caller_process);
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
        else { /* writing to it so it becomes readable" */
            is_readable = TRUE;
        }
        owner_uid = caller_process.uid;
        owned = TRUE;
    }
    else {
        if (writing) { /* can only write once */
            printf("[OPEN] trying to write while owned\n");
            return EACCES;
        }
        else { 
            if (owner_uid != caller_process.uid) {
                printf("[OPEN] trying to read while owned by someone else\n");
                return EACCES;
            }
            is_readable = FALSE;
        }
    }

    fd_open_counter++;
    return OK;
}

/* closes the device */
PRIVATE int secret_close(struct driver* d, message* m) {
    struct ucred caller_process;

    getnucred(m->IO_ENDPT, &caller_process);

    fd_open_counter--;

    if (fd_open_counter == 0 && !is_readable) {
        owner_uid = NO_OWNER_ID;
        owned = FALSE;
        memset(buffer, 0, SECRET_SIZE);
        write_pos = 0;
        read_pos = 0;
    }

    return OK;
}


/* switch owners */
PRIVATE int secret_ioctl(struct driver* d, message* m) {
    uid_t new_owner_uid;
    int ret;

    if (m->REQUEST != SSGRANT) {
        return ENOTTY;
    }

    ret = sys_safecopyfrom(m->IO_ENDPT, (vir_bytes)m->IO_GRANT, 0,
                                (vir_bytes)&new_owner_uid, sizeof(uid_t), D);
    owner_uid = new_owner_uid;

    return ret;
}


/* prepare the device (stolen from hello) */
PRIVATE struct device* secret_prepare(int device) {
    secret_device.dv_base.lo = 0;
    secret_device.dv_base.hi = 0;
    secret_device.dv_size.lo = 0;
    secret_device.dv_size.hi = 0;
    return &secret_device;
}


/* transfer data to/from the device */
PRIVATE int secret_transfer(int procnr, int opcode, 
                   u64_t position, iovec_t* iov, unsigned nr_req) {
    int bytes;
    int ret;
    
    switch (opcode) {
        case DEV_GATHER_S: /* read from device */
            bytes = write_pos - read_pos < iov->iov_size ? 
                    write_pos - read_pos : iov->iov_size;
            if (bytes <= 0) { /* unsuccessful read but continue on */
                return OK;
            }

            if (ret = sys_safecopyto(procnr, iov->iov_addr, 0,
                                    (vir_bytes)(buffer + read_pos), 
                                    bytes, D) != OK) {
                return ret;
            }

            iov->iov_size -= bytes;
            read_pos += bytes;
            break;

        case DEV_SCATTER_S: /* write to device */
            bytes = SECRET_SIZE - write_pos < iov->iov_size ? 
                    SECRET_SIZE - write_pos : iov->iov_size;
            if (bytes <= 0) { /* unsuccessful write but continue on */
                return ENOSPC;
            }

            if (ret = sys_safecopyfrom(procnr, iov->iov_addr, 0,
                                    (vir_bytes)(buffer + write_pos), 
                                    bytes, D) != OK) {
                return ret;
            }

            iov->iov_size -= bytes;
            write_pos += bytes;
            break;

        default:
            return EINVAL;
    }

    return ret;
}


/* stolen from hello */
PRIVATE void secret_geometry(struct partition* entry) {
    printf("hello_geometry()\n");
    entry->cylinders = 0;
    entry->heads = 0;
    entry->sectors = 0;

    return;
}


/* save all the variables to be restored later */
PRIVATE int sef_cb_lu_state_save(int state) {
    SecretState_t cur_secret;
    cur_secret.fd_open_counter = fd_open_counter;
    cur_secret.owner_uid = owner_uid;
    cur_secret.owned = owned;
    cur_secret.write_pos = write_pos;
    cur_secret.read_pos = read_pos;
    memcpy(cur_secret.buffer, buffer, SECRET_SIZE);
    
    ds_publish_mem(SECRET_STATE_NAME, (char*)&cur_secret,
                    sizeof(SecretState_t), DSF_OVERWRITE);
    
    return OK;
}


PRIVATE int lu_state_restore() {
    SecretState_t restored_secret;
    size_t len;
    
    ds_retrieve_mem(SECRET_STATE_NAME, (char*)&restored_secret, &len);

    fd_open_counter = restored_secret.fd_open_counter;
    owner_uid = restored_secret.owner_uid;
    owned = restored_secret.owned;
    write_pos = restored_secret.write_pos;
    read_pos = restored_secret.read_pos;
    memcpy(buffer, restored_secret.buffer, SECRET_SIZE);

    ds_delete_mem(SECRET_STATE_NAME);

    return OK;
}


/* stolen from hello */
PRIVATE void sef_local_startup() {
    /* Register init callbacks. Use the same function for all event types */
    sef_setcb_init_fresh(sef_cb_init);
    sef_setcb_init_lu(sef_cb_init);
    sef_setcb_init_restart(sef_cb_init);
    
    /* Register live update callbacks */
    /* - Agree to update immediately when LU is requested in a valid state. */
    sef_setcb_lu_prepare(sef_cb_lu_prepare_always_ready);
    /* - Support live update starting from any standard state. */
    sef_setcb_lu_state_isvalid(sef_cb_lu_state_isvalid_standard);
    /* - Register a custom routine to save the state. */
    sef_setcb_lu_state_save(sef_cb_lu_state_save);
    /* Let SEF perform startup. */
    sef_startup();

    return;
}


/* initialize the driver */
PRIVATE int sef_cb_init(int type, sef_init_info_t *info) {
    int do_announce_driver = TRUE;

    switch(type) {
        case SEF_INIT_FRESH:
            fd_open_counter = 0;
            is_readable = FALSE;
            owned = FALSE;
            owner_uid = NO_OWNER_ID;
            read_pos = 0;
            write_pos = 0;
            memset(buffer, 0, SECRET_SIZE); 
            break;
        case SEF_INIT_LU:
            lu_state_restore();
            do_announce_driver = FALSE;
            printf("Secret driver: live update\n");
            break;
        case SEF_INIT_RESTART:
            printf("Secret driver: restart\n");
            break;
    }

    /* announce when up */
    if (do_announce_driver) {
        driver_announce();
    }
    /* been initialized properly */
    return OK;
}


/* stolen from hello */
PUBLIC int main(int argc, char **argv) {
    /*
     * Perform initialization.
     */
    sef_local_startup();
    
    /*
     * Run the main loop.
     */
    driver_task(&secret_tab, DRIVER_STD);
    
    return OK;
}




