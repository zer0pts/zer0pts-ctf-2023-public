#ifndef CRED_H
#define CRED_H

#include <stdint.h>

struct user_cap_data_struct {
    uint32_t effective;
    uint32_t permitted;
    uint32_t inheritable;
};

void alloc_n_creds(int uring_fd, size_t n_creds);
void delete_n_creds(int uring_fd, size_t n_creds);

#endif  // CRED_H