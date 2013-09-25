#include "COPPERCallback.hh"

#include <nsm/NSMData.hh>
#include <nsm/NSMNodeDaemon.hh>

#include <node/COPPERNode.hh>

#include <util/Debugger.hh>
#include <util/StringUtil.hh>

#include <dlfcn.h>

using namespace B2DAQ;

typedef void* func_t(void*, const char*);

int main(int argc, char** argv)
{
  if (argc < 4) {
    B2DAQ::debug("Usage : ./copperd <name> <class> <path>");
    return 1;
  }
  const char* name = argv[1];
  const char* class_name = argv[2];
  const char* path = argv[3];
  void* handle = dlopen(B2DAQ::form("%s/lib/libB2DAQ_%s.so",
                                    path, class_name).c_str(),
                        RTLD_NOW | RTLD_GLOBAL);
  if (!handle) {
    B2DAQ::debug("%s", dlerror());
    return 1;
  }

  char* error = NULL;
  func_t* createCOPPERData = (func_t*)dlsym(handle, B2DAQ::form("create%sData", class_name).c_str());
  if ((error = dlerror()) != NULL) {
    B2DAQ::debug("%s", error);
    return 1;
  }

  COPPERNode* node = new COPPERNode(name);
  NSMData* data = (NSMData*)createCOPPERData((void*)node,
                                             B2DAQ::form("%s_DATA", name).c_str());
  if (data == NULL) {
    B2DAQ::debug("System error: No COPPER node is ready.");
    return 1;
  }
  COPPERCallback* callback = new COPPERCallback(node, data);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(node, callback, data);
  daemon->run();

  dlclose(handle);
  return 0;
}
