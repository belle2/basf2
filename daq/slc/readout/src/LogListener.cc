#include "daq/slc/readout/LogListener.h"

#include "daq/slc/readout/ProcessController.h"

#include <daq/slc/runcontrol/RCCallback.h>
#include <daq/slc/runcontrol/RCCommand.h>
#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/File.h>
#include <daq/slc/system/FileReader.h>

#include <daq/slc/base/IOException.h>
#include <daq/slc/base/StringUtil.h>
#include <daq/slc/system/LogFile.h>

#include <iostream>
#include <sstream>
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
  int count = 0;
  try {
    while (true) {
      c = preader.readChar(); //read next character from pipe

      if (c != '\n' && iscntrl(c)) continue; //character is unprintable, skip

      if (c == '\n' && count > 0) { //newline received line was not empty -> message is assembled, submit
        std::string assembledLogMessage(ss.str());

        // basf2 has a flag to escape newlines with "\\n" to write multi-line log messages into a single line
        // see: https://agira.desy.de/browse/BII-6470
        // if m_enableUnescapeNewlines is set, these "\\n" will be replaced with "\n" newlines again before the log message is sent out

        if (m_enableUnescapeNewlines) {
          assembledLogMessage = StringUtil::replace(assembledLogMessage, "\\n", "\n");
        }

        s = m_con->getParName() + " : " + assembledLogMessage;
        ss.str("");
        //m_con->lock();
        if (priority == LogFile::UNKNOWN) {
          priority = LogFile::DEBUG;
        }

        if (priority > LogFile::DEBUG) {
          m_con->getCallback()->log(priority, s);
        } else {
          LogFile::debug(s);
        }

        if (m_con->getCallback()->getNode().getState() == RCState::RUNNING_S) {
          if (priority == LogFile::ERROR) {
            // m_con->getCallback()->log(LogFile::ERROR, s);
          } else if (priority == LogFile::FATAL) {
            // m_con->getCallback()->log(LogFile::FATAL, s));
            m_con->getCallback()->setState(RCState::ERROR_ES);
          }
        }
        //m_con->unlock();
        count = 0;
        priority = LogFile::UNKNOWN;
      } else if (isprint(c)) { //continue to assemble message
        if (count == 0 && c == '[') { //start of a "[DEBUG]"-like priority identifier preceding each log line
          ss << c;
          while (true) {
            c = preader.readChar();
            if (c == ']') { //end of a "[DEBUG]"-like priority identifier
              ss << c;
              s = ss.str();
              if (s == "[DEBUG]") priority = LogFile::DEBUG;
              else if (s == "[INFO]") priority = LogFile::INFO;
              else if (s == "[NOTICE]") priority = LogFile::NOTICE;
              else if (s == "[WARNING]") priority = LogFile::WARNING;
              else if (s == "[ERROR]") priority = LogFile::ERROR;
              else if (s == "[FATAL]") priority = LogFile::FATAL;
              else if (s.find("STOP") != std::string::npos) {
                StringList sl = StringUtil::split(s, '=');
                if (sl.size() > 1) {
                  std::string nodename = StringUtil::replace(sl[1], "]", "");
                  try {
                    NSMCommunicator::send(NSMMessage(NSMNode(nodename), RCCommand::STOP));
                  } catch (const std::exception& e) {
                  }
                }
                count = 0;
                ss.str("");
                break;
              }
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
    LogFile::debug(e.what());
    if (count > 0) {
      s = m_con->getParName() + " : " + ss.str();
      ss.str("");
      //m_con->lock();
      if (priority == LogFile::UNKNOWN) {
        priority = LogFile::DEBUG;
      }
      m_con->getCallback()->log(priority, s);
      if (m_con->getCallback()->getNode().getState() == RCState::RUNNING_S) {
        if (priority == LogFile::ERROR) {
          //m_con->getCallback()->reply(NSMMessage(NSMCommand::ERROR, s));
        } else if (priority == LogFile::FATAL) {
          //m_con->getCallback()->reply(NSMMessage(NSMCommand::ERROR, s));
          m_con->getCallback()->setState(RCState::ERROR_ES);
        }
      }
      //m_con->unlock();
    }
  }
  close(m_pipe[0]);
}
