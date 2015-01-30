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
  class NSMMessage;

  class NSMVar {

  public:
    enum Type {
      NONE = 0,
      INT,
      FLOAT,
      TEXT
    };

  public:
    static const NSMVar NOVALUE;

  public:
    NSMVar(): m_name(), m_type(NONE), m_len(0), m_value(NULL) {}
    NSMVar(const std::string& name, Type type, int len, const void* value) {
      copy(name, type, len, value);
    }
    NSMVar(const std::string& name, const std::string& value) {
      copy(name, TEXT, 0, value.c_str());
    }
    NSMVar(const std::string& name, int value) { copy(name, INT, 0, &value); }
    NSMVar(const std::string& name, float value) { copy(name, FLOAT, 0, &value); }
    NSMVar(const std::string& name, int len, int* value) { copy(name, INT, len, value); }
    NSMVar(const std::string& name, int len, float* value) { copy(name, FLOAT, len, value); }
    NSMVar(const std::string& value) { copy("", TEXT, 0, value.c_str()); }
    NSMVar(int value) { copy("", INT, 0, &value); }
    NSMVar(float value) { copy("", FLOAT, 0, &value); }
    NSMVar(int len, int* value) { copy("", INT, len, value); }
    NSMVar(int len, float* value) { copy("", FLOAT, len, value); }
    NSMVar(const NSMVar& var) { copy(var.m_name, var.m_type, var.m_len, var.m_value); }
    ~NSMVar() throw();

  public:
    void setName(const std::string& name) { m_name = name; }
    const void* get() const { return m_value; }
    void* get() { return m_value; }
    int size() const;
    const std::string& getName() const { return m_name; }
    Type getType() const { return m_type; }
    int getLength() const { return m_len; }
    int getInt() const { return (m_type == INT && m_len == 0) ? *(int*)m_value : 0; }
    float getFloat() const { return (m_type == FLOAT && m_len == 0) ? *(float*)m_value : 0; }
    std::string getText() const { return (m_type == TEXT && m_len > 0) ? (const char*)m_value : ""; }
    int getInt(int i) const { return (m_type == INT && i < m_len) ? ((int*)m_value)[i] : 0; }
    float getFloat(int i) const { return (m_type == FLOAT && i < m_len) ? ((float*)m_value)[i] : 0; }

  private:
    void copy(const std::string& name,
              Type type, int len, const void* value);
  private:
    std::string m_name;
    Type m_type;
    int m_len;
    void* m_value;

  };

  class NSMMessage : public Serializable {

    friend class NSMCommunicator;

  public:
    static const unsigned int DATA_SIZE;

  public:
    NSMMessage() throw();
    NSMMessage(const NSMNode& node) throw();
    NSMMessage(const NSMNode& node, const NSMCommand& cmd) throw();
    NSMMessage(const NSMNode& node, const NSMCommand& cmd,
               int npar, int* pars) throw();
    NSMMessage(const NSMNode& node, const NSMCommand& cmd,
               int par, const std::string& obj) throw();
    NSMMessage(const NSMNode& node, const NSMCommand& cmd,
               const std::string& obj) throw();
    NSMMessage(const NSMCommand& cmd) throw();
    NSMMessage(const NSMMessage& msg) throw();
    NSMMessage(const NSMNode& node,
               const NSMVar& var) throw();
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
    void setRequestName(const std::string& name) throw();
    void setRequestName(const NSMCommand& cmd) throw();
    void setRequestName() throw();
    void setNodeName(const std::string& name) throw();
    void setNodeName(const NSMNode& node) throw();
    void setRequestId(unsigned short id) throw();
    void setSequenceId(unsigned short id) throw();
    void setNodeId(unsigned short id) throw();
    void setNParams(unsigned short id) throw();
    void setParam(int i, unsigned int v) throw();
    void setData(int len, const char* data)  throw();
    void setData(const std::string& text)  throw();
    const NSMVar getVar();

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
    bool m_hasobj;
    NSMVar m_var;

  };

};

#endif
