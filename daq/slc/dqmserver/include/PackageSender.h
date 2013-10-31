#ifndef _Belle2_PackageSender
#define _Belle2_PackageSender

#include <system/TCPSocket.h>

#include <base/Writer.h>

#include <vector>

namespace Belle2 {

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
                  Belle2::TCPSocket& socket)
      : _server(server), _socket(socket) {}
    ~PackageSender();

  public:
    void run();

  private:
    bool sendUpdates(Belle2::Writer& writer, size_t i, int n)
    throw(Belle2::IOException);

  private:
    HistoServer* _server;
    Belle2::TCPSocket _socket;
    std::vector<zipped_buf> _conf_v;
    std::vector<zipped_buf> _contents_v;
    std::vector<std::string> _xml_v;
    std::vector<int> _update_id_v;

  };

}

#endif
