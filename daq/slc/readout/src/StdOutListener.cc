#include "daq/slc/readout/StdOutListener.h"

#include "daq/slc/readout/ProcessController.h"

#include "daq/slc/nsm/RCCallback.h"
#include "daq/slc/nsm/NSMCommunicator.h"

#include "daq/slc/system/File.h"
#include "daq/slc/system/FileReader.h"

#include "daq/slc/base/Debugger.h"
#include "daq/slc/base/StringUtil.h"

#include <iostream>
#include <sstream>
#include <cerrno>

namespace Belle2 {

  typedef FileReader<File> PipeReader;

}

using namespace Belle2;

void StdOutListener::run()
{
  File fd(_pipe[0]);
  PipeReader preader(fd);
  char c;
  std::stringstream ss;
  std::string s;
  int priority = -1;
  try {
    while (true) {
      while (true) {
        c = preader.readChar();
        std::cerr << c;
        /*
          if (c == '[') {
          if (priority > 0) {
          s = ss.str();
          ss.str("");
          _con->lock();
          _con->getLog().send(priority, s);
          _con->unlock();
        }
        while (true) {
          c = preader.readChar();
          std::cerr << c;
          if (c == ']') {
            std::cerr << std::endl;
            s = ss.str();
            ss.str("");
            if (s == "DEBUG") priority = SystemLog::DEBUG;
            else if (s == "INFO") priority = SystemLog::INFO;
            else if (s == "ERROR") priority = SystemLog::ERROR;
            else if (s == "FATAL") priority = SystemLog::FATAL;
            break;
          }
          ss << c;
        }
            } else {
        while (preader.readChar() != '\n') {
        }
            }
            ss << c;
            */
      }
    }
  } catch (const IOException& e) {
    Belle2::debug("%s:%d %s", __FILE__, __LINE__, e.what());
  }
  close(_pipe[0]);
}
