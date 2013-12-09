#include "SenderManager.h"

#include "daq/slc/base/Debugger.h"
#include "daq/slc/base/StringUtil.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

using namespace Belle2;

void SenderManager::run() throw()
{
  if (_node != NULL) {
    const char* belle2_path = getenv("BELLE2_LOCAL_DIR");
    const char* belle2_sub = getenv("BELLE2_SUBDIR");
    if (belle2_path == NULL) {
      Belle2::debug("[DEBUG] env BELLE2_LOCAL_DIR is not avaiable.");
      return;
    }
    DataObject* obj = _node->getData();
    int flag = 0;
    for (size_t slot = 0; slot < 4; slot++) {
      if (obj->getBool(Belle2::form("used_%d", slot)))
        flag += 1 << slot;
    }
    char path[64];
    char* argv[20];
    int i = 0;
    sprintf(path, "%s/bin/%s/basf2", belle2_path, belle2_sub);
    argv[i] = (char*)malloc(sizeof(char) * 64);
    sprintf(argv[i++], "%s/daq/rawdata/examples/%s",
            belle2_path, obj->getText("script").c_str());
    argv[i] = (char*)malloc(sizeof(char) * 64);
    sprintf(argv[i++], "%s", obj->getText("host").c_str());
    argv[i] = (char*)malloc(sizeof(char) * 64);
    sprintf(argv[i++], "%d", (int)_node->getID());
    argv[i] = (char*)malloc(sizeof(char) * 64);
    sprintf(argv[i++], "%d", flag);
    argv[i] = (char*)malloc(sizeof(char) * 64);
    sprintf(argv[i++], "%d", 1);
    argv[i] = (char*)malloc(sizeof(char) * 64);
    sprintf(argv[i++], "--no-stats");
    argv[i] = NULL;
    i = 0;
    printf("[BUDEG] Executing : ");
    while (argv[i] != NULL) {
      printf("%s ", argv[i++]);
    }
    printf("\n");
    if (execvp(path, argv) == -1) {
      Belle2::debug("[ERROR] Faield to start receiver basf2 script");
    }
    i = 0;
    while (argv[i] != NULL) {
      free(argv[i++]);
    }
  }
}

