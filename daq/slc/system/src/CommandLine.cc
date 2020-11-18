//+
// File : CommandLine.cc
// Description : Run external shell
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 25 - Mar - 2020
//-

#include "daq/slc/system/CommandLine.h"

using namespace Belle2;
using namespace std;

int CommandLine::run_shell(const string& cmdline, string& response)
{
  FILE* fp = ::popen(cmdline.c_str(), "r");
  if (fp == NULL) {
    fprintf(stderr, "run_shell: Error to run command : %s\n", cmdline.c_str());
    return -1;
  }
  char buf[4096];
  fgets(buf, 4096, fp);
  buf[strlen(buf) - 1] = '\0';
  response = std::string(buf);
  fclose(fp);
  return 0;
}
