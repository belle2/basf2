#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <assert.h>
#include <unistd.h>
#include <sys/socket.h>

#define NTHREAD 10

int fd[NTHREAD][2];

pthread_t thr[NTHREAD];

void
usr1_handler(int arg)
{
  // signal(SIGUSR1, &usr1_handler);
  write(2, "xxx\n", 4);
  for (int i = 0; i < NTHREAD; i++) {
    close(fd[i][0]);
  }
}

void
enable_usr1()
{
  signal(SIGUSR1, &usr1_handler);
  return;
  sigset_t usr1, old;
  sigemptyset(&usr1);
  sigaddset(&usr1, SIGUSR1);
  int ret = pthread_sigmask(SIG_UNBLOCK, &usr1, &old);
  signal(SIGUSR1, &usr1_handler);
}

void*
reader(void* arg)
{
  enable_usr1();
  int id = *(int*)arg;
  fprintf(stderr, "id = %d\n", id);
  char buffer[4];
#if 0
  FILE* fp = fdopen(fd[id][0], "r");
  int ret = fread(buffer, sizeof(buffer), 1, fp);
#else
  int ret = read(fd[id][0], buffer, sizeof(buffer));
#endif
  fprintf(stderr, "id=%d ret=%d\n", id, ret);
  return NULL;
}

int
main(int argc, char** argv)
{
  enable_usr1();
  printf("pid = %d\n", getpid());

  for (int i = 0; i < NTHREAD; i++) {
    int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, fd[i]);
    assert(0 == ret);

    pthread_t thread;

    int* p = new int;
    *p = i;
    ret = pthread_create(&thr[i], NULL, reader, p);
    assert(0 == ret);
  }

  while (1) {
    pause();
    fprintf(stderr, "got signal\n");
    sleep(1);
#if 0
    for (int i = 0; i < NTHREAD; i++) {
      pthread_kill(thr[i], SIGUSR1);
    }
#endif
  }
}
