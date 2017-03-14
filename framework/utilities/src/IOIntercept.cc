/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/utilities/IOIntercept.h>
#include <framework/logging/Logger.h>
#include <memory>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>

namespace Belle2 {
  namespace IOIntercept {
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
        int size = read(fd, buffer.get(), 1024);
        if (size < 0) {
          // in case we get interrupted by signal, try again
          if (errno == EINTR)  continue;
          break;
        }
        out.append(buffer.get(), size);
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
        if (errno == EINTR || errno == EBUSY) {
          //sleep and repeat
        } else {
          B2ERROR("Error in dup2(), cannot replace file descriptor: " << std::strerror(errno));
          return false;
          break;
        }
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
      // no need to close the write part, done by base class
      if (m_pipeReadFD >= 0) close(m_pipeReadFD);
    }
  }
}
