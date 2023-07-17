#include <errno.h>
#include <linux/futex.h>
#include <sched.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <byteswap.h>

#define RELEASE
#ifndef RELEASE
#define DEBUG(fmt, ...)                                       \
  { fprintf(stderr, "[DEBUG] id=%d: " fmt "\n",               \
            whoami __VA_OPT__(,) __VA_ARGS__); }
#else
#define DEBUG(fmt, ...)
#endif


#define N_RING #####NUM#####

inline uint64_t rol(uint64_t in, int x) {
  uint64_t res;
  __asm__ __volatile__("rol %%cl, %%rax"  :"=a"(res) :"a"(in), "c"(x));
  return res;
}

inline uint64_t ror(uint64_t in, int x) {
  uint64_t res;
  __asm__ __volatile__("ror %%cl, %%rax" :"=a"(res) :"a"(in), "c"(x));
  return res;
}

#####FUNCS#####

int (*f[N_RING-1])(const char*) = {
  #####REFS#####
};

#define CMD_PING  0x1337cafe
#define CMD_FLAG  0x1337f146
#define CMD_REPLY 0x1337beef
#define CMD_QUIT  0x1337dead
#define BROADCAST -1

typedef struct {
  int lock;
  int dest;
  int cmd;
  int datalen;
  char data[0x50];
} message_t;

int futex(int* uaddr, int futex_op, int val, const struct timespec* timeout,
          int* uaddr2, int val3) {
  return syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr2, val3);
}

message_t *prev, *next;
int head = -1, tail = -1, whoami;

int setup_network() {
  int prev_id, id, last_id, pid;

  if ((last_id = shmget(IPC_PRIVATE, sizeof(message_t), 0600)) == -1)
    exit(EXIT_FAILURE);

  for (int i = 0; i < N_RING; i++) {
    if ((id = shmget(IPC_PRIVATE, sizeof(message_t), 0600)) == -1)
      exit(EXIT_FAILURE);

    if (i == 0) {
      head = id;
    } else {
      whoami = i;
      if (i == N_RING - 1) {
        tail = id;
        break;
      }
    }

    pid = fork();
    if (pid == -1)
      exit(EXIT_FAILURE);
    else if (pid) // parent
      break;

    prev_id = id;
  }

  if (head == id) {
    if ((prev = (message_t*)shmat(last_id, NULL, 0)) == (void*)-1)
      exit(EXIT_FAILURE);
  } else {
    if ((prev = (message_t*)shmat(prev_id, NULL, 0)) == (void*)-1)
      exit(EXIT_FAILURE);
  }

  if (tail == id) {
    if ((next = (message_t*)shmat(last_id, NULL, 0)) == (void*)-1)
      exit(EXIT_FAILURE);
  } else {
    if ((next = (message_t*)shmat(id, NULL, 0)) == (void*)-1)
      exit(EXIT_FAILURE);
  }

  next->lock = 0;
  return id;
}

void destroy_network(int id) {
  shmdt(prev);
  shmdt(next);
  shmctl(id, IPC_RMID, NULL);
}

int send_msg(int cmd, int dest, char *data, int size) {
  next->cmd = cmd;
  next->dest = dest;
  if (data) {
    next->datalen = size;
    memcpy(next->data, data, size);
  }

  int rc;
  do {
    rc = futex(&next->lock, FUTEX_WAKE, 1, NULL, NULL, 0);
    if (rc == -1) return 1;
  } while (rc == 0);

  return 0;
}

int recv_msg() {
  int rc;
  while (1) {
    do {
      rc = futex(&prev->lock, FUTEX_WAIT, 0, NULL, NULL, 0);
      if (rc == -1 && rc != EAGAIN) return 1;
    } while (rc != 0);

    if (prev->dest == BROADCAST || prev->dest == whoami)
      break;

    send_msg(prev->cmd, prev->dest, prev->data, prev->datalen);
  }

  return 0;
}

void handle_message() {
  while (!recv_msg()) {

    switch (prev->cmd) {
      case CMD_PING:
        DEBUG("Received CMD_PING");
        send_msg(CMD_PING, BROADCAST, NULL, 0);
        break;

      case CMD_FLAG:
        DEBUG("Received CMD_FLAG");
        if (f[whoami-1](prev->data)) {
          send_msg(CMD_REPLY, 0, "NG", 3);
        } else {
          send_msg(CMD_REPLY, 0, "OK", 3);
        }
        break;

      case CMD_QUIT:
        DEBUG("Received CMD_QUIT");
        if (whoami != N_RING - 1)
          send_msg(CMD_QUIT, BROADCAST, NULL, 0);
        return;

      default:
        DEBUG("Broken message: 0x%x", prev->cmd);
        break;
    }
  }
}

void network_main(char *buf, int size) {
  char flag[0x50] = {};
  memcpy(flag, buf, size > 0x50 ? 0x50 : size);

  /* Handshake */
  DEBUG("Sending handshake...");
  send_msg(CMD_PING, BROADCAST, NULL, 0);
  if (recv_msg() || prev->cmd != CMD_PING) {
    send_msg(CMD_QUIT, BROADCAST, NULL, 0);
    return;
  }
  DEBUG("Handshake successfull");

  /* Flag checker (vote) */
  int wrong = 0;
  for (int j = 0; j < 0x50; j+=8) {
    int vote = 0;

    for (int i = 1; i < N_RING; i++) {
      send_msg(CMD_FLAG, i, flag + j, 8);
      if (recv_msg() || prev->cmd != CMD_REPLY) {
        send_msg(CMD_QUIT, BROADCAST, NULL, 0);
        return;
      }

      if (strcmp(prev->data, "OK") == 0)
        vote++;
    }

    if (vote < 5)
      wrong = 1;
    putchar('.');
    fflush(stdout);
  }

  if (wrong) {
    puts("\nWrong...");
  } else {
    puts("\nCorrect!");
  }

  send_msg(CMD_QUIT, BROADCAST, NULL, 0);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: %s FLAG\n", argv[0]);
    return 1;
  }

  int id = setup_network();

  if (id == head) {
    network_main(argv[1], strlen(argv[1]) + 1);
  } else {
    handle_message();
  }

  if (id != tail)
    wait(NULL);
  destroy_network(id);

  return 0;
}
