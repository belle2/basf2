// ----------------------------------------------------------------------
//  master.cc
//
//  A simple-minded NSM2 master program which does:
//  - text-base command line input with readline
//  - log message storage in a file
//    (no screen output as it conflicts with readline)
// ----------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <nsm2/nsmlib2.h>
#include <nsm2/belle2nsm.h>

extern "C" {
#ifdef USE_READLINE
#include <readline/readline.h>
#undef CTRL /* also defined in sys/ioctl.h in solaris */
  extern int add_history(char*);
#endif
}

// -- global variables --------------------------------------------------
const char* master_logdir = ".";
FILE* master_logfp = 0;

// -- xreopenlog ----------------------------------------------------
//    log directory
// ----------------------------------------------------------------------
void
xreopenlog()
{
  //static int logid = -1;
  static char logprefix[1024];
  //static int  isdirprefix = 0;
  char logfile[1024];
  time_t now = time(0);
  tm* cur = localtime(&now);
  struct stat statbuf;

  if (! logprefix[0]) {
    strcpy(logprefix, master_logdir);
    if (! logprefix[0]) strcpy(logprefix, ".");
    if (logprefix[strlen(logprefix) - 1] == '/') {
      logprefix[strlen(logprefix) - 1] = 0;
    }
    if (stat(logprefix, &statbuf) >= 0 && S_ISDIR(statbuf.st_mode)) {
      strcat(logprefix, "/master");
    }
  }

  sprintf(logfile, "%s.%04d%02d%02d.log", logprefix,
          cur->tm_year + 1900, cur->tm_mon + 1, cur->tm_mday);
  if (master_logfp != stdout) {
    if (master_logfp) fclose(master_logfp);
    if (!(master_logfp = fopen(logfile, "a"))) {
      printf("cannot open logfile %s\n", logfile);
      exit(1);
    }
  }
}
// -- xprintlog ---------------------------------------------------------
//    printlog
// ----------------------------------------------------------------------
void
xprintlog(const char* fmt, ...)
{
  va_list ap;
  char datebuf[32];
  void xlogtime(char* buf);

  if (master_logfp) {
    va_start(ap, fmt);
    xlogtime(datebuf);
    fputs(datebuf, master_logfp);
    vfprintf(master_logfp, fmt, ap);
    fputc('\n', master_logfp);
    va_end(ap);

    fflush(master_logfp);
  }
}
// -- xlogtime ----------------------------------------------------------
//    logtime
// ----------------------------------------------------------------------
void
xlogtime(char* buf)
{
  timeval now;
  tm* cur;
  static int lastday = -1;

  gettimeofday(&now, 0);
  cur = localtime((time_t*)&now.tv_sec);
  if (lastday != cur->tm_yday) {
    char datebuf[128];
    lastday = cur->tm_yday;
    xreopenlog();
    xprintlog(
      "------------------------------------------------------------------");
    sprintf(datebuf, "Date: %04d.%02d.%02d",
            cur->tm_year + 1900, cur->tm_mon + 1, cur->tm_mday);
    xprintlog(datebuf);
  }
  sprintf(buf, "%02d:%02d:%02d.%03d ",
          cur->tm_hour, cur->tm_min, cur->tm_sec, (int)now.tv_usec / 1000);
}
// -- xgetargs ----------------------------------------------------------
int
xgetargs(char* buf, char* argv[])
{
  int /*i,*/ argc = 0, inword = 0, inquote = 0;
  char* p = buf;

  while (*p) {
    if (*p == '\\') {
      if (*(p + 1) == 'n') {
        memmove(p, p + 1, strlen(p));
        *p = '\n';
      } else {
        memmove(p, p + 1, strlen(p));
      }
    } else if (*p == '"') {
      inquote = !inquote;
      memmove(p, p + 1, strlen(p));
      if (! inword) {
        argc++;
        *argv++ = p;
        inword = 1;
      }
      p--;
    } else if (isspace(*p) && !inquote && inword) {
      *p = 0;
      inword = 0;
    } else if (! isspace(*p) && !inword) {
      inword = 1;
      argc++;
      *argv++ = p;
    }
    p++;
  }
  if (inquote) return -1;
  return argc;
}
// -- ok_handler --------------------------------------------------------
//
// ----------------------------------------------------------------------
void
ok_handler(NSMmsg*, NSMcontext*)
{
  xprintlog("OK received");
}
// -- error_handler -----------------------------------------------------
//
// ----------------------------------------------------------------------
void
error_handler(NSMmsg*, NSMcontext*)
{
  xprintlog("ERROR received");
}
// -- main --------------------------------------------------------------
//    main does everything except callback functions
// ----------------------------------------------------------------------
int
main(int argc, char** argv)
{
  const char* program  = argv[0];
  const char* nodename = argv[1]; // need to check before using

  //char* buf = 0;
  //char* input = 0;
  //char* prev = 0;
  char* av[128];
  int ac;
  char* prompt;

  //int ret;

  xreopenlog();

  // ARGV check
  if (argc < 2) {
    printf("usage: %s <nodename>\n", program);
    return 1;
  }

  // INIT
  if (! b2nsm_init(nodename)) {
    xprintlog("%s: INIT %s", program, b2nsm_strerror());
    return 1;
  }
  xprintlog("%s: INIT", program);
  b2nsm_logging(master_logfp);

  // handlers
  if (b2nsm_callback("OK", ok_handler) < 0) {
    xprintlog("%s: CALLBACK(OK) %s", program, b2nsm_strerror());
    return 1;
  }
  if (b2nsm_callback("ERROR", ok_handler) < 0) {
    xprintlog("%s: CALLBACK(ERROR) %s", program, b2nsm_strerror());
    return 1;
  }

  // prompt
  prompt = (char*)malloc(strlen(nodename) + 2);
  strcpy(prompt, nodename);
  strcat(prompt, ">");

  // infinite loop
  const char* text = "KONNO";
  int len = strlen(text);
  while (1) {
#ifdef USE_READLINE
    /* On solaris, fgets returns without changing buf when it receives
       a network message (it doesn't happen on linux).  If the buffer
       is not initialized, it will produce a faulty input.
       */
    if (buf) free(buf);
    if (input) free(input);
    if (!(input = readline(prompt)))
      exit(1);
    buf = (char*)malloc(strlen(input) + 1);
    strcpy(buf, input);

    if (!prev || (*input && strcmp(prev, input) != 0)) {
      add_history(input);
      if (prev) free(prev);
      prev = (char*)malloc(strlen(input) + 1);
      strcpy(prev, input);
    }
#else
    static char buf[1024];
    buf[0] = 0;
    printf("%s", prompt);
    do {
      fgets(buf, sizeof(buf), stdin);
    } while (! buf[0]); /* to cope with EINTR */
#endif

    if ((ac = xgetargs(buf, av)) <= 0)
      continue;
    printf("ac=%d av[0]=%s\n", ac, av[0]);

    if (strcasecmp(av[0], "q") == 0 || strcasecmp(av[0], "quit") == 0) {
      break;
    } else if (strcasecmp(av[0], "boot") == 0) {
      if (ac < 1) {
        printf("usage: boot <node>\n");
      } else {
        b2nsm_sendany(av[1], "BOOT", 0, 0, len, text, NULL);
      }
    } else if (strcasecmp(av[0], "load") == 0) {
      if (ac < 1) {
        printf("usage: load <node>\n");
      } else {
        b2nsm_sendany(av[1], "LOAD", 0, 0, len, text, NULL);
      }
    } else if (strcasecmp(av[0], "start") == 0) {
      if (ac < 4 || ! isdigit(av[2][0]) || ! isdigit(av[3][0])) {
        printf("usage: start <node> <exp-number> <run-number>\n");
      } else {
        int expno = atoi(av[2]);
        int runno = atoi(av[3]);
        int params[2] = { expno, runno };
        b2nsm_sendany(av[1], "START", 2, params, len, text, NULL);
      }
    } else if (strcasecmp(av[0], "trigft") == 0) {
      if (ac < 5 || ! isdigit(av[3][0])) {
        printf("usage: trigft <node> <trg_mode=in/tlu/pulse/revo/random/possion> <dummy_rate> <trg_limit>\n");
      } else {
        int trg_mode = 0;
        if (strcasecmp(av[2], "aux") == 0) {
          trg_mode = 1;
        } else if (strcasecmp(av[2], "i") == 0) {
          trg_mode = 2;
        } else if (strcasecmp(av[2], "tlu") == 0) {
          trg_mode = 3;
        } else if (strcasecmp(av[2], "pulse") == 0) {
          trg_mode = 4;
        } else if (strcasecmp(av[2], "revo") == 0) {
          trg_mode = 5;
        } else if (strcasecmp(av[2], "random") == 0) {
          trg_mode = 6;
        } else if (strcasecmp(av[2], "possion") == 0) {
          trg_mode = 7;
        }
        int dummy_rate = atoi(av[3]);
        int trg_limit = atoi(av[4]);
        int params[3] = { trg_mode, dummy_rate, trg_limit };
        b2nsm_sendany(av[1], "TRIGFT", 3, params, len, text, NULL);
      }
    } else if (strcasecmp(av[0], "stop") == 0) {
      if (ac < 1) {
        printf("usage: stop <node>\n");
      } else {
        b2nsm_sendany(av[1], "STOP", 0, 0, len, text, NULL);
      }
    } else if (strcasecmp(av[0], "recover") == 0) {
      if (ac < 1) {
        printf("usage: recover <node>\n");
      } else {
        b2nsm_sendany(av[1], "RECOVER", 0, 0, len, text, NULL);
      }
    } else if (strcasecmp(av[0], "abort") == 0) {
      if (ac < 1) {
        printf("usage: abort <node>\n");
      } else {
        b2nsm_sendany(av[1], "ABORT", 0, 0, len, text, NULL);
      }
    } else if (strcasecmp(av[0], "log") == 0) {
      if (ac < 3) {
        printf("usage: log <message>\n");
      } else {
        len = strlen(av[2]);
        text = av[2];
        b2nsm_sendany(av[1], "LOG", 0, 0, len, text, NULL);
      }
    } else {
      printf("unknown request %s\n", av[0]);
    }
  }

  return 0; // never reached
}
// ----------------------------------------------------------------------
// -- (emacs outline mode setup)
// Local Variables: ***
// mode:outline-minor ***
// outline-regexp:"^// --[+ ]" ***
// End: ***
