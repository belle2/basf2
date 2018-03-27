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
  if (strcmp(hostname, "svd11") == 0) {
    for (int i = 1049; i <= 1053; i++) cpr.push_back(i);
    ropcname = "ropc111";
  } else if (strcmp(hostname, "cdc01") == 0) {
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
    for (int i = 3006; i <= 3010; i++) cpr.push_back(i);
    ropcname = "ropc302";
  } else if (strcmp(hostname, "top03") == 0) {
    for (int i = 3011; i <= 3016; i++) cpr.push_back(i);
    ropcname = "ropc303";
  } else if (strcmp(hostname, "arich01") == 0) {
    for (int i = 4001; i <= 4003; i++) cpr.push_back(i);
    ropcname = "ropc401";
  } else if (strcmp(hostname, "arich02") == 0) {
    for (int i = 4004; i <= 4006; i++) cpr.push_back(i);
    ropcname = "ropc402";
  } else if (strcmp(hostname, "arich03") == 0) {
    for (int i = 4007; i <= 4009; i++) cpr.push_back(i);
    ropcname = "ropc403";
  } else if (strcmp(hostname, "arich04") == 0) {
    for (int i = 4010; i <= 4012; i++) cpr.push_back(i);
    ropcname = "ropc404";
  } else if (strcmp(hostname, "arich05") == 0) {
    for (int i = 4013; i <= 4015; i++) cpr.push_back(i);
    ropcname = "ropc405";
  } else if (strcmp(hostname, "arich06") == 0) {
    for (int i = 4016; i <= 4018; i++) cpr.push_back(i);
    ropcname = "ropc406";
  } else if (strcmp(hostname, "arich07") == 0) {
    ropcname = "ropc407";
  } else if (strcmp(hostname, "arich08") == 0) {
    ropcname = "ropc408";
  } else if (strcmp(hostname, "arich09") == 0) {
    ropcname = "ropc409";
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
    for (int i = 6007; i <= 6008; i++) cpr.push_back(i);
    cpr.push_back(13001);
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
    cpr.push_back(11002);
    cpr.push_back(11003);
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
    for (int i = 0; i < ncpr; i++) {
      sprintf(s, "ssh cpr%d \"booths -abcd /usr/local/firmware/hslb065_xtal.bit\"",
              cprs[i]);
      printf("%s\n", s);
      system(s);
    }
  }
  if (cpr) {
    for (int i = 0; i < ncpr; i++) {
      sprintf(s, "ssh cpr%d \"tesths -abcd \"",
              cprs[i]);
      sprintf(s, "ssh cpr%d \"staths; tesths -abcd \"",
              cprs[i]);
      printf("%s\n", s);
      system(s);
    }
  }
  return 0;
}
