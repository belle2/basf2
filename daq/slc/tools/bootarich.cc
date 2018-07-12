#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <unistd.h>

#include <daq/slc/base/StringUtil.h>

/*
int cprarich01[] = { 2046, 2047, 2039, 2040, 2041, 2012, 2013, 2014 };
int cprarich02[] = { 2031, 2032, 2033, 2034, 2035, 2036, 2037, 2038 };
int cprarich03[] = { 2015, 9002, 9003, 9004, 9005, 9006, 9007 };
int cprarich04[] = { 2016, 2017, 2018, 2019, 2021, 2022, 2023 };
int cprarich05[] = { 2024, 2025, 2026, 2027, 2028, 2029, 2030 };
int cprarich07[] = { 2046, 2047, 2050, 2051, 2052 };
int cprarich01[] = { 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 };
int cprarich02[] = { 2031, 2032, 2033, 2034, 2035, 2036, 2037, 2038 };
int cprarich03[] = { 2015, 9002, 9003, 9004, 9005, 9006, 9007 };
int cprarich04[] = { 2016, 2017, 2018, 2019, 2021, 2022, 2023 };
int cprarich05[] = { 2024, 2025, 2026, 2027, 2028, 2029, 2030 };
int cprarich07[] = { 2046, 2047, 2050, 2051, 2052 };
*/
std::vector<int> cprarich01;
std::vector<int> cprarich02;
std::vector<int> cprarich03;
std::vector<int> cprarich04;
std::vector<int> cprarich05;
std::vector<int> cprarich06;
std::vector<int> cprarich07;
std::vector<int> cprarich08;
std::vector<int> cprarich09;
std::vector<int> cprarich00;

std::vector<int>& getcopper(const char* hostname, int& ncpr, std::string& ropcname)
{
  ropcname = Belle2::StringUtil::replace(hostname, "arich0", "ropc40");
  if (strcmp(hostname, "arich01") == 0) {
    ncpr = cprarich01.size();
    return cprarich01;
  } else if (strcmp(hostname, "arich02") == 0) {
    ncpr = cprarich02.size();
    return cprarich02;
  } else if (strcmp(hostname, "arich03") == 0) {
    ncpr = cprarich03.size();
    return cprarich03;
  } else if (strcmp(hostname, "arich04") == 0) {
    ncpr = cprarich04.size();
    return cprarich04;
  } else if (strcmp(hostname, "arich05") == 0) {
    ncpr = cprarich05.size();
    return cprarich05;
  } else if (strcmp(hostname, "arich06") == 0) {
    ncpr = cprarich06.size();
    return cprarich06;
  } else if (strcmp(hostname, "arich07") == 0) {
    ncpr = cprarich07.size();
    return cprarich07;
  } else if (strcmp(hostname, "arich08") == 0) {
    ncpr = cprarich08.size();
    return cprarich08;
  } else if (strcmp(hostname, "arich09") == 0) {
    ncpr = cprarich09.size();
    return cprarich09;
  }
  ncpr = 0;
  return cprarich00;
}

int main(int argc, char** argv)
{
  /*
  for (int i = 2001; i <= 2009; i++) cprarich01.push_back(i);
  for (int i = 2010; i <= 2017; i++) cprarich02.push_back(i);
  for (int i = 2018; i <= 2025; i++) cprarich03.push_back(i);
  for (int i = 2026; i <= 2034; i++) cprarich04.push_back(i);
  for (int i = 2035; i <= 2042; i++) cprarich05.push_back(i);
  for (int i = 2043; i <= 2050; i++) cprarich06.push_back(i);
  for (int i = 2051; i <= 2059; i++) cprarich07.push_back(i);
  for (int i = 2060; i <= 2067; i++) cprarich08.push_back(i);
  for (int i = 2068; i <= 2075; i++) cprarich09.push_back(i);
  */
  for (int i = 4001; i <= 4003; i++) cprarich01.push_back(i);
  for (int i = 4004; i <= 4006; i++) cprarich02.push_back(i);
  for (int i = 4007; i <= 4009; i++) cprarich03.push_back(i);
  for (int i = 4010; i <= 4012; i++) cprarich04.push_back(i);
  for (int i = 4013; i <= 4015; i++) cprarich05.push_back(i);
  for (int i = 4016; i <= 4018; i++) cprarich06.push_back(i);

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
  std::vector<int>& cprarich(getcopper(hostname, ncpr, ropcname));
  char s[256];
  if (nsm) {
    sprintf(s, "killall nsmd2 runcontrold rocontrold");
    printf("%s\n", s);
    system(s);
    for (int i = 0; i < ncpr; i++) {
      sprintf(s, "ssh cpr%d \"killall nsmd2 cprcontrold\"", cprarich[i]);
      printf("%s\n", s);
      system(s);
    }
    usleep(100000);
    sprintf(s, "bootnsmd2; bootnsmd2 -g");
    printf("%s\n", s);
    system(s);
    for (int i = 0; i < ncpr; i++) {
      sprintf(s, "ssh cpr%d \"source ~/.bash_profile; bootnsmd2 cpr%d\"",
              cprarich[i], cprarich[i]);
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
      sprintf(s, "ssh cpr%d \"killall -9 cprcontrold basf2 des_ser_COPPER_main;\" ", cprarich[i]);
      printf("%s\n", s);
      system(s);
    }
    usleep(100000);
    for (int i = 0; i < ncpr; i++) {
      sprintf(s, "ssh cpr%d \"source ~/.bash_profile; cprcontrold cpr%d -d\"", cprarich[i], cprarich[i]);
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
