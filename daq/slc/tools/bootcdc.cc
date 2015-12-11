#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <unistd.h>

#include <daq/slc/base/StringUtil.h>

int cprcdc01[] = { 2046, 2047, 2039, 2040, 2041, 2012, 2013, 2014 };
int cprcdc02[] = { 2046, 2047, 2039, 2040, 2041, 2012, 2013, 2014 };
int cprcdc03[] = { 2015 };
int cprcdc04[] = { 2016, 2017, 2018, 2019, 2021, 2022, 2023 };
int cprcdc05[] = { 2024, 2025, 2026, 2027, 2028, 2029, 2030 };

int* getcopper(const char* hostname, int& ncpr, std::string& ropcname)
{
  ropcname = Belle2::StringUtil::replace(hostname, "cdc0", "ropc20");
  if (strcmp(hostname, "cdc01") == 0) {
    ncpr = sizeof(cprcdc01) / sizeof(int);
    return cprcdc01;
  } else if (strcmp(hostname, "cdc02") == 0) {
    ncpr = sizeof(cprcdc02) / sizeof(int);
    return cprcdc02;
  } else if (strcmp(hostname, "cdc03") == 0) {
    ncpr = sizeof(cprcdc03) / sizeof(int);
    return cprcdc03;
  } else if (strcmp(hostname, "cdc04") == 0) {
    ncpr = sizeof(cprcdc04) / sizeof(int);
    return cprcdc04;
  } else if (strcmp(hostname, "cdc05") == 0) {
    ncpr = sizeof(cprcdc05) / sizeof(int);
    return cprcdc05;
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
  int* cprcdc = getcopper(hostname, ncpr, ropcname);
  char s[256];
  if (nsm) {
    sprintf(s, "killall nsmd2 runcontrold rocontrold");
    printf("%s\n", s);
    system(s);
    for (int i = 0; i < ncpr; i++) {
      sprintf(s, "ssh cpr%d \"killall nsmd2 cprcontrold\"", cprcdc[i]);
      printf("%s\n", s);
      system(s);
    }
    usleep(100000);
    sprintf(s, "bootnsmd2; bootnsmd2 -g");
    printf("%s\n", s);
    system(s);
    for (int i = 0; i < ncpr; i++) {
      sprintf(s, "ssh cpr%d \"source ~/.bash_profile; bootnsmd2 cpr%d\"",
              cprcdc[i], cprcdc[i]);
      printf("%s\n", s);
      system(s);
    }
  }
  if (cpr) {
    if (nsm) sleep(1);
    sprintf(s, "killall runcontrold rocontrold");
    printf("%s\n", s);
    system(s);
    for (int i = 0; i < ncpr; i++) {
      sprintf(s, "ssh cpr%d \"killall -9 cprcontrold basf2;\" ", cprcdc[i]);
      printf("%s\n", s);
      system(s);
    }
    usleep(100000);
    for (int i = 0; i < ncpr; i++) {
      sprintf(s, "ssh cpr%d \"source ~/.bash_profile; cprcontrold cpr%d -d\"", cprcdc[i], cprcdc[i]);
      printf("%s\n", s);
      system(s);
    }
    sprintf(s, "runcontrold %s -d; rocontrold %s -d", hostname, ropcname.c_str());
    printf("%s\n", s);
    system(s);
  }
  return 0;
}
