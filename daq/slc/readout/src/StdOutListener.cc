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
#include <stdio.h>
#include <ctype.h>

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
  SystemLog::Priority priority = SystemLog::UNKNOWN;
  try {
    while (true) {
      while (true) {
        c = preader.readChar();
        if (c == '\n') {
          s = ss.str();
          ss.str("");
          if (priority > 0) {
            _con->lock();
            _con->getLog().send(priority, s);
            _con->unlock();
          }
        } else if (isprint(c)) {
          if (priority > 0 && c == '[') {
            while (true) {
              c = preader.readChar();
              if (c == ']') {
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
            ss << c;
          }
        }
      }
    }
  } catch (const IOException& e) {
    Belle2::debug("%s:%d %s", __FILE__, __LINE__, e.what());
  }
  close(_pipe[0]);
}
