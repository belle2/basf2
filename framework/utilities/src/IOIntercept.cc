/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/utilities/IOIntercept.h>
#include <framework/logging/Logger.h>
#include <memory>
#include <cstring>
#include <csignal>
#include <set>
#include <unistd.h>
#include <fcntl.h>
#include <boost/algorithm/string.hpp>

namespace Belle2::IOIntercept {
  /** Small class to handle std::abort() calls by external libraries
   *
   * External libraries like EvtGen seem to prefer calling abort() on error
   * which means destructors are not called and so interception is never
   * finished and thus the error message are lost.
   *
   * This class installs a SIGABRT handler which is the only way to do
   * something once std::abort() is called and dump all the output
   * intercepted so far back on the original output (stdout or stderr) to
   * basically "reverse" the redirection.
   *
   * This seems like the best choice since calling abort means the user needs
   * to be able to see the messages
   */
  class CaptureStreamAbortHandler {
  public:
    /** Singleton, no copy construction */
    CaptureStreamAbortHandler(const CaptureStreamAbortHandler&) = delete;
    /** Singleton, no move construction */
    CaptureStreamAbortHandler(CaptureStreamAbortHandler&&) = delete;
    /** Singleton, no assignment */
    CaptureStreamAbortHandler& operator=(const CaptureStreamAbortHandler&) = delete;
    /** Return the singleton instance */
    static CaptureStreamAbortHandler& getInstance()
    {
      static CaptureStreamAbortHandler instance;
      return instance;
    }
    /** Add a CaptureStream object to guard against SIGABRT */
    void addObject(CaptureStream* obj) { m_objects.emplace(obj); }
    /** Remove a CaptureStream object to no longer guard against SIGABRT */
    void removeObject(CaptureStream* obj) { m_objects.erase(obj); }
  private:
    /** Register handler */
    CaptureStreamAbortHandler()
    {
      auto result = std::signal(SIGABRT, &CaptureStreamAbortHandler::handle);
      if (result == SIG_ERR) B2FATAL("Cannot register abort handler");
    }
    /** signal handler: print all pending redirection buffers and exit */
    static void handle(int signal);
    /** list of all active stream redirections */
    std::set<CaptureStream*> m_objects;
  };

  void CaptureStreamAbortHandler::handle(int)
  {
    // move set of registered objects in here so we can call finish when
    // looping over them without invalidating the iterators
    std::set<CaptureStream*> objects;
    std::swap(CaptureStreamAbortHandler::getInstance().m_objects, objects);
    // loop over all registered instances and print the content in their
    // buffer to the original file descriptor
    for (CaptureStream* stream : objects) {
      // close the write end of the pipe
      close(stream->m_replacementFD);
      // read all content
      if (stream->finish()) {
        const std::string& out = stream->getOutput();
        // and write it to the original file descriptor
        if (out.size()) write(stream->m_savedFD, out.c_str(), out.size());
      }
    }
    // write error message and signal error
    write(STDERR_FILENO, "abort() called, exiting\n", 24);
    std::_Exit(EXIT_FAILURE);
  }

  StreamInterceptor::StreamInterceptor(std::ostream& stream, FILE* fileObject):
    m_stream(stream), m_fileObject(fileObject), m_savedFD(dup(fileno(m_fileObject)))
  {
    if (m_savedFD < 0) {
      B2ERROR("Error duplicating file descriptor: " << std::strerror(errno));
    }
  }
  StreamInterceptor::~StreamInterceptor()
  {
    finish();
    if (m_savedFD >= 0) close(m_savedFD);
    if (m_replacementFD >= 0) close(m_replacementFD);
  }

  void StreamInterceptor::readFD(int fd, std::string& out)
  {
    out.clear();
    if (fd <= 0) return;
    // need a buffer to read
    static std::unique_ptr<char[]> buffer(new char[1024]);
    // so then, read everything
    while (true) {
      ssize_t size = read(fd, buffer.get(), 1024);
      if (size <= 0) {
        // in case we get interrupted by signal, try again
        if (size < 0 && errno == EINTR)  continue;
        break;
      }
      out.append(buffer.get(), static_cast<size_t>(size));
    }
  }

