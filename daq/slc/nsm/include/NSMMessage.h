#ifndef _Belle2_NSMMessage_hh
#define _Belle2_NSMMessage_hh

extern "C" {
#include "nsm2/nsm2.h"
#include "nsm2/nsmsys2.h"
}

#include "daq/slc/nsm/NSMHandlerException.h"

#include "daq/slc/base/Serializable.h"

namespace Belle2 {

  class NSMCommunicator;

  class NSMMessage : public Serializable {

    friend class NSMCommunicator;

  public:
    static const unsigned int DATA_SIZE;

  public:
    NSMMessage() throw();
    NSMMessage(const NSMMessage& msg) throw();
    virtual ~NSMMessage() throw() {}

  public:
    const NSMMessage& operator=(const NSMMessage& msg) throw();

  public:
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
    void setRequestId(unsigned short id) throw();
    void setSequenceId(unsigned short id) throw();
    void setNodeId(unsigned short id) throw();
    void setNParams(unsigned short id) throw();
    void setParam(int i, unsigned int v) throw();
    void setData(int len, const char* data)  throw();
    void setData(const std::string& text)  throw();
    void setData(const Serializable& obj) throw(IOException);
    virtual void readObject(Reader&) throw(IOException);
    virtual void writeObject(Writer&) const throw(IOException);

  public:
    size_t read(NSMcontext* nsmc) throw(NSMHandlerException);
    NSMmsg* getMsg() { return &_nsm_msg; }

  private:
    int try_read(int sock, char* buf, int datalen);

  private:
    mutable NSMcontext* _nsmc;
    NSMmsg _nsm_msg;
    int _data[NSM_TCPDATSIZ];

  };

};

#endif
