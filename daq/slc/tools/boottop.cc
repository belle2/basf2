#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <unistd.h>

#include <daq/slc/base/StringUtil.h>

int cprtop01[] = { 3001, 3002, 3003, 3004, 3005 };
int cprtop02[] = { 3006, 3007, 3008, 3009, 3010 };
int cprtop03[] = { 3011, 3012, 3013, 3014, 3015, 3016 };

int* getcopper(const char* hostname, int& ncpr, std::string& ropcname)
{
  ropcname = Belle2::StringUtil::replace(hostname, "top0", "ropc30");
  if (strcmp(hostname, "top01") == 0) {
    ncpr = sizeof(cprtop01) / sizeof(int);
    return cprtop01;
  } else if (strcmp(hostname, "top02") == 0) {
    ncpr = sizeof(cprtop02) / sizeof(int);
    return cprtop02;
  } else if (strcmp(hostname, "top03") == 0) {
    ncpr = sizeof(cprtop03) / sizeof(int);
    return cprtop03;
  }
  ncpr = 0;
  return NULL;
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
  int* cprtop = getcopper(hostname, ncpr, ropcname);
  char s[256];
  if (nsm) {
    sprintf(s, "killall nsmd2 runcontrold rocontrold");
    printf("%s\n", s);
    system(s);
    for (int i = 0; i < ncpr; i++) {
      sprintf(s, "ssh cpr%d \"killall nsmd2 cprcontrold\"", cprtop[i]);
      printf("%s\n", s);
      system(s);
    }
    usleep(100000);
    sprintf(s, "bootnsmd2; bootnsmd2 -g");
    printf("%s\n", s);
    system(s);
    for (int i = 0; i < ncpr; i++) {
      sprintf(s, "ssh cpr%d \"source ~/.bash_profile; bootnsmd2 cpr%d\"",
              cprtop[i], cprtop[i]);
      printf("%s\n", s);
      system(s);
    }
    sprintf(s, "nsm2socket 9090 -d");
    printf("%s\n", s);
    system(s);
  }
  if (cpr) {
    if (nsm) sleep(1);
    sprintf(s, "killall runcontrold rocontrold des_ser_ROPC_main eb0");
    printf("%s\n", s);
    system(s);
    for (int i = 0; i < ncpr; i++) {
      sprintf(s, "ssh cpr%d \"killall -9 cprcontrold des_ser_COPPER_main;\" ", cprtop[i]);
      printf("%s\n", s);
      system(s);
    }
    usleep(100000);
    for (int i = 0; i < ncpr; i++) {
      sprintf(s, "ssh cpr%d \"source ~/.bash_profile; cprcontrold cpr%d -d\"", cprtop[i], cprtop[i]);
      printf("%s\n", s);
      system(s);
    }
    sprintf(s, "runcontrold %s -d; rocontrold %s -d", hostname, ropcname.c_str());
    printf("%s\n", s);
    system(s);
    sprintf(s, "logcollectord -d");
    printf("%s\n", s);
    system(s);
  }
  return 0;
}
