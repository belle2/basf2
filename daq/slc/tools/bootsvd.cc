#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <unistd.h>

#include <daq/slc/base/StringUtil.h>

int cprsvd11[] = {1049, 1050, 1051, 1052, 1053};

int* getcopper(const char* hostname, int& ncpr, std::string& ropcname)
{
  ncpr = sizeof(cprsvd11) / sizeof(int);
  ropcname = "ropc11";
  return cprsvd11;
}

int main(int argc, char** argv)
{
  bool nsm = false;
  bool cpr = false;
  if (argc == 1) {
    nsm = true;
    cpr = true;
  }
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-nsm") == 0) {
      nsm = true;
      continue;
    } else if (strcmp(argv[i], "-cpr") == 0) {
      cpr = true;
      continue;
    }
  }

  const char* hostname = getenv("HOSTNAME");
  std::string ropcname;
  int ncpr = 0;
  int* cprecl = getcopper(hostname, ncpr, ropcname);
  char s[256];
  if (nsm) {
    sprintf(s, "killall nsmd2 runcontrold rocontrold des_ser_COPPER_main des_ser_ROPC_main");
    printf("%s\n", s);
    system(s);
    for (int i = 0; i < ncpr; i++) {
      sprintf(s, "ssh cpr%03d \"killall nsmd2 cprcontrold\"", cprecl[i]);
      printf("%s\n", s);
      system(s);
    }
    usleep(100000);
    sprintf(s, "bootnsmd2; bootnsmd2 -g");
    printf("%s\n", s);
    system(s);
    for (int i = 0; i < ncpr; i++) {
      sprintf(s, "ssh cpr%03d \"source ~/.bash_profile; bootnsmd2 cpr%03d &\"",
              cprecl[i], cprecl[i]);
      printf("%s\n", s);
      system(s);
    }
    sprintf(s, "nsm2socket 9090 -d");
    printf("%s\n", s);
    system(s);
  }
  if (cpr) {
    if (nsm) sleep(1);
    sprintf(s, "killall runcontrold rocontrold");
    printf("%s\n", s);
    system(s);
    for (int i = 0; i < ncpr; i++) {
      sprintf(s, "ssh cpr%03d \"killall -9 cprcontrold basf2;\" ", cprecl[i]);
      printf("%s\n", s);
      system(s);
    }
    usleep(100000);
    for (int i = 0; i < ncpr; i++) {
      sprintf(s, "ssh cpr%03d \"source ~/.bash_profile; cprcontrold cpr%03d -d\"", cprecl[i], cprecl[i]);
      printf("%s\n", s);
      system(s);
    }
    sprintf(s, "runcontrold %s -d; rocontrold %s -d; runcontrold rc_ttd -d", "svd01", ropcname.c_str());
    printf("%s\n", s);
    system(s);
    sprintf(s, "logcollectord -d");
    printf("%s\n", s);
    system(s);
  }
  return 0;
}
