#include "HistoServer.hh"
#include "DefaultHistoManager.hh"
#include "SocketAcceptor.hh"

#include <dqm/HistoManager.hh>

#include <system/PThread.hh>

#include <util/StringUtil.hh>
#include <util/Debugger.hh>

#include <fstream>
#include <iostream>
#include <cstdlib>

#include <dlfcn.h>

using namespace B2DQM;

typedef void* func_t();

struct DQM_input {
  std::string filename;
  std::string name;
  bool use_so_file;
};

int main(int argc, char** argv)
{
  if (argc < 1) {
    std::cout << "Usage: ./dqmserver"
              << std::endl;
    return 1;
  }

  std::vector<DQM_input> input_v;
  std::ifstream fin(argv[1]);
  std::string buf;
  while (fin && getline(fin, buf)) {
    if (buf.size() == 0 || buf.at(0) == '#') continue;
    std::vector<std::string> str_v = B2DAQ::split(buf, ':');
    if (str_v.size() == 3) {
      DQM_input input = {str_v[0], str_v[1], str_v[2] == "use_so_file"};
      input_v.push_back(input);
    } else if (str_v.size() == 2) {
      DQM_input input = {str_v[0], str_v[1], false};
      input_v.push_back(input);
    }
  }

  const size_t ninputs = input_v.size();
  HistoServer* server = new HistoServer();
  server->setDirectory(getenv("B2SC_DQM_MAP_PATH"));
  for (size_t n = 0; n < ninputs; n++) {
    DQM_input& input(input_v[n]);
    if (input.use_so_file) {
      void* handle = dlopen(B2DAQ::form("%s/libB2DQM_%s.so",
                                        getenv("B2SC_DQM_LIB_PATH"),
                                        input.name.c_str()).c_str(),
                            RTLD_NOW | RTLD_GLOBAL);
      if (!handle) {
        B2DAQ::debug("%s", dlerror());
        return 1;
      }
      char* error = NULL;
      func_t* createHistoManager =
        (func_t*)dlsym(handle, B2DAQ::form("create%sHistoManager",
                                           input.name.c_str()).c_str());
      if ((error = dlerror()) != NULL) {
        B2DAQ::debug("%s", error);
        return 1;
      }
      server->addManager(input.filename, (HistoManager*)createHistoManager());
    } else {
      server->addManager(input.filename, new DefaultHistoManager(input.name));
    }
  }
  B2DAQ::PThread(new SocketAcceptor(getenv("B2SC_SERVER_HOST"), server));
  server->run();
  return 0;
}

