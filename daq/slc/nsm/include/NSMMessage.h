#ifndef _Belle2_NSMMessage_hh
#define _Belle2_NSMMessage_hh

extern "C" {
#include "nsm2/nsm2.h"
#include "nsm2/nsmsys2.h"
}

#include "daq/slc/nsm/NSMHandlerException.h"
#include "daq/slc/nsm/NSMCommand.h"
#include "daq/slc/nsm/NSMNode.h"

#include <daq/slc/system/Buffer.h>

#include <daq/slc/base/Serializable.h>

namespace Belle2 {

  class NSMCommunicator;
  class NSMCommand;
  class NSMNode;

  class NSMMessage : public Serializable {

    friend class NSMCommunicator;

  public:
    static const unsigned int DATA_SIZE;

  public:
    NSMMessage() throw();
    NSMMessage(const NSMNode& node) throw();
    NSMMessage(const NSMNode& node,
               const NSMCommand& cmd) throw();
    NSMMessage(const NSMNode& node,
               const NSMCommand& cmd,
               int npar, int* pars) throw();
    NSMMessage(const NSMNode& node,
               const NSMCommand& cmd,
               int par, const Serializable& obj) throw();
    NSMMessage(const NSMNode& node,
               const NSMCommand& cmd,
               int par, const std::string& obj) throw();
    NSMMessage(const NSMNode& node,
               const NSMCommand& cmd,
               const Serializable& obj) throw();
    NSMMessage(const NSMNode& node,
               const NSMCommand& cmd,
               const std::string& obj) throw();
    NSMMessage(const NSMCommand& cmd) throw();
    NSMMessage(const NSMMessage& msg) throw();
    virtual ~NSMMessage() throw() { }

  public:
    const NSMMessage& operator=(const NSMMessage& msg) throw();

  public:
    void init() throw();
    const char* getRequestName() const throw();
    const char* getNodeName() const throw();
    unsigned short getRequestId() const throw();
    unsigned short getSequenceId() const throw();
    unsigned short getNodeId() const throw();
    unsigned short getNParams() const throw();
    int getParam(int i) const throw();
#if NSM_PACKAGE_VERSION >= 1914
    int* getParams() throw();
    const int* getParams() const throw();
#else
    unsigned int* getParams() throw();
    const unsigned int* getParams() const throw();
#endif
    unsigned int getLength() const throw();
    const char* getData() const throw();
    void getData(Serializable& obj) const throw(IOException);
    void setRequestName(const std::string& name) throw();
    void setRequestName(const NSMCommand& cmd) throw();
    void setRequestName() throw();
    void setNodeName(const std::string& name) throw();
    void setNodeName(const NSMNode& node) throw();
    //void setLength(unsigned int len) throw();
    void setRequestId(unsigned short id) throw();
    void setSequenceId(unsigned short id) throw();
    void setNodeId(unsigned short id) throw();
    void setNParams(unsigned short id) throw();
    void setParam(int i, unsigned int v) throw();
    void setData(int len, const char* data)  throw();
    void setData(const std::string& text)  throw();
    void setData(const Serializable& obj) throw(IOException);

  public:
    virtual void readObject(Reader&) throw(IOException);
    virtual void writeObject(Writer&) const throw(IOException);

  public:
    size_t read(NSMcontext* nsmc) throw(NSMHandlerException);
    NSMmsg* getMsg() { return &m_nsm_msg; }

  private:
    int try_read(int sock, char* buf, int datalen);

  private:
    mutable NSMcontext* m_nsmc;
    NSMmsg m_nsm_msg;
    Buffer m_data;
    mutable std::string m_nodename;
    mutable std::string m_reqname;

  };

};

#endif
