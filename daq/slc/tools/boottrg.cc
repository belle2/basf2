#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <unistd.h>

#include <daq/slc/base/StringUtil.h>

int cprtrg01[] = {11001, 11002, 11003, 11004, 15001};
int cprtrg02[] = {5003, 5004};
int cprtrg03[] = {5005, 5006, 5007};
int cprtrg04[] = {5008, 5009, 5010};
int cprtrg05[] = {5011, 5012, 5013};
int cprtrg06[] = {5014, 5015};
int cprtrg07[] = {5016, 5017, 5018};
int cprtrg08[] = {6001, 6002, 6003};
int cprtrg09[] = {6004, 6005, 6006};
int cprtrg10[] = {6007, 6008, 13001};

int* getcopper(const char* hostname, int& ncpr, std::string& ropcname)
{
  ropcname = Belle2::StringUtil::replace(hostname, "trg0", "ropc90");
  if (strcmp(hostname, "trg01") == 0) {
    ncpr = sizeof(cprtrg01) / sizeof(int);
    return cprtrg01;
  } else if (strcmp(hostname, "trg02") == 0) {
    ncpr = sizeof(cprtrg02) / sizeof(int);
    return cprtrg02;
  } else if (strcmp(hostname, "trg03") == 0) {
    ncpr = sizeof(cprtrg03) / sizeof(int);
    return cprtrg03;
  } else if (strcmp(hostname, "trg04") == 0) {
    ncpr = sizeof(cprtrg04) / sizeof(int);
    return cprtrg04;
  } else if (strcmp(hostname, "trg05") == 0) {
    ncpr = sizeof(cprtrg05) / sizeof(int);
    return cprtrg05;
  } else if (strcmp(hostname, "trg06") == 0) {
    ncpr = sizeof(cprtrg06) / sizeof(int);
    return cprtrg06;
  } else if (strcmp(hostname, "trg07") == 0) {
    ncpr = sizeof(cprtrg07) / sizeof(int);
    return cprtrg07;
  } else if (strcmp(hostname, "trg08") == 0) {
    ncpr = sizeof(cprtrg08) / sizeof(int);
    return cprtrg08;
  } else if (strcmp(hostname, "trg09") == 0) {
    ncpr = sizeof(cprtrg09) / sizeof(int);
    return cprtrg09;
  } else if (strcmp(hostname, "trg10") == 0) {
    ncpr = sizeof(cprtrg10) / sizeof(int);
    return cprtrg10;
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
  int* cprtrg = getcopper(hostname, ncpr, ropcname);
  char s[256];
  if (nsm) {
    sprintf(s, "killall nsmd2 runcontrold rocontrold");
    printf("%s\n", s);
    system(s);
    for (int i = 0; i < ncpr; i++) {
      sprintf(s, "ssh cpr%d \"killall nsmd2 cprcontrold\"", cprtrg[i]);
      printf("%s\n", s);
      system(s);
    }
    usleep(100000);
    sprintf(s, "bootnsmd2; bootnsmd2 -g");
    printf("%s\n", s);
    system(s);
    for (int i = 0; i < ncpr; i++) {
      sprintf(s, "ssh cpr%d \"source ~/.bash_profile; bootnsmd2 cpr%d &\"",
              cprtrg[i], cprtrg[i]);
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
      sprintf(s, "ssh cpr%d \"killall -9 cprcontrold des_ser_COPPER_main;\" ", cprtrg[i]);
      printf("%s\n", s);
      system(s);
    }
    usleep(100000);
    for (int i = 0; i < ncpr; i++) {
      sprintf(s, "ssh cpr%d \"source ~/.bash_profile; cprcontrold cpr%d -d\"", cprtrg[i], cprtrg[i]);
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
