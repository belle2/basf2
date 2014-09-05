#include "daq/slc/readout/LogListener.h"

#include "daq/slc/readout/ProcessController.h"

#include <daq/slc/runcontrol/RCCallback.h>
#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/File.h>
#include <daq/slc/system/FileReader.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/system/LogFile.h>

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
  File fd(m_pipe[0]);
  PipeReader preader(fd);
  char c;
  std::stringstream ss;
  std::string s;
  LogFile::Priority priority = LogFile::UNKNOWN;
  NSMCommunicator* comm = m_con->getCallback()->getCommunicator();
  NSMNode& node(m_con->getCallback()->getNode());
  try {
    int count = 0;
    while (true) {
      c = preader.readChar();
      if (c == '\n' && count > 0) {
        s = m_con->getName() + " : " + ss.str();
        ss.str("");
        m_con->lock();
        LogFile::put(priority, s);
        if (node.getState() == RCState::RUNNING_S ||
            node.getState() == RCState::STARTING_TS) {
          node.setState(RCState::RECOVERING_RS);
        }
        if (priority >= LogFile::NOTICE) {
          comm->sendLog(DAQLogMessage(node.getName(), priority, s));
        }
        m_con->unlock();
        count = 0;
        priority = LogFile::UNKNOWN;
      } else if (isprint(c)) {
        if (count == 0 && c == '[') {
          ss << c;
          while (true) {
            c = preader.readChar();
            if (c == ']') {
              ss << c;
              s = ss.str();
              if (s == "[DEBUG]") priority = LogFile::DEBUG;
              else if (s == "[INFO]") priority = LogFile::INFO;
              else if (s == "[NOTICE]") priority = LogFile::NOTICE;
              else if (s == "[WARNING]") priority = LogFile::WARNING;
              else if (s == "[ERROR]") priority = LogFile::ERROR;
              else if (s == "[FATAL]") priority = LogFile::FATAL;
              if (priority > 0) {
                count = 0;
                ss.str("");
              } else {
                priority = LogFile::DEBUG;
              }
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
  }
  close(m_pipe[0]);
}
