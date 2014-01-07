#include <daq/slc/nsm/NSMCommunicator.h>
#include <daq/slc/base/Debugger.h>

#include <nsm2/nsm2.h>
#include <nsm2/belle2nsm.h>

#include <unistd.h>
#include <cstring>

using namespace Belle2;

int main(int argc, char** argv)
{
#if NSM_PACKAGE_VERSION >= 1914
  if (argc < 3) {
    printf("Usage : ./logsender2 <name> <priority> <message>\n");
    return 1;
  }
  const char* nodename = argv[1];
  const char* priority_s = argv[2];
  const char* message = argv[3];
  NSMcontext* nsmc = NULL;
  if ((nsmc = b2nsm_init(nodename)) == 0) {
    printf("[FATAL] Failed to connect NSM : %s", b2nsm_strerror());
    return 1;
  }
  NSMCommunicator* comm = new NSMCommunicator();
  comm->setContext(nsmc);
  SystemLog::Priority priority = SystemLog::DEBUG;
  if (strcmp(priority_s, "INFO") == 0) {
    priority = SystemLog::INFO;
  } else if (strcmp(priority_s, "NOTICE") == 0) {
    priority = SystemLog::NOTICE;
  } else if (strcmp(priority_s, "WARNING") == 0) {
    priority = SystemLog::WARNING;
  } else if (strcmp(priority_s, "ERROR") == 0) {
    priority = SystemLog::ERROR;
  } else if (strcmp(priority_s, "FATAL") == 0) {
    priority = SystemLog::FATAL;
  }
  comm->sendLog(SystemLog(nodename, priority, message));
#else
#warning "Wrong version of nsm2. try source daq/slc/extra/nsm2/export.sh"
#endif
  return 0;
}
