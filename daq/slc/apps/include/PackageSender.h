#ifndef _Belle2_PackageSender
#define _Belle2_PackageSender

#include "daq/slc/apps/MonitorMaster.h"

#include <daq/slc/system/TCPSocket.h>

#include <daq/slc/base/Writer.h>

#include <vector>

namespace Belle2 {

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
    PackageSender(TCPSocket& socket,
                  MonitorMaster* master)
      : _master(master), _socket(socket) {}
    ~PackageSender();

  public:
    void run();

  private:
    bool sendUpdates(Writer& writer, size_t i, int n)
    throw(IOException);

  private:
    MonitorMaster* _master;
    TCPSocket _socket;
    std::vector<zipped_buf> _conf_v;
    std::vector<zipped_buf> _contents_v;
    std::vector<std::string> _xml_v;
    std::vector<int> _update_id_v;

  };

}

#endif
