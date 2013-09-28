#ifndef _B2DQM_PackageSender
#define _B2DQM_PackageSender

#include <system/TCPSocket.hh>

#include <util/Writer.hh>

#include <vector>

namespace B2DQM {

  class HistoServer;

  class PackageSender {

  private:
    static const int FLAG_ALL;
    static const int FLAG_UPDATE;
    static const int FLAG_LIST;

  private:
    struct zipped_buf {
      char* buf;
      size_t size;
      size_t count;
    };
    
  public:
    PackageSender(HistoServer* server,
		  B2DAQ::TCPSocket& socket)
      : _server(server), _socket(socket) {}
    ~PackageSender();
    
  public:
    void run();

  private:
    bool sendUpdates(B2DAQ::Writer& writer, size_t i, int n)
      throw(B2DAQ::IOException);
    
  private:
    HistoServer* _server;
    B2DAQ::TCPSocket _socket;
    std::vector<zipped_buf> _conf_v;
    std::vector<zipped_buf> _contents_v;
    std::vector<std::string> _xml_v;
    std::vector<int> _update_id_v;

  };

}

#endif
