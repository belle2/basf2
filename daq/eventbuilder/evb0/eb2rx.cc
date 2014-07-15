#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include "onsen_tcp.h"
#include "basf2_tcp.h"
#include "log.h"

char help_message[] = "\n"
                      "v: verbose\n"
                      "n: nthread (default: 1)\n"
                      "h: this help\n";

int nthread = 1;
int nonsen = 1;
int need_reopen = 0;
int verbose = 0;

onsen_tcp* onsen = NULL;  // [nonsen]
basf2_tcp hlt;

void*
eof_watch(void*)
{
  log("EOF watcher start\n");

  while (1) {
    char buf[1];
    int ret;

    ret = recv(0, &buf, 1, MSG_PEEK | MSG_DONTWAIT);
    switch (ret) {
      case 0: /* EOF */
        log("EOF found from Storage\n");
        kill(getpid(), SIGTERM);
        exit(0);
        break;
      case 1:
        log("Unpexpected data comes from Storage to eb2rx\n");
        break;
      case -1: /* alive */
        break;
    }

    const timespec ts = {1, 0};
    nanosleep(&ts, NULL);
  }
}


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
    for (int i = id; i <= nonsen; i += nthread) {
      if (i == 0) {
        // log("fetch_event from HLT\n");
        hlt.fetch_event();
      } else {
        // log("fetch_event from ONSEN\n");
        onsen[i - 1].fetch_event();
      }
    }
    // log("got all event\n");
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
    for (int i = 0; i < nonsen; i++) {
      onsen[i].clear_input();
    }
    log("set need reopen\n");
    need_reopen = 1;

    for (int i = 0; i < nthread; i++) {
      log("send SIGUSR1 to slave %d\n", i);
      pthread_kill(slave[i].tid(), SIGUSR1);
    }
  }
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
  uint32_t exp_run = 0;

  program_name_in_syslog = "eb2rx";

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

  log("connect HLT at %s\n", argv[optind]);
  hlt.open(argv[optind]);
  optind++;

  nonsen = argc - optind;
  log("%d ONSEN(TCP)\n", nonsen);

  onsen = new onsen_tcp[nonsen];

  for (int i = optind, j = 0; i < argc; i++, j++) {
    log("connect ONSEN at %s\n", argv[i]);
    onsen[j].open(argv[i]);
  };

  start_threads(nthread);

#if 0
  pthread_t eof_watcher;
  pthread_create(&eof_watcher, NULL, eof_watch, NULL);
#endif

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
      for (int j = 0; j < nonsen; j++) {
        log("reopen %d\n", j);
        onsen[j].reopen();
      }
      need_reopen = 0;
      continue;
    }

    if (verbose) {
      for (int j = 0; j < nonsen; j++) {
        log("onsen[%d] magic in trailer %08x\n", j, onsen[j].magic());
      }
    }

    SendHeader header;
    SendTrailer trailer;

    int nboard = 1 + nonsen /* HLT + ONSENs*/;
    header.nboard(nboard);

    header.nevent(1);
    header.exp_run = hlt.header().exp_run;
    header.event_number = hlt.header().event_number;

    for (int j = 0; j < nonsen; j++) {
      if (0 != onsen[j].event_number()) {
        uint32_t en_onsen = onsen[j].event_number();
        if (en_onsen != header.event_number) {
          log("event number from ONSEN[%d] = %d, but that from HLT is %d\n",
              j,
              en_onsen, header.event_number);
        }
      }
    }

    trailer.magic = 0x7FFF0007;

    header.nword += hlt.nword();

    for (int i = 0; i < nonsen; i++) {
      header.nword += onsen[i].nword();
    }

    ret = fwrite(&header, sizeof(header), 1, stdout);
    assert(ret == 1);

    hlt.send_event(stdout);

    for (int i = 0; i < nonsen; i++) {
      onsen[i].send_event(stdout);
    }

    ret = fwrite(&trailer, sizeof(trailer), 1, stdout);
    assert(ret == 1);

    if (exp_run < header.exp_run) {
      exp_run = header.exp_run;
      event_number = 0;
    }

    if (event_number < 10 ||
        (event_number % 100 == 0 && event_number < 10000) ||
        event_number % 10000 == 0) {
      log("event %d\n", event_number);
    }
  }

}
