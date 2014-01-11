#include "daq/slc/readout/LogListener.h"

#include "daq/slc/readout/ProcessController.h"

#include "daq/slc/nsm/RCCallback.h"
#include "daq/slc/nsm/NSMCommunicator.h"

#include "daq/slc/system/File.h"
#include "daq/slc/system/FileReader.h"

#include "daq/slc/base/Debugger.h"
#include "daq/slc/base/StringUtil.h"
#include "daq/slc/system/LogFile.h"

#include <iostream>
#include <sstream>
#include <cerrno>
#include <stdio.h>
#include <ctype.h>

namespace Belle2 {

  typedef FileReader<File> PipeReader;

}

using namespace Belle2;

void LogListener::run()
{
  File fd(_pipe[0]);
  PipeReader preader(fd);
  char c;
  std::stringstream ss;
  std::string s;
  SystemLog::Priority priority = SystemLog::UNKNOWN;
  NSMCommunicator* comm = _con->getCallback()->getCommunicator();
  NSMNode* node = _con->getCallback()->getNode();
  try {
    while (true) {
      int count = 0;
      c = preader.readChar();
      if (c == '\n') {
        s = ss.str();
        ss.str("");
        if (priority > 0) {
          _con->lock();
          if (priority == SystemLog::ERROR) {
            comm->sendError(s);
            _con->setState(RunInfoBuffer::ERROR);
          } else if (priority == SystemLog::FATAL) {
            comm->sendFatal(s);
            _con->setState(RunInfoBuffer::ERROR);
          }
          comm->sendLog(SystemLog(node->getName(), priority, s));
          LogFile::put(priority, s);
          sleep(1);
          _con->unlock();
          priority = SystemLog::UNKNOWN;
        }
      } else if (isprint(c)) {
        if (priority == 0 && c == '[') {
          while (true) {
            c = preader.readChar();
            if (c == ']') {
              s = ss.str();
              ss.str("");
              if (s == "DEBUG") priority = SystemLog::DEBUG;
              else if (s == "INFO") priority = SystemLog::INFO;
              else if (s == "NOTICE") priority = SystemLog::NOTICE;
              else if (s == "WARNING") priority = SystemLog::WARNING;
              else if (s == "ERROR") priority = SystemLog::ERROR;
              else if (s == "FATAL") priority = SystemLog::FATAL;
              break;
            }
            ss << c;
          }
        } else {
          if (c != ' ' || count > 0) {
            ss << c;
            count++;
          }
        }
      }
    }
  } catch (const IOException& e) {
    LogFile::error("Failed to read log: %s", e.what());
  }
  close(_pipe[0]);
}
