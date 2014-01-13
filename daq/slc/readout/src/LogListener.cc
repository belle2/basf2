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
    int count = 0;
    while (true) {
      c = preader.readChar();
      if (c == '\n') {
        s = ss.str();
        ss.str("");
        if (priority > 0) {
          _con->lock();
          LogFile::put(priority, s);
          if (priority == SystemLog::ERROR) {
            comm->sendError(s);
            _con->setState(RunInfoBuffer::ERROR);
          } else if (priority == SystemLog::FATAL) {
            comm->sendFatal(s);
            _con->setState(RunInfoBuffer::ERROR);
          }
          if (priority >= SystemLog::NOTICE) {
            comm->sendLog(SystemLog(node->getName(), priority, s));
          }
          _con->unlock();
          priority = SystemLog::UNKNOWN;
        }
      } else if (isprint(c)) {
        if (priority == 0 && c == '[') {
          count = 0;
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
    //LogFile::debug("failed to read pipe: %s", e.what());
  }
  close(_pipe[0]);
  std::string process_name = _con->getName();
  unsigned int state = _con->getInfo().getState();
  std::string emsg;
  switch (state) {
    case RunInfoBuffer::ERROR:
    case RunInfoBuffer::RUNNING:
      emsg = Belle2::form("Forked process %s was crashed",  process_name.c_str());
      LogFile::error(emsg);
      _con->setState(State::ERROR_ES);
      comm->sendError(emsg);
      break;
    case RunInfoBuffer::READY:
      emsg = Belle2::form("Forked process %s was not started", process_name.c_str());
      LogFile::warning(emsg);
      comm->sendError(emsg);
      break;
    case RunInfoBuffer::NOTREADY:
    default:
      emsg = Belle2::form("Forked process %s was finished", process_name.c_str());
      LogFile::debug(emsg);
      comm->sendLog(SystemLog(node->getName(), SystemLog::INFO, emsg));
      break;
  }
  Fork forkid = _con->getFork();
  if (forkid.wait(WNOHANG) < 0) {
    LogFile::debug("Failed to wait forked process %s", process_name.c_str());
    return;
  }
}
