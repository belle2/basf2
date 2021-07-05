/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_NSMMessage_hh
#define _Belle2_NSMMessage_hh

extern "C" {
#include <nsm2/nsm2.h>
}

#include "daq/slc/nsm/NSMCommand.h"
#include "daq/slc/nsm/NSMNode.h"
#include "daq/slc/nsm/NSMVar.h"

#include "daq/slc/database/DAQLogMessage.h"

#include <daq/slc/system/Buffer.h>

#include <daq/slc/base/Serializable.h>

#include <string>

namespace Belle2 {

  class NSMCommunicator;
  class NSMData;

  class NSMMessage : public Serializable {

    friend class NSMCommunicator;

  public:
    static const unsigned int DATA_SIZE;

  public:
    NSMMessage();
    NSMMessage(const NSMNode& node);
    NSMMessage(const NSMNode& node, const NSMCommand& cmd);
    NSMMessage(const NSMNode& node, const NSMCommand& cmd,
               int npar, int* pars);
    NSMMessage(const NSMNode& node, const NSMCommand& cmd,
               int par, const std::string& data);
    NSMMessage(const NSMNode& node, const NSMCommand& cmd, int par);
    NSMMessage(const NSMNode& node, const NSMCommand& cmd,
               const std::string& data);
    NSMMessage(const NSMNode& node, const NSMVar& var);
    NSMMessage(const NSMNode& node, const DAQLogMessage& log);
    NSMMessage(const NSMNode& node, const DAQLogMessage& log, const NSMCommand& cmd);
    NSMMessage(const NSMNode& node, const NSMData& data);
    NSMMessage(const NSMCommand& cmd);
    NSMMessage(const NSMCommand& cmd, int par);
    NSMMessage(const NSMCommand& cmd, int npar, int* pars);
    NSMMessage(const NSMNode& node, const NSMCommand& cmd, int npar, int* pars,
               const std::string& data);
    NSMMessage(const NSMCommand& cmd, const std::string& data);
    NSMMessage(const NSMVar& var);
    NSMMessage(const DAQLogMessage& log);
    NSMMessage(const NSMData& data);
    NSMMessage(const NSMMessage& msg);
    virtual ~NSMMessage() { }

  public:
    const NSMMessage& operator=(const NSMMessage& msg);

  public:
    void init();
    void init(const NSMNode& node, const NSMVar& var);
    void init(const NSMNode& node, const DAQLogMessage& log);
    void init(const NSMNode& node, const DAQLogMessage& log, const NSMCommand& cmd);
    void init(const NSMNode& node, const NSMData& data);
    const char* getRequestName() const;
    const char* getNodeName() const;
    unsigned short getRequestId() const;
    unsigned short getSequenceId() const;
    unsigned short getNodeId() const;
    unsigned short getNParams() const;
    int getParam(int i) const;
#if NSM_PACKAGE_VERSION >= 1914
    int* getParams();
    const int* getParams() const;
#else
    unsigned int* getParams();
    const unsigned int* getParams() const;
#endif
    unsigned int getLength() const;
    const char* getData() const;
    void setRequestName(const std::string& name);
    void setRequestName(const NSMCommand& cmd);
    void setRequestName();
    void setNodeName(const std::string& name);
    void setNodeName(const NSMNode& node);
    void setRequestId(unsigned short id);
    void setSequenceId(unsigned short id);
    void setNodeId(unsigned short id);
    void setNParams(unsigned short id);
    void setParam(int i, unsigned int v);
    void setData(int len, const char* data) ;
    void setData(const std::string& text) ;
    const NSMVar getVar();

  public:
    NSMMessage wait(int timeout);
    void push(const NSMMessage& msg);

  public:
    virtual void readObject(Reader&);
    virtual void writeObject(Writer&) const;

  public:
    size_t read(NSMcontext* nsmc);
    NSMmsg* getMsg() { return &m_nsm_msg; }

  private:
    int try_read(int sock, char* buf, int datalen);

  private:
    mutable NSMcontext* m_nsmc;
    NSMmsg m_nsm_msg;
    Buffer m_data;
    mutable std::string m_nodename;
    mutable std::string m_reqname;
    bool m_hasobj;
    NSMVar m_var;

  };

};

#endif
