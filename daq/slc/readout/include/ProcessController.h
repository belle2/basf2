/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_ProcessController_h
#define _Belle2_ProcessController_h

#include "daq/slc/readout/RunInfoBuffer.h"

#include "daq/slc/runcontrol/RCCallback.h"

#include "daq/slc/system/PThread.h"
#include "daq/slc/system/Process.h"
#include "daq/slc/system/Mutex.h"

#include <vector>
#include <sstream>

namespace Belle2 {

  class ProcessController {

    friend class ProcessSubmitter;

  public:
    ProcessController()
    {
      m_callback = NULL;
    }
    ProcessController(RCCallback* callback)
    {
      m_callback = callback;
    }
    ~ProcessController()
    {
      m_info.close();
    }

  public:
    bool init(const std::string& parname, int nodeid = 0);
    void clear();
    bool load(int timeout);
    bool start(int expno, int runno);
    bool pause();
    bool resume();
    bool stop();

    /***
     * Guaranteed abort by sending first SIGINT and after the given timeout SIGABRT in case SIGINT is not able to kill
     * the process. Same cleanup procedure as the abort method, but circumvents hanging SIGINT calls.
     * @param timeout time (in seconds)
     * @return
     */
    bool abort(unsigned int timeout = 60);

    const std::string& getName() { return m_name; }
    const std::string& getParName() { return m_parname; }
    const std::string& getExecutable() { return m_exename; }
    RunInfoBuffer& getInfo() { return m_info; }
    RCCallback* getCallback() { return m_callback; }
    const Process& getProcess() const { return  m_process; }
    Process& getProcess() { return  m_process; }
    void setCallback(RCCallback* callback) { m_callback = callback; }
    void setName(const std::string& name) { m_name = name; }
    void setExecutable(const std::string& exe) { m_exename = exe; }
    void addArgument(const std::string& arg) { m_arg_v.push_back(arg); }
    void addArgument(const char* format, ...);
    template<typename T>
    void addArgument(T arg);
    void clearArguments() { m_arg_v = std::vector<std::string>(); }
    bool isAlive() const { return m_process.isAlive(); }
    bool waitReady(int timeout);

  public:
    void lock() { m_mutex.lock(); }
    void unlock() { m_mutex.unlock(); }

  private:
    RunInfoBuffer m_info;
    std::string m_name;
    std::string m_parname;
    RCCallback* m_callback;
    std::string m_exename;
    std::vector<std::string> m_arg_v;
    Process m_process;
    Mutex m_mutex;
    std::string m_message;
    PThread m_th_log;
    PThread m_th_process;
    int m_iopipe[2];

  };

  template<typename T>
  inline void ProcessController::addArgument(T arg)
  {
    std::stringstream ss; ss << arg;
    m_arg_v.push_back(ss.str());
  }

  class ProcessSubmitter {

  public:
    ProcessSubmitter(ProcessController* con, int iopipe[2])
      : m_con(con)
    {
      m_iopipe[0] = iopipe[0];
      m_iopipe[1] = iopipe[1];
    }

  public:
    void run();

  private:
    ProcessController* m_con;
    int m_iopipe[2];

  };

}

#endif
