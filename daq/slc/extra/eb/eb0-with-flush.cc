#include <assert.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include "b2eb_format.h"
#include "log.h"
#include "basf2_tcp.h"
#include "ftsw_tcp.h"

const char* help_message =
  "h: show help message\n"
  "-: receive data from stdin (only single mode)\n";

int stdin_mode = 0;

int number_of_copper = 1;

int nboard = 0; /* number of input stream */

char* ftsw_name = NULL;
char* tlu_name = NULL;
ftsw_tcp ftsw;
ftsw_tcp tlu;

pthread_t flusher;

void parse_option(int argc, char** argv)
{
  int ch;
  while (-1 != (ch = getopt(argc, argv, "F:T:h"))) {
    switch (ch) {
      case 'F':
        ftsw_name = optarg;
        break;
      case 'T':
        tlu_name = optarg;
        break;
      case 'h':
        fputs(help_message, stderr);
        exit(0);
        break;
      case '-':
        stdin_mode = 1;
        break;
    }
  }
}

void*
flush_stdout(void*)
{
  while (1) {
    printf("Check 1\n");
    poll(NULL, 0, 100); /* 100ms sleep */
    printf("Check 2\n");
    if (ftrylockfile(stdout)) {
      printf("Check 3\n");
      fflush(stdout);
      funlockfile(stdout);
      printf("Check 4\n");
    }
    printf("Check 5\n");
  }
}

int
main(int argc, char** argv)
{
  int ret;

  program_name_in_syslog = "eb0";

  parse_option(argc, argv);
  for (int i = optind; i < argc; i++) {
    log("argv[%d] %s\n", i, argv[i]);
  }

  int ncopper = argc - optind;
  log("%d COPPERs\n", ncopper);

  nboard = ncopper;


  if (ftsw_name) {
    log("FTSW %s\n", ftsw_name);
    ftsw.open(ftsw_name);
    nboard ++;
  }

  if (tlu_name) {
    log("TLU %s\n", tlu_name);
    tlu.open(tlu_name);
    nboard ++;
  }

  basf2_tcp* copper = new basf2_tcp[ncopper];
  for (int i = optind, j = 0; i < argc; i++, j++) {
    copper[j].open(argv[i], "r", 32 * 1024 * 1024);
  };

#if 0
  char* stdout_vbuf = new char[basf2_tcp::max_event_size];
  ret = setvbuf(stdout, stdout_vbuf, _IOFBF, basf2_tcp::max_event_size);
  if (ret) {
    log("failed to setvbuf of stdout %s\n", strerror(errno));
  }
#endif

  ret = pthread_create(&flusher, NULL, flush_stdout, NULL);

  log("ready to process data\n");

  /* BOR */

  uint32_t expected_event_number = 0;

  for (int event_number = 0;; ++event_number) {

    SendHeader header;
    SendTrailer trailer;

    memset(&header, 0, sizeof(header));
    header.nword = (sizeof(header) + sizeof(trailer)) / 4;
    header.nword_in_header = sizeof(header) / 4;
    header.nboard(nboard);
    header.nevent(1);
    /* header.event_number = event_number; */
    // log("size %d\n", header.nword);
    trailer.magic = 0x7FFF0007;

    uint64_t latest_event_id64 = 0;

    if (ftsw.is_opened()) {
      ftsw.fetch_event();
      header.nword += ftsw.nword();
      if (latest_event_id64 < ftsw.event_id64()) {
        latest_event_id64 = ftsw.event_id64();
      }
    }

    if (tlu.is_opened()) {
      tlu.fetch_event();
      header.nword += tlu.nword();
      if (latest_event_id64 < tlu.event_id64()) {
        latest_event_id64 = tlu.event_id64();
      }
    }

    for (int i = 0; i < ncopper; i++) {
      int nbyte = copper[i].fetch_event();

      copper[i].check_nword();

      if (latest_event_id64 < copper[i].event_id64()) {
        latest_event_id64 = copper[i].event_id64();
      }

      header.nword += copper[i].nword_of_body();

      // copper[i].dump_event();

      uint32_t trailer_magic = copper[i].trailer_magic();
      if ((trailer_magic & 0xFFFF0000) != 0x7FFF0000) {
        log(" bad trailer magic(0x%08x), possibly word count (0x%08x) is wrong\n",
            trailer_magic, copper[i].nword());
      }
    }

    /* skip bogus event */

    if (ftsw.is_opened()) {
      ftsw.skip_to(latest_event_id64);
    }
    if (tlu.is_opened()) {
      tlu.skip_to(latest_event_id64);
    }
    for (int i = 0; i < ncopper; i++) {
      copper[i].skip_to(latest_event_id64);
    }

    /* check time_tag consistency */


    if (ftsw.is_opened()) {
      uint64_t time_tag = ftsw.time_tag();

      for (int i = 0; i < ncopper; i++) {
        if (time_tag != copper[i].time_tag()) {
          log("timetag %16llx from %s:%s != timetag %16llx from ftsw\n",
              copper[i].time_tag(),
              copper[i].remote_host(),
              copper[i].remote_port(),
              time_tag);
        }
      }
    } else if (ncopper > 1) {
      uint64_t time_tag = copper[0].time_tag();
      for (int i = 1; i < ncopper; i++) {
        if (time_tag != copper[i].time_tag()) {
          log("timetag %16llx from %s:%s != timetag %16llx from %s:%s\n",
              copper[i].time_tag(),
              copper[i].remote_host(),
              copper[i].remote_port(),
              time_tag,
              copper[0].remote_host(),
              copper[0].remote_port()
             );
        }
      }
    }


#if 0
    if (event_number == 0 && copper[0].is_ftsw() == 0) {
      log("FIRST module should be TTD, but %08x\n", copper[0].header().node_id);
    }
#endif

    header.exp_run = copper[0].header().exp_run;
    header.event_number = copper[0].header().event_number;

    if (header.event_number != expected_event_number) {
      log("event number from the first copper is 0x%08x, but 0x%08x is expected\n",
          header.event_number, expected_event_number);
    }

    expected_event_number = header.event_number + 1;

    ret = fwrite(&header, sizeof(header), 1, stdout);
    if (ret != 1) {
      log("event#%d failed to write header: %s\n", event_number, strferror(stdout, "r"));
      exit(1);
    }

    if (ftsw.is_opened()) {
      ftsw.send_event(stdout);

      if (ftsw.header().event_number < 3) {
        ftsw.dump_event();
      }
    }

    if (tlu.is_opened()) {
      tlu.send_event(stdout);
    }

    for (int i = 0; i < ncopper; i++) {
      copper[i].send_body(stdout);
    }

    ret = fwrite(&trailer, sizeof(trailer), 1, stdout);
    if (ret != 1) {
      log("event#%d failed to write trailer: %s\n", event_number, strferror(stdout, "r"));
      exit(1);
    }

    if (event_number < 10 ||
        (event_number < 10000 && event_number % 100 == 0) ||
        event_number % 10000 == 0)  {
      log("event_no = %d \n", event_number);
    }
  }

  /* EOR */
}
