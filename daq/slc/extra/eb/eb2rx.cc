#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "mmap_statistics.h"
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

int listening_mode = 0;
const char* listening_port = NULL;

const char* statfile_up = "/dev/shm/eb2rx.up";
const char* statfile_down = "/dev/shm/eb2rx.down";

onsen_tcp* onsen = NULL;  // [nonsen]
basf2_tcp hlt;

eb_statistics* eb_stat = NULL;

void*
eof_watch(void*)
{
  log("[DEBUG] EOF watcher start\n");

  while (1) {
    char buf[1];
    int ret;

    ret = recv(0, &buf, 1, MSG_PEEK | MSG_DONTWAIT);
    switch (ret) {
      case 0: /* EOF */
        log("[FATAL] EOF found from Storage\n");
        kill(getpid(), SIGTERM);
        exit(0);
        break;
      case 1:
        log("[INFO] Unpexpected data comes from Storage to eb2rx\n");
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
    m_id(-1), m_tid(0)
  {
    pthread_mutex_init(&m_master_to_slave, NULL);
    pthread_mutex_lock(&m_master_to_slave);
    pthread_mutex_init(&m_slave_to_master, NULL);
    pthread_mutex_lock(&m_slave_to_master);
  };

  int id() const
  {
    return m_id;
  };

  void id(int i)
  {
    m_id = i;
  };

  pthread_t* tid_p()
  {
    return &m_tid;
  };

  pthread_t tid() const
  {
    return m_tid;
  };

  pthread_mutex_t slave_to_master() const
  {
    return m_slave_to_master;
  };

  pthread_mutex_t master_to_slave() const
  {
    return m_master_to_slave;
  };

  void wait_master()
  {
    pthread_mutex_lock(&m_master_to_slave);
  };

  void call_slave()
  {
    pthread_mutex_unlock(&m_master_to_slave);
  };

  void wait_slave()
  {
    pthread_mutex_lock(&m_slave_to_master);
  };

  void call_master()
  {
    pthread_mutex_unlock(&m_slave_to_master);
  };
};

thread_property* slave;

void
parse_option(int argc, char** argv)
{
  int ch;
  while (-1 != (ch = getopt(argc, argv, "vhn:l:u:d:"))) {
    switch (ch) {
      case 'u':
        statfile_up = optarg;
        break;
      case 'd':
        statfile_down = optarg;
        break;
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
      case 'l':
        listening_mode = 1;
        listening_port = optarg;
        break;
    }
  }
}

void*
slave_func(void* arg)
{
  thread_property& prop = *(thread_property*)arg;
  const int& id  = prop.id();

  log("[DEBUG] slave id %d is launched\n", id);

  int x;
  int len;
  int ret;

  while (1) {
    prop.wait_master();
    for (int i = id; i <= nonsen; i += nthread) {
      if (i == 0) {
        // log("fetch_event from HLT\n");
        hlt.fetch_event();

        if (eb_stat) {
          eb_stat->up(0).event = hlt.event_id64();
          eb_stat->up(0).byte = hlt.nbyte();
        }

        const uint32_t en = hlt.event_number();
        if (en < 10) {
          log("[INFO] got event %d from HLT\n", en);
        }
      } else {
        // log("fetch_event from ONSEN\n");
        onsen[i - 1].fetch_event();

        if (eb_stat) {
          eb_stat->up(i).event = onsen[i - 1].event_id64();
          eb_stat->up(i).byte = onsen[i - 1].nbyte();
        }

        const uint32_t en = onsen[i - 1].event_number();
        if (en < 10) {
          log("[INFO] got event %d from ONSEN(%d)\n", en, i - 1);
        }
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
  log("[DEBUG] actual hup handler start....\n");
  while (ret = read(hup_pipe[0], &ch, 1)) {
    if (0 == ret)
      break;

    log("[INFO] clear input\n");
    for (int i = 0; i < nonsen; i++) {
      onsen[i].clear_input();
    }
    log("[INFO] set need reopen\n");
    need_reopen = 1;

    for (int i = 0; i < nthread; i++) {
      log("[INFO] send SIGUSR1 to slave %d\n", i);
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
  int listening_socket = -1;

  program_name_in_syslog = "eb2rx";

  ret = pipe(hup_pipe);
  if (ret) {
    log("[FATAL] make pipe failed: %s\n", strerror(errno));
    exit(1);
  }

  pthread_t hup_thread;

  pthread_create(&hup_thread, NULL, actual_hup_handler, NULL);

  signal(SIGHUP, hup_handler);
  signal(SIGUSR1, usr1_handler);

  parse_option(argc, argv);

  for (int i = optind; i < argc; i++) {
    log("[DEBUG] ARG %d %s\n", i, argv[i]);
  }

  eb_stat = new eb_statistics(statfile_up, 50, statfile_down, 1);

  FILE* fdown = stdout;

  if (listening_mode) {
    addrinfo hint, *res = NULL;

    memset(&hint, 0, sizeof(hint));

    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;

    int ret = getaddrinfo(NULL, listening_port, &hint, &res);
    if (ret != 0) {
      log("[FATAL] gai error %s", gai_strerror(ret));
      exit(1);
    }

    listening_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    assert(listening_socket > 0);

    int on = 1;
    ret = setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    assert(ret == 0);

    ret = bind(listening_socket, res->ai_addr, res->ai_addrlen);
    assert(ret == 0);

    ret = listen(listening_socket, 5);
    assert(ret == 0);

    sockaddr a;
    socklen_t len = sizeof(a);

    int sdown = accept(listening_socket, &a, &len);

    char down[256];

    log("[INFO] accepted from %s\n", inet_ntop(res->ai_family, &a, down, sizeof(down)));

    fdown = fdopen(sdown, "w");
    assert(fdown);

  } else {
    fdown = stdout;
  }

  sockaddr_in sa;
  socklen_t len = sizeof(sa);
  if (getsockname(fileno(fdown), (sockaddr*)&sa, &len)) {
    log("failed to getsockname of downstream\n");
  }
  eb_stat->down(0).addr = sa.sin_addr.s_addr;
  eb_stat->down(0).port = sa.sin_port;

  const char* hlt_arg = argv[optind];
  log("[DEBUG] %s HLT(TCP)\n", hlt_arg);

  if (getsockname(fileno(hlt.file()), (sockaddr*)&sa, &len)) {
    log("failed to getsockname of HLT\n");
  }
  eb_stat->up(0).addr = sa.sin_addr.s_addr;
  eb_stat->up(0).port = sa.sin_port;

  optind++;

  nonsen = argc - optind;
  log("[DEBUG] %d ONSEN(TCP)\n", nonsen);

  onsen = new onsen_tcp[nonsen];

  for (int i = optind, j = 0; i < argc; i++, j++) {
    log("[INFO] connecting ONSEN(%d) at %s\n", j, argv[i]);
    onsen[j].open(argv[i]);
    if (getsockname(fileno(onsen[j].file()), (sockaddr*)&sa, &len)) {
      log("failed to getsockname of ONSEN(%d)\n", j);
    }
    eb_stat->up(1 + j).addr = sa.sin_addr.s_addr;
    eb_stat->up(1 + j).port = sa.sin_port;
  };

  log("[INFO] connecting HLT at %s\n", hlt_arg);
  hlt.open(hlt_arg, "r", -1, 360000);

  log("[INFO] connected to all ONSEN and HLT\n");

  start_threads(nthread);

#if 0
  pthread_t eof_watcher;
  pthread_create(&eof_watcher, NULL, eof_watch, NULL);
#endif

  for (int event_number = 0;; ++event_number) {

    if (verbose || (event_number < 3))
      log("[DEBUG] reading event %d\n", event_number);

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

    const uint64_t hlt_eid64 = hlt.event_id64();

    int mismatch = 0;

    for (int j = 0; j < nonsen; j++) {
#if 0
      if (0 != onsen[j].event_number()) {
        uint32_t en_onsen = onsen[j].event_number();
        if (en_onsen != header.event_number) {
          log("event number from ONSEN[%d] = %d, but that from HLT is %d\n",
              j,
              en_onsen, header.event_number);
        }
      }
#else
      if (0 != onsen[j].event_id64()) {
        uint64_t eid64 = onsen[j].event_id64();
        if (eid64 != hlt_eid64) {
          mismatch = 1;
          log("event number from ONSEN[%d] = %016llx, but that from HLT is %016llx\n",
              j, eid64, hlt_eid64);
        }
      }
#endif
    }

    while (mismatch) {
      mismatch = 0;
      uint64_t max_id = hlt.event_id64();
      for (int j = 0; j < nonsen; j++) {
        if (max_id < onsen[j].event_id64()) {
          max_id = onsen[j].event_id64();
        }
      }

      hlt.skip_to(max_id);
      for (int j = 0; j < nonsen; j++) {
        onsen[j].skip_to(max_id);
      }

      if (max_id != hlt.event_id64()) {
        mismatch = 1;
        continue;
      }
      for (int j = 0; j < nonsen; j++) {
        if (max_id != onsen[j].event_id64()) {
          mismatch = 1;
          break;
        }
      }

      if (mismatch == 0) {
        log("[INFO] recovered from mismatch\n");
      }
    }

    trailer.magic = 0x7FFF0007;

    header.nword += hlt.nword();

    for (int i = 0; i < nonsen; i++) {
      header.nword += onsen[i].nword();
    }

    ret = fwrite(&header, sizeof(header), 1, fdown);
    assert(ret == 1);

    hlt.send_event(fdown);

    for (int i = 0; i < nonsen; i++) {
      onsen[i].send_event(fdown);
    }

    ret = fwrite(&trailer, sizeof(trailer), 1, fdown);
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
