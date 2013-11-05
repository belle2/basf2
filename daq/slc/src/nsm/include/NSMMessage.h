#ifndef _Belle2_NSMMessage_hh
#define _Belle2_NSMMessage_hh

extern "C" {
#include "nsm/nsm2.h"
}

#include "nsm/NSMHandlerException.h"

namespace Belle2 {

  class NSMCommunicator;

  class NSMMessage {

    friend class NSMCommunicator;

  public:
    NSMMessage() throw();
    virtual ~NSMMessage() throw() {}

  public:
    const char* getRequestName() const throw();
    const char* getNodeName() const throw();
    unsigned short getRequestId() const throw();
    unsigned short getSequenceId() const throw();
    unsigned short getNodeId() const throw();
    unsigned short getNParams() const throw();
    unsigned int getParam(int i) const throw();
    unsigned int* getParams() throw();
    const unsigned int* getParams() const throw();
    unsigned int getLength() const throw();
    const std::string& getData() const throw();
    void setRequestId(unsigned short id) throw();
    void setSequenceId(unsigned short id) throw();
    void setNodeId(unsigned short id) throw();
    void setNParams(unsigned short id) throw();
    void setParam(int i, unsigned int v) throw();
    void setData(int len, const char* data)  throw();
    void setData(const std::string& text)  throw();

  public:
    size_t read(NSMcontext* nsmc) throw(NSMHandlerException);

  private:
    size_t try_read(int sock, char* buf, int datalen);
    NSMmsg* getMsg() { return &_nsm_msg; }

  private:
    mutable NSMcontext* _nsmc;
    NSMmsg _nsm_msg;
    std::string _text;

  };

};

#endif
