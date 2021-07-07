/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <iostream>
#include <string>
#include <framework/logging/LogConfig.h>

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
      /** Replace the file descriptor of m_fileObject with the one passed
       * @param fileDescriptor file descriptor to be set for m_fileObject using dup2()
       */
      bool replaceFD(int fileDescriptor);
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
      /** Start intercepting the output */
      bool start();
      /** Restore the stream and get the output from the pipe */
      bool finish();
    private:
      /** file descriptor of the read end of the pipe */
      int m_pipeReadFD{ -1};
      /** string with the output, only filled after finish() */
      std::string m_outputStr;
      /** allow handling of SIGABRT */
      friend class CaptureStreamAbortHandler;
    };


    /** Class to intercept stdout and stderr and either capture, discard or
     * keep them unmodified depending on the template arguments.
     *
     * For example
     * \code{.cc}
       IOIntercept::InterceptOutput<IOIntercept::CaptureStream, IOIntercept::DiscardStream> capture;
       capture.start();
       // here all output will be intercepted. Output to stdout will be buffered while
       // output to stderr will be discarded immediately.
       capture.finish();
       // output restored to normal. Buffered stdout can now be retrieved
       std::cout << "Output was: " << capture.getStdOut() << std::endl;
       \endcode
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

    /** Capture stdout and stderr and convert into log messages.
     * This class can be used to convert output by a third party library into basf2 log messages.
     *
     * For example
     * \code{.cc}
       IOIntercept::OutputToLogMessages capture("external_library");
       capture.start();
       // here all output will be intercepted. the call to finish will convert
       // any output to stdout to a B2INFO message and any output to stderr to
       // a B2ERROR message
       std::cerr << "this is my error"
       capture.finish();
       \endcode
     *
     * this will emit an ERROR message of the form
     *
     *    [ERROR] Output from external_library:
     *    external_library: this is my error
     *
     * The leading indentation defaults to the name supplied on construction
     * plus a colon and can be changed using the setIndent() member.
     *
     * \warning to NOT use this class for big periods of time. It is intended
     *   to be used around short calls to external software which produces
     *   output that cannot be converted to log messages otherwise. Don't try
     *   to just enable it the whole time. Not only will this cut off ouput
     *   that is longer then 64kB, it will also generate long and meaningless
     *   messages and will intercept normal log messages emitted while capture
     *   is active as well.
     */
    class OutputToLogMessages: public CaptureStdOutStdErr {
    public:
      /** Full constructor to choose the log levels and debug levels for both stdout and stderr
       * @param name name of the code causing the output, for example "ROOT", "Rave", ...
       * @param stdoutLevel severity of the log message to be emitted for output on stdout
       * @param stderrLevel severity of the log message to be emitted for output on stderr
       * @param stdoutDebugLevel debug level for the log message to be emitted for output on stdout if stdoutLevel is c_Debug
       * @param stderrDebugLevel debug level for the log message to be emitted for output on stderr if stderrLevel is c_Debug
       */
      OutputToLogMessages(const std::string& name, LogConfig::ELogLevel stdoutLevel, LogConfig::ELogLevel stderrLevel,
                          int stdoutDebugLevel, int stderrDebugLevel):
        m_name(name), m_indent(name + ": "), m_stdoutLevel(stdoutLevel), m_stderrLevel(stderrLevel),
        m_stdoutDebugLevel(stdoutDebugLevel), m_stderrDebugLevel(stderrDebugLevel)
      {}
      /** Constructor to choose the log levels both stdout and stderr.
       * If the level is set to c_Debug a debug level of 100 is used.
       * @param name name of the code causing the output, for example "ROOT", "Rave", ...
       * @param stdoutLevel severity of the log message to be emitted for output on stdout
       * @param stderrLevel severity of the log message to be emitted for output on stderr
       */
      OutputToLogMessages(const std::string& name, LogConfig::ELogLevel stdoutLevel, LogConfig::ELogLevel stderrLevel):
        OutputToLogMessages(name, stdoutLevel, stderrLevel, 100, 100)
      {}
      /** Simple constructor which uses c_Info for output on stdout and c_Error for output on stderr
       * @param name name of the code causing the output, for example "ROOT", "Rave", ...
       */
      explicit OutputToLogMessages(const std::string& name): OutputToLogMessages(name, LogConfig::c_Info, LogConfig::c_Error)
      {}
      /** Destructor to make sure that output is converted to messages on destruction */
      ~OutputToLogMessages() { finish(); }
      /** Set the indent for each line of the output, default is the supplied name + `": "` */
      void setIndent(const std::string& indent) { m_indent = indent; }
      /** Finish the capture and emit the message if output has appeard on stdout or stderr */
      bool finish();
    private:
      /** Name of the output producing tool/library */
      const std::string m_name;
      /** Identation to add to the beginning of each line of output. defaults to "${m_name}: " */
      std::string m_indent;
      /** severity of the log message to be emitted for output on stdout */
      LogConfig::ELogLevel m_stdoutLevel;
      /** severity of the log message to be emitted for output on stderr */
      LogConfig::ELogLevel m_stderrLevel;
      /** debug level for the log message to be emitted for output on stdout if m_stdoutLevel is c_Debug */
      int m_stdoutDebugLevel;
      /** debug level for the log message to be emitted for output on stderr if m_stderrLevel is c_Debug */
      int m_stderrDebugLevel;
    };

    /** Simple RAII guard for output interceptor
     *
     * In case you have one of the IOIntercept classes as member and want to
     * enable it in a function with multiple return paths this class makes sure
     * that the interceptor is properly finished on exiting the function/scope.
     *
     * \code{.cc}
       struct example {
         IOIntercept::OutputToLogMessages m_interceptor{"examplestruct"};
         int sign(int number) {
           // this starts interception which will last until the
           // InterceptorGuard is destructed so no matter which return is taken
           // the intercept will always be properly finished()
           IOIntercept::InteceptorGuard<IOIntercept::OutputToLogMessages> guard(m_interceptor);
           if(number<0) {
             return -1;
           } else if(number>0) {
             return 1;
           }
           return 0;
         }
       };
       \endcode
     *
     * \sa start_intercept for a convenience wrapper for this class
     */
    template<class T> class InterceptorScopeGuard {
    public:
      /** Construct a new instance for a given interceptor object and start intercepting io
       * @param interceptor the interceptor object to use, must stay valid
       *        during the lifetime of this object.
       */
      explicit InterceptorScopeGuard(T& interceptor): m_interceptor(&interceptor)
      {
        m_interceptor->start();
      }
      /** Move constructor which will take over the interception state */
      InterceptorScopeGuard(InterceptorScopeGuard<T>&& b): m_interceptor(b.m_interceptor)
      {
        b.m_interceptor = nullptr;
      }
      /** We don't want copying */
      InterceptorScopeGuard(const InterceptorScopeGuard<T>&) = delete;
      /** Also no assignment */
      InterceptorScopeGuard& operator=(const InterceptorScopeGuard&) = delete;
      /** Finish interception on cleanup */
      ~InterceptorScopeGuard()
      {
        if (m_interceptor) m_interceptor->finish();
      }
    private:
      /** pointer to the interceptor we guard */
      T* m_interceptor;
    };

    /** Convenience wrapper to simplify use of InterceptorScopeGuard<T>.
     * This function will return the correct scope guard without the need to
     * specify the template parameter.
     * \code{.cc}
       IOIntercept::OutputToLogMessages iointercept("myinterceptor");
       if(needIntercept){
         auto guard = IOIntercept::start_intercept(iointercept);
         // while the variable guard is in scope the intercept will be active
       }
       // intercept will be disabled here
       \endcode

     * @param interceptor the interceptor object to use, must stay valid
     *        during the lifetime of the returned object.
     */
    template<class T> InterceptorScopeGuard<T> start_intercept(T& interceptor)
    {
      return InterceptorScopeGuard<T> {interceptor};
    }
  }
}
