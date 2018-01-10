#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <unistd.h>

#include <daq/slc/base/StringUtil.h>

std::vector<int> getcopper(const char* hostname, int& ncpr, std::string& ropcname)
{
  std::vector<int> cpr;
  if (strcmp(hostname, "cdc01") == 0) {
    for (int i = 2001; i <= 2009; i++) cpr.push_back(i);
    ropcname = "ropc201";
  } else if (strcmp(hostname, "cdc02") == 0) {
    for (int i = 2010; i <= 2017; i++) cpr.push_back(i);
    ropcname = "ropc202";
  } else if (strcmp(hostname, "cdc03") == 0) {
    for (int i = 2018; i <= 2025; i++) cpr.push_back(i);
    ropcname = "ropc203";
  } else if (strcmp(hostname, "cdc04") == 0) {
    for (int i = 2026; i <= 2034; i++) cpr.push_back(i);
    ropcname = "ropc204";
  } else if (strcmp(hostname, "cdc05") == 0) {
    for (int i = 2035; i <= 2042; i++) cpr.push_back(i);
    ropcname = "ropc205";
  } else if (strcmp(hostname, "cdc06") == 0) {
    for (int i = 2043; i <= 2050; i++) cpr.push_back(i);
    ropcname = "ropc206";
  } else if (strcmp(hostname, "cdc07") == 0) {
    for (int i = 2051; i <= 2059; i++) cpr.push_back(i);
    ropcname = "ropc207";
  } else if (strcmp(hostname, "cdc08") == 0) {
    for (int i = 2060; i <= 2067; i++) cpr.push_back(i);
    ropcname = "ropc208";
  } else if (strcmp(hostname, "cdc09") == 0) {
    for (int i = 2068; i <= 2075; i++) cpr.push_back(i);
    ropcname = "ropc209";
  } else if (strcmp(hostname, "top01") == 0) {
    for (int i = 3001; i <= 3005; i++) cpr.push_back(i);
    ropcname = "ropc301";
  } else if (strcmp(hostname, "top02") == 0) {
    for (int i = 3006; i <= 3008; i++) cpr.push_back(i);
    ropcname = "ropc302";
  } else if (strcmp(hostname, "top03") == 0) {
    for (int i = 3009; i <= 3010; i++) cpr.push_back(i);
    ropcname = "ropc303";
  } else if (strcmp(hostname, "ecl01") == 0) {
    for (int i = 5001; i <= 5002; i++) cpr.push_back(i);
    ropcname = "ropc501";
  } else if (strcmp(hostname, "ecl02") == 0) {
    for (int i = 5003; i <= 5004; i++) cpr.push_back(i);
    ropcname = "ropc502";
  } else if (strcmp(hostname, "ecl03") == 0) {
    for (int i = 5005; i <= 5007; i++) cpr.push_back(i);
    ropcname = "ropc503";
  } else if (strcmp(hostname, "ecl04") == 0) {
    for (int i = 5008; i <= 5010; i++) cpr.push_back(i);
    ropcname = "ropc504";
  } else if (strcmp(hostname, "ecl05") == 0) {
    for (int i = 5011; i <= 5013; i++) cpr.push_back(i);
    ropcname = "ropc505";
  } else if (strcmp(hostname, "ecl06") == 0) {
    for (int i = 5014; i <= 5015; i++) cpr.push_back(i);
    ropcname = "ropc506";
  } else if (strcmp(hostname, "ecl07") == 0) {
    for (int i = 5016; i <= 5018; i++) cpr.push_back(i);
    ropcname = "ropc507";
  } else if (strcmp(hostname, "ecl08") == 0) {
    for (int i = 6001; i <= 6003; i++) cpr.push_back(i);
    ropcname = "ropc508";
  } else if (strcmp(hostname, "ecl09") == 0) {
    for (int i = 6004; i <= 6006; i++) cpr.push_back(i);
    ropcname = "ropc509";
  } else if (strcmp(hostname, "ecl10") == 0) {
    for (int i = 6007; i <= 6086; i++) cpr.push_back(i);
    cpr.push_back(9001);
    ropcname = "ropc510";
  } else if (strcmp(hostname, "klm01") == 0) {
    for (int i = 7001; i <= 7002; i++) cpr.push_back(i);
    ropcname = "ropc701";
  } else if (strcmp(hostname, "klm02") == 0) {
    for (int i = 7003; i <= 7004; i++) cpr.push_back(i);
    ropcname = "ropc702";
  } else if (strcmp(hostname, "klm03") == 0) {
    for (int i = 8001; i <= 8004; i++) cpr.push_back(i);
    ropcname = "ropc703";
  } else if (strcmp(hostname, "trg01") == 0) {
    cpr.push_back(11001);
    cpr.push_back(15001);
    ropcname = "ropc901";
  }
  ncpr = cpr.size();
  return cpr;
}

int main(int argc, char** argv)
{

  bool nsm = false;
  bool cpr = false;
  bool print = false;
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
    } else if (strcmp(argv[i], "-p") == 0) {
      print = true;
      continue;
    }
  }

  const char* hostname = getenv("HOSTNAME");
  std::string ropcname;
  int ncpr = 0;
  std::vector<int> cprs(getcopper(hostname, ncpr, ropcname));
  char s[256];
  if (print) {
    for (int i = 0; i < ncpr; i++) {
      printf("cpr%d \n", cprs[i]);
    }
    return 0;
  }
  if (nsm) {
    sprintf(s, "killall nsmd2 runcontrold rocontrold");
    printf("%s\n", s);
    system(s);
    for (int i = 0; i < ncpr; i++) {
      sprintf(s, "ssh cpr%d \"killall nsmd2 cprcontrold\"", cprs[i]);
      printf("%s\n", s);
      system(s);
    }
    usleep(100000);
    sprintf(s, "bootnsmd2; bootnsmd2 -g");
    printf("%s\n", s);
    system(s);
    for (int i = 0; i < ncpr; i++) {
      sprintf(s, "ssh cpr%d \"source ~/.bash_profile; bootnsmd2 cpr%d\"",
              cprs[i], cprs[i]);
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
      sprintf(s, "ssh cpr%d \"killall -9 cprcontrold basf2 des_ser_COPPER_main;\" ", cprs[i]);
      printf("%s\n", s);
      system(s);
    }
    usleep(100000);
    for (int i = 0; i < ncpr; i++) {
      sprintf(s, "ssh cpr%d \"source ~/.bash_profile; cprcontrold cpr%d -d\"", cprs[i], cprs[i]);
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
