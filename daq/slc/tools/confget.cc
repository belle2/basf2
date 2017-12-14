#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>

using namespace Belle2;

int help(const char** argv)
{
  printf("usage  : %s <parname1> [<parname2>...] [-c conf]\n", argv[0]);
  printf("options: -c : set conf file \"conf\" (default:slowcontrol)\n");
  printf("options: -v : set verbose mode (default:no)\n");
  return 0;
}

int nsm_read_argv(int argc, const char** argv, char** argv_in,
                  ConfigFile& config, bool& verbose, int nargv)
{
  int argc_in = 0;
  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0) {
      return help(argv);
    } else if (strcmp(argv[i], "-v") == 0) {
      verbose = true;
    } else if (strcmp(argv[i], "-c") == 0) {
      if (i + 1 < argc && argv[i + 1][0] != '-') {
        i++;
        config.read(argv[i]);
      }
    } else {
      argv_in[argc_in] = new char[100];
      strcpy(argv_in[argc_in], argv[i]);
      argc_in++;
    }
  }
  if (argc_in < nargv) {
    return help(argv);
  }
  return argc_in;
}

int main(int argc, const char** argv)
{
  ConfigFile config("slowcontrol");
  bool verbose = false;
  char** argv_in = new char* [argc];
  int argc_in = nsm_read_argv(argc, argv, argv_in,
                              config, verbose, 2);
  if (argc_in == 0) return 0;
  if (!verbose) {
    for (int i = 1; i < argc_in; i++) {
      std::cout << config.get(argv_in[i]) << " ";
    }
    std::cout << std::endl;
  } else {
    size_t length = 0;
    for (int i = 1; i < argc_in; i++) {
      if (strlen(argv_in[i]) > length) length = strlen(argv_in[i]);
    }
    for (int i = 1; i < argc_in; i++) {
      printf(StringUtil::form("%%-%ds : %%s\n", length).c_str(),
             argv_in[i], config.get(argv_in[i]).c_str());
    }
  }
  return 0;
}
