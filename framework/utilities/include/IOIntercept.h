/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <iostream>
#include <string>

namespace Belle2 {
  /** Encapsulate all classes needed to intercept stdout and stderr */
  namespace IOIntercept {
    /** Base class with all necessary features to intercept output to a file descriptor */
    class StreamInterceptor {
    public:
      /** Construct keeping a reference to the std::ostream and the file
       * descriptor which are associated with this stream as well as a file
       * descriptor for the replacement
       *
       * @param stream C++ ostream which is associated with the file descriptor we want to intercept
       * @param fileObject file object which is associated with the file descriptor we want to intercept
       * @param replacementFD file descriptor to use as replacement while capturing
       */
      StreamInterceptor(std::ostream& stream, FILE* fileObject);
      /** close file descriptors */
      ~StreamInterceptor();

      /** start intercepting the stream.
       * @return true on success, false on any error */
      bool start()
      {
        // only start if we are not already running
        if (!m_capturing) m_capturing = replaceFD(m_replacementFD);
        return m_capturing;
      }
      /** stop intercepting the stream.
       * @return true if the object was capturing and stream could be restored  */
      bool finish()
      {
        if (!m_capturing) return false;
        m_capturing = false;
        return replaceFD(m_savedFD);
      }
    protected:
      /** set the replacement file descriptor, should be called in the constructor of derived classes */
      void setReplacementFD(int fd) { m_replacementFD = fd; }
      /** Read the contents of a file descriptor until there is no more input and place them in out
       * @param fd file descriptor to read, should be opened in non blocking mode O_NOBLOCK
       * @param out string to be replaced with all the bytes read from fd
       */
      static void readFD(int fd, std::string& out);
    private:
      /** C++ stream object, only needed to flush before replacement */
      std::ostream& m_stream;
      /** File object of the file we want to replace, needed to obtain file descriptor and to flush */
      FILE* m_fileObject;
      /** Saved file descriptor: a duplicate of the file descriptor of m_fileObject */
      int m_savedFD{ -1};
      /** Replacement file descriptor to be used while capturing */
      int m_replacementFD{ -1};
      /** Check whether we are already capturing */
      bool m_capturing{false};

      /** Replace the file descriptor of m_fileObject with the one passed
       * @param fileDescriptor file descriptor to be set for m_fileObject using dup2()
       */
      bool replaceFD(int fileDescriptor);
    };

    /** Dummy class which keeps the stream unmodified */
    class KeepStream {
    public:
      /** This class is basically empty so the constructor does nothing */
      KeepStream(std::ostream&, FILE*) {}
      /** doing nothing always succeds */
      bool start() const { return true; }
      /** doing nothing always succeds */
      bool finish() const { return true; }
    };

    /** Simple class to just discard anything written to stream by redirecting it to /dev/null */
    class DiscardStream: public StreamInterceptor {
    public:
      /** Create StreamInterceptor which will redirect to /dev/null */
      DiscardStream(std::ostream& stream, FILE* fileObject);
    };

    /** Class to capture anything written to stream into a string.
     *
     * Underlying this uses a pipe to buffer the output.
     *
     * @warning the default capacity for capturing output is 64 kB but might be
     * smaller on certain systems. If there is more output than this while
     * capturing it the captured output will be truncated and processes who
     * check if output was successful might fail.
     */
    class CaptureStream: public StreamInterceptor {
    public:
      /** Create a StreamInterceptor which writes into a pipe */
      CaptureStream(std::ostream& stream, FILE* fileObject);
      /** Close file descriptors */
      ~CaptureStream();
      /** Get the output, only set after finish() */
      const std::string& getOutput() const { return m_outputStr; }
      /** Restore the stream and get the output from the pipe */
      bool finish()
      {
        if (StreamInterceptor::finish()) {
          readFD(m_pipeReadFD, m_outputStr);
          return true;
        }
        return false;
      }
    private:
      /** file descriptor of the read end of the pipe */
      int m_pipeReadFD{ -1};
      /** string with the output, only filled after finish() */
      std::string m_outputStr;

    };


    /** Class to intercept stdout and stderr and either capture, discard or
     * keep them unmodified depending on the template arguments.
     *
     * For example
     * \code{.cc}
       IOIntercept::InterceptOutput<IOIntercept::CaptureStream, IOIntercept::DiscardStream> capture;
       capture.start();
       // here all will be intercepted. Output to stdout will be buffered while
       // output to stderr will be discarded immediately.
       capture.finish();
       // output restored to normal. Buffered stdout can now be retrieved
       std::cout << "Output was: " << capture.getStdOut() << std::endl;
     *
     * Shorthand classes are defined for all use cases, so in the above example
     * we could have just used IOIntercept::CaptureStdOutDiscardStdErr;
     *
     * @warning the default capacity for capturing output is 64 kB but might be
     * shorter on certain systems. If there is more output than this to either
     * stdout or stderr while capturing it the captured output will be
     * truncated and processes who check if output was successful might fail.
     *
     * @sa CaptureStdOutStdErr, CaptureStdOut, CaptureStdErr, DiscardStdOutStdErr,
     *     DiscardStdOut, DiscardStdErr, CaptureStdOutDiscardStdErr, DiscardStdOutCaptureStdErr
     * */
    template<class STDOUT, class STDERR>
    class InterceptOutput {
    public:
      /** Start intercepting the output. This function can be called multiple times.
       * @return true if interception could be started successfully or is already active */
      bool start() { return m_stdout.start() && m_stderr.start(); }
      /** Finish intercepting the output. This function can be called multiple times.
       * @return true if interception was active and could be removed successfully */
      bool finish() { return m_stdout.finish() && m_stderr.finish(); }
      /** Return the captured stdout output if any. Only filled after finish() is called */
      const std::string& getStdOut() const
      {
        static_assert(std::is_same<STDOUT, CaptureStream>::value, "Only valid if stdout is captured using CaptureStream");
        return m_stdout.getOutput();
      }
      /** Return the captured stderr output if any. Only filled after finish() is called */
      const std::string& getStdErr() const
      {
        static_assert(std::is_same<STDERR, CaptureStream>::value, "Only valid if stderr is captured using CaptureStream");
        return m_stderr.getOutput();
      }

    private:
      /** StreamInterceptor for stdout */
      STDOUT m_stdout{std::cout, stdout};
      /** StreamInterceptor for stderr */
      STDERR m_stderr{std::cerr, stderr};
    };

    /** Capture both stdout and stderr as strings */
    using CaptureStdOutStdErr = InterceptOutput<CaptureStream, CaptureStream>;
    /** Capture only stdout and don't modify stderr */
    using CaptureStdOut = InterceptOutput<CaptureStream, KeepStream>;
    /** Capture only stderr and don't modify stdout */
    using CaptureStdErr = InterceptOutput<KeepStream, CaptureStream>;
    /** Discard both stdout and stderr */
    using DiscardStdOutStdErr = InterceptOutput<DiscardStream, DiscardStream>;
    /** Discard only stdout and don't modify stderr */
    using DiscardStdOut = InterceptOutput<DiscardStream, KeepStream>;
    /** Discard only stderr and don't modify stdout */
    using DiscardStdErr = InterceptOutput<KeepStream, DiscardStream>;
    /** Capture stdout and discard stderr */
    using CaptureStdOutDiscardStdErr = InterceptOutput<CaptureStream, DiscardStream>;
    /** Capture stderr and discard stdout */
    using DiscardStdOutCaptureStdErr = InterceptOutput<DiscardStream, CaptureStream>;
  }
}
