#include <eutel/eudaq/TransportFactory.h>
#include <eutel/eudaq/Exception.h>
#include <iostream>
#include <ostream>
#include <string>
#include <map>

// All transport header files must be included here
#include <eutel/eudaq/TransportNULL.h>
#include <eutel/eudaq/TransportTCP.h>

namespace eudaq {

  /** Stores the name and factory methods for a type of Transport.
   */
  struct TransportFactory::TransportInfo {
    typedef TransportServer* (*ServerFactory)(const std::string&);
    typedef TransportClient* (*ClientFactory)(const std::string&);
    TransportInfo() : name(""), serverfactory(0), clientfactory(0) {}
    TransportInfo(const std::string& nm, ServerFactory sf, ClientFactory cf):
      name(nm), serverfactory(sf), clientfactory(cf) {}
    std::string name;
    ServerFactory serverfactory;
    ClientFactory clientfactory;
  };

  namespace {

    template <typename T_Server, typename T_Client>
    struct MakeTransportInfo : public TransportFactory::TransportInfo {
      MakeTransportInfo(const std::string& thename) :
        TransportInfo(thename, theserverfactory, theclientfactory)
      {
      }
      static TransportServer* theserverfactory(const std::string& param)
      {
        return new T_Server(param);
      }
      static TransportClient* theclientfactory(const std::string& param)
      {
        return new T_Client(param);
      }
    };

    typedef std::map<std::string, TransportFactory::TransportInfo> map_t;

    static map_t& TransportMap()
    {
      static bool initialised = false;
      if (!initialised) {
        initialised = true;
        // All transports have to be registered here
        TransportFactory::Register(MakeTransportInfo<NULLServer, NULLClient>(NULLServer::name));
        TransportFactory::Register(MakeTransportInfo<TCPServer, TCPClient>(TCPServer::name));
      }
      static map_t m;
      return m;
    }
  }

  void TransportFactory::Register(const TransportInfo& info)
  {
    //std::cout << "DEBUG: TransportFactory::Register " << info.name << std::endl;
    TransportMap()[info.name] = info;
  }

  TransportServer* TransportFactory::CreateServer(const std::string& name)
  {
    std::string proto = "tcp", param = name;
    size_t i = name.find("://");
    if (i != std::string::npos) {
      proto = std::string(name, 0, i);
      param = std::string(name, i + 3);
    }
    map_t::const_iterator it = TransportMap().find(proto);
    if (it == TransportMap().end()) EUDAQ_THROW("Unknown protocol: " + proto);
    return (it->second.serverfactory)(param);
  }

  TransportClient* TransportFactory::CreateClient(const std::string& name)
  {
    std::string proto = "tcp", param = name;
    size_t i = name.find("://");
    if (i != std::string::npos) {
      proto = std::string(name, 0, i);
      param = std::string(name, i + 3);
    }
    map_t::const_iterator it = TransportMap().find(proto);
    if (it == TransportMap().end()) EUDAQ_THROW("Unknown protocol: " + proto);
    return (it->second.clientfactory)(param);
  }

}
