#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include "basf2_tcp.h"
#include "log.h"

char help_message[] = "\n"
                      "v: verbose\n"
                      "n: nthread (default: 4)\n"
                      "h: this help\n";

int nthread = 4;
int nropc = 1;
int need_reopen = 0;
int verbose = 0;

basf2_tcp* ropc = NULL;  // [nropc]

class thread_property {
private:
  int m_id;
  pthread_t m_tid;
  pthread_mutex_t m_slave_to_master;
  pthread_mutex_t m_master_to_slave;
public:

  thread_property() :
    m_id(-1), m_tid(0) {
    pthread_mutex_init(&m_master_to_slave, NULL);
    pthread_mutex_lock(&m_master_to_slave);
    pthread_mutex_init(&m_slave_to_master, NULL);
    pthread_mutex_lock(&m_slave_to_master);
  };

  int id() const {
    return m_id;
  };

  void id(int i) {
    m_id = i;
  };

  pthread_t* tid_p() {
    return &m_tid;
  };

  pthread_t tid() const {
    return m_tid;
  };

  pthread_mutex_t slave_to_master() const {
    return m_slave_to_master;
  };

  pthread_mutex_t master_to_slave() const {
    return m_master_to_slave;
  };

  void wait_master() {
    pthread_mutex_lock(&m_master_to_slave);
  };

  void call_slave() {
    pthread_mutex_unlock(&m_master_to_slave);
  };

  void wait_slave() {
    pthread_mutex_lock(&m_slave_to_master);
  };

  void call_master() {
    pthread_mutex_unlock(&m_slave_to_master);
  };
};

thread_property* slave;

void
parse_option(int argc, char** argv)
{
  int ch;
  while (-1 != (ch = getopt(argc, argv, "vhn:"))) {
    switch (ch) {
      case 'v':
        verbose = 1;
        break;
      case 'n':
        nthread = strtol(optarg, 0, 0);
        break;
      case 'h':
        log(help_message);
        exit(0);
        break;
    }
  }
}

void*
slave_func(void* arg)
{
  thread_property& prop = *(thread_property*)arg;
  const int& id  = prop.id();

  log("id %d is launched\n", id);

  int x;
  int len;
  int ret;

  while (1) {
    prop.wait_master();
    for (int i = id; i < nropc; i += nthread) {
      ropc[i].fetch_event();
    }
    prop.call_master();
  }
}

void
start_threads(int nthread)
{
  slave = new thread_property[nthread];
  for (int i = 0; i < nthread; i++) {
    slave[i].id(i);
    pthread_create(slave[i].tid_p(), NULL, slave_func, &slave[i]);
  }
}

int hup_pipe[2];

void
hup_handler(int arg)
{
  static char ch = 0;
  write(hup_pipe[1], &ch, ++ch);
}

void*
actual_hup_handler(void* arg)
{
  int ret;
  char ch;
  log("actual hup handler start....\n");
  while (ret = read(hup_pipe[0], &ch, 1)) {
    if (0 == ret)
      break;

    log("clear input\n");
    for (int i = 0; i < nropc; i++) {
      ropc[i].clear_input();
    }
    log("set need reopen\n");
    need_reopen = 1;

    for (int i = 0; i < nthread; i++) {
      log("send SIGUSR1 to slave %d\n", i);
      pthread_kill(slave[i].tid(), SIGUSR1);
    }
  }
  return 0;
}

void
usr1_handler(int arg)
{
  /* just EINTR read in fread */
  return;
}


int
main(int argc, char** argv)
{
  int ret;

  ret = pipe(hup_pipe);
  if (ret) {
    log("make pipe failed: %s\n", strerror(errno));
    exit(1);
  }

  pthread_t hup_thread;

  pthread_create(&hup_thread, NULL, actual_hup_handler, NULL);

  signal(SIGHUP, hup_handler);
  signal(SIGUSR1, usr1_handler);

  parse_option(argc, argv);

  for (int i = optind; i < argc; i++) {
    log("%d %s\n", i, argv[i]);
  }

  nropc = argc - optind;
  log("%d ROPCs\n", nropc);

  ropc = new basf2_tcp[nropc];

  for (int i = optind, j = 0; i < argc; i++, j++) {
    ropc[j].open(argv[i]);
  };

  start_threads(nthread);

  for (int event_number = 0;; ++event_number) {

    if (verbose)
      log("reading event %d\n", event_number);

    /* Start slaves */

    for (int j = 0; j < nthread; j++) {
      slave[j].call_slave();
    }

    /* Wait for slaves */
    for (int j = 0; j < nthread; j++) {
      slave[j].wait_slave();
    }

    if (need_reopen) {
      for (int j = 0; j < nropc; j++) {
        log("reopen %d\n", j);
        ropc[j].reopen();
      }
      need_reopen = 0;
      continue;
    }

    if (verbose) {
      for (int j = 0; j < nropc; j++) {
        log("ropc[%d] magic in trailer %x\n", j, ropc[j].trailer().magic);
      }
    }

    SendHeader header;
    SendTrailer trailer;


#if 0 /* done in the SendHeader constructor */
    memset(&header, 0, sizeof(header));
    header.nword = (sizeof(header) + sizeof(trailer)) / 4;
    header.nword_in_header = sizeof(header) / 4;
#endif

    int nboard = 0;
    for (int j = 0; j < nropc; j++) {
      nboard += ropc[j].header().nboard();
      header.nword += ropc[j].nword_of_body();
    }
    header.nboard(nboard);

    header.nevent(1);
    header.exp_run = ropc[0].header().exp_run;
    header.event_number = ropc[0].header().event_number;

    trailer.magic = 0x7FFF0007;

    ret = fwrite(&header, sizeof(header), 1, stdout);
    assert(ret == 1);

    for (int i = 0; i < nropc; i++) {
      ropc[i].send_body(stdout);
    }

    ret = fwrite(&trailer, sizeof(trailer), 1, stdout);
    assert(ret == 1);

    if (event_number % 10000 == 0) {
      log("event %d\n", event_number);
    }
  }

}
