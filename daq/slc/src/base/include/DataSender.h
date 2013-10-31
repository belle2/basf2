#ifndef _Belle2_DataSender_hh
#define _Belle2_DataSender_hh

#include "Node.h"

#include <string>

namespace Belle2 {

  class DataSender : public Node {

  public:
    DataSender() : _script("") {}
    virtual ~DataSender() throw() {}

  public:
    const std::string& getScript() const throw() { return _script; }
    const std::string& getHost() const throw() { return _host; }
    short getPort() const throw() { return _port; }
    int getEventSize() const throw() { return _event_size; }
    void setScript(const std::string& script) throw() { _script = script; }
    void setHost(const std::string& host) throw() { _host = host; }
    void setPort(short port) throw() { _port = port; }
    void setEventSize(int size) throw() { _event_size = size; }
    virtual const std::string getSQLFields() const throw();
    virtual const std::string getSQLLabels() const throw();
    virtual const std::string getSQLValues() const throw();

  private:
    std::string _script;
    std::string _host;
    short _port;
    int _event_size;

  };

}

#endif
