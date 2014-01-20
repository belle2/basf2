#include <eutel/eudaq/Logger.h>

namespace eudaq {
  LogSender& GetLogger()
  {
    static LogSender logger;
    return logger;
  }
}