  bool StreamInterceptor::replaceFD(int fileDescriptor)
  {
    // obviously we don't want to replace invalid descriptors
    if (fileDescriptor < 0) return false;
    // flush existing stream
    m_stream << std::flush;
    std::fflush(m_fileObject);
    // and clear the bad bits we might have gotten when pipe capacity is reached
    m_stream.clear();
    // and then replace the file descriptor
    const int currentFD = fileno(m_fileObject);
    if (currentFD < 0) {
      B2ERROR("Error obtaining file descriptor: " << std::strerror(errno));
      return false;
    }
    while (dup2(fileDescriptor, currentFD) < 0) {
      if (errno != EINTR && errno != EBUSY) {
        B2ERROR("Error in dup2(), cannot replace file descriptor: " << std::strerror(errno));
        return false;
      }
      // interrupted or busy, let's try again
    }
    return true;
  }

  DiscardStream::DiscardStream(std::ostream& stream, FILE* fileObject): StreamInterceptor(stream, fileObject)
  {
    int devNull = open("/dev/null", O_WRONLY);
    if (devNull < 0) {
      // warning
      B2ERROR("Cannot open /dev/null: " << std::strerror(errno));
      return;
    }
    setReplacementFD(devNull);
  }

  CaptureStream::CaptureStream(std::ostream& stream, FILE* fileObject): StreamInterceptor(stream, fileObject)
  {
    int pipeFD[2] {0};
    if (pipe2(pipeFD, O_NONBLOCK) < 0) {
      B2ERROR("Error creating pipe: " << std::strerror(errno));
      return;
    }
    m_pipeReadFD = pipeFD[0];
    setReplacementFD(pipeFD[1]);
  }

  CaptureStream::~CaptureStream()
  {
    // the base destructor is called after this one so we need to make sure
    // everything is finished before we close the pipe.
    finish();
    // no need to close the write part, done by base class
    if (m_pipeReadFD >= 0) close(m_pipeReadFD);
  }

  bool CaptureStream::start()
  {
    if (StreamInterceptor::start()) {
      CaptureStreamAbortHandler::getInstance().addObject(this);
      return true;
    }
    return false;
  }

  bool CaptureStream::finish()
  {
    if (StreamInterceptor::finish()) {
      CaptureStreamAbortHandler::getInstance().removeObject(this);
      readFD(m_pipeReadFD, m_outputStr);
      return true;
    }
    return false;
  }

  namespace {
    /** small helper function to format output into a nice error message:
     *  - do nothing if message is empty
     *  - trim white space from message
     *  - replace all newlines and add an indentation marker to distuingish
     *    that this is sill part of the message
     *
     *  @param logLevel log level of the message
     *  @param debugLevel debug level in case logLevel is c_Debug
     *  @param name name of the thing causing the output, e.g. Geant4
     *  @param indent string to add to the beginning of each line of the message
     *  @param message actuall message to output
     */
    void sendLogMessage(LogConfig::ELogLevel logLevel, int debugLevel, const std::string& name, const std::string& indent,
                        std::string message)
    {
      // avoid formatting if message will not be shown anyway
      if (!LogSystem::Instance().isLevelEnabled(logLevel, debugLevel)) return;
      // remove trailing whitespace and end of lines
      boost::algorithm::trim_right(message);
      // remove empty lines but keep white space in non empty first line
      while (!message.empty()) {
        std::string new_message = boost::algorithm::trim_left_copy_if(message, boost::algorithm::is_any_of(" \t\r"));
        if (new_message.empty()) {
          message = new_message;
        } else if (new_message[0] == '\n') {
          message = new_message.substr(1, std::string::npos);
          continue;
        }
        break;
      }
      // is the message empty?
      if (message.empty()) return;
      // add indentation
      boost::algorithm::replace_all(message, "\n", "\n" + indent);
      // fine, show message
      B2LOG(logLevel, debugLevel, "Output from " << name << ":\n" << indent << message);
    }
  }

  bool OutputToLogMessages::finish()
  {
    bool result = CaptureStdOutStdErr::finish();
    sendLogMessage(m_stdoutLevel, m_stdoutDebugLevel, m_name, m_indent, getStdOut());
    sendLogMessage(m_stderrLevel, m_stderrDebugLevel, m_name, m_indent, getStdErr());
    return result;
  }
}
