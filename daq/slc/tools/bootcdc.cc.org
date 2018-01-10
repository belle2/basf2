#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <unistd.h>

#include <daq/slc/base/StringUtil.h>

/*
int cprcdc01[] = { 2046, 2047, 2039, 2040, 2041, 2012, 2013, 2014 };
int cprcdc02[] = { 2031, 2032, 2033, 2034, 2035, 2036, 2037, 2038 };
int cprcdc03[] = { 2015, 9002, 9003, 9004, 9005, 9006, 9007 };
int cprcdc04[] = { 2016, 2017, 2018, 2019, 2021, 2022, 2023 };
int cprcdc05[] = { 2024, 2025, 2026, 2027, 2028, 2029, 2030 };
int cprcdc07[] = { 2046, 2047, 2050, 2051, 2052 };
int cprcdc01[] = { 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 };
int cprcdc02[] = { 2031, 2032, 2033, 2034, 2035, 2036, 2037, 2038 };
int cprcdc03[] = { 2015, 9002, 9003, 9004, 9005, 9006, 9007 };
int cprcdc04[] = { 2016, 2017, 2018, 2019, 2021, 2022, 2023 };
int cprcdc05[] = { 2024, 2025, 2026, 2027, 2028, 2029, 2030 };
int cprcdc07[] = { 2046, 2047, 2050, 2051, 2052 };
*/
std::vector<int> cprcdc01;
std::vector<int> cprcdc02;
std::vector<int> cprcdc03;
std::vector<int> cprcdc04;
std::vector<int> cprcdc05;
std::vector<int> cprcdc06;
std::vector<int> cprcdc07;
std::vector<int> cprcdc08;
std::vector<int> cprcdc09;
std::vector<int> cprcdc00;

std::vector<int>& getcopper(const char* hostname, int& ncpr, std::string& ropcname)
{
  ropcname = Belle2::StringUtil::replace(hostname, "cdc0", "ropc20");
  if (strcmp(hostname, "cdc01") == 0) {
    ncpr = cprcdc01.size();
    return cprcdc01;
  } else if (strcmp(hostname, "cdc02") == 0) {
    ncpr = cprcdc02.size();
    return cprcdc02;
  } else if (strcmp(hostname, "cdc03") == 0) {
    ncpr = cprcdc03.size();
    return cprcdc03;
  } else if (strcmp(hostname, "cdc04") == 0) {
    ncpr = cprcdc04.size();
    return cprcdc04;
  } else if (strcmp(hostname, "cdc05") == 0) {
    ncpr = cprcdc05.size();
    return cprcdc05;
  } else if (strcmp(hostname, "cdc06") == 0) {
    ncpr = cprcdc06.size();
    return cprcdc06;
  } else if (strcmp(hostname, "cdc07") == 0) {
    ncpr = cprcdc07.size();
    return cprcdc07;
  } else if (strcmp(hostname, "cdc08") == 0) {
    ncpr = cprcdc08.size();
    return cprcdc08;
  } else if (strcmp(hostname, "cdc09") == 0) {
    ncpr = cprcdc09.size();
    return cprcdc09;
  }
  ncpr = 0;
  return cprcdc00;
}

int main(int argc, char** argv)
{
  /*
  for (int i = 2001; i <= 2009; i++) cprcdc01.push_back(i);
  for (int i = 2010; i <= 2017; i++) cprcdc02.push_back(i);
  for (int i = 2018; i <= 2025; i++) cprcdc03.push_back(i);
  for (int i = 2026; i <= 2034; i++) cprcdc04.push_back(i);
  for (int i = 2035; i <= 2042; i++) cprcdc05.push_back(i);
  for (int i = 2043; i <= 2050; i++) cprcdc06.push_back(i);
  for (int i = 2051; i <= 2059; i++) cprcdc07.push_back(i);
  for (int i = 2060; i <= 2067; i++) cprcdc08.push_back(i);
  for (int i = 2068; i <= 2075; i++) cprcdc09.push_back(i);
  */
  for (int i = 2001; i <= 2013; i++) cprcdc01.push_back(i);
  for (int i = 2014; i <= 2025; i++) cprcdc03.push_back(i);
  for (int i = 2026; i <= 2038; i++) cprcdc04.push_back(i);
  for (int i = 2039; i <= 2050; i++) cprcdc06.push_back(i);
  for (int i = 2051; i <= 2063; i++) cprcdc07.push_back(i);
  for (int i = 2064; i <= 2075; i++) cprcdc09.push_back(i);

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
  std::vector<int>& cprcdc(getcopper(hostname, ncpr, ropcname));
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
    sprintf(s, "nsm2socket 9090 -d");
    printf("%s\n", s);
    system(s);
    sprintf(s, "daqdbprovider %s -d", hostname);
    printf("%s\n", s);
    system(s);
  }
  if (cpr) {
    if (nsm) sleep(1);
    sprintf(s, "killall runcontrold rocontrold basf2 des_ser_ROPC_main eb0");
    printf("%s\n", s);
    system(s);
    for (int i = 0; i < ncpr; i++) {
      sprintf(s, "ssh cpr%d \"killall -9 cprcontrold basf2 des_ser_COPPER_main;\" ", cprcdc[i]);
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
    sprintf(s, "logcollectord -d");
    printf("%s\n", s);
    system(s);
  }
  return 0;
}
