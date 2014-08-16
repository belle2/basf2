#include <daq/slc/nsm/NSMNodeDaemon.h>
#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/system/Mutex.h>
#include <daq/slc/system/Cond.h>
#include <daq/slc/system/PThread.h>
#include <daq/slc/system/UDPSocket.h>
#include <daq/slc/system/TCPServerSocket.h>
#include <daq/slc/system/TCPSocketWriter.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Time.h>

#include <cstring>
#include <map>

#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const unsigned short BUFFER_MAX = 10240;
const unsigned int UDP_TCP_PORT = 9020;

std::string g_hostname;

Belle2::NSMDataStore& g_dstore(Belle2::NSMDataStore::getStore());

namespace Belle2 {

  typedef std::map<std::string, SharedMemory> MemoryList;

}

Belle2::MemoryList g_mem_m;
Belle2::Mutex g_mutex;

namespace Belle2 {

  struct NSMDataPaket {
    struct Header {
      unsigned short paketid;
      unsigned short flag;
      unsigned short id;
      unsigned short revision;
      unsigned int max;
      unsigned int offset;
      unsigned int size;
    };
    Header hdr;
    char buf[BUFFER_MAX];
  };

  class UDPListener {

  public:
    void run() {
      UDPSocket socket(UDP_TCP_PORT);
      socket.bind();
      NSMDataPaket paket;
      while (true) {
        memset(&paket, 0, sizeof(paket));
        socket.read((char*)&paket, sizeof(paket));
        std::string remotehost = socket.getRemoteHostName();
        unsigned int addr = socket.getRemoteAddress();
        if (g_hostname == remotehost) continue;
        if (paket.hdr.flag == NSMCommand::NSMSET.getId()) {
          NSMDataStore::Entry* en = g_dstore.get(addr, paket.hdr.id);
          if (en == NULL) {
            StringList str_v = StringUtil::split(paket.buf, '\n');
            const std::string name = str_v[0];
            const std::string format = str_v[1];
            en = g_dstore.add(addr, paket.hdr.max, paket.hdr.revision,
                              name, format, paket.hdr.id);
            SharedMemory cmem;
            std::string path = StringUtil::form("%s:%s",
                                                remotehost.c_str(),
                                                name.c_str());
            cmem.open(path, paket.hdr.max);
            cmem.map();
            g_mutex.lock();
            g_mem_m.insert(MemoryList::value_type(name, cmem));
            g_mutex.unlock();
          }
          en->size = paket.hdr.max;
          if (g_mem_m.find(en->name) == g_mem_m.end()) {
            SharedMemory cmem;
            std::string path = StringUtil::form("%s:%s",
                                                remotehost.c_str(),
                                                en->name);
            cmem.open(path, en->size);
            cmem.map();
            g_mutex.lock();
            g_mem_m.insert(MemoryList::value_type(en->name, cmem));
            g_mutex.unlock();
          }
          SharedMemory& mem(g_mem_m[en->name]);
          char* buf = (char*)mem.map();
          memcpy((buf + paket.hdr.offset), paket.buf, paket.hdr.size);
          if (paket.hdr.size + paket.hdr.offset == paket.hdr.max) {
            en->utime = Time().getSecond();
            g_dstore.signal();
          }
        } else if (paket.hdr.flag == NSMCommand::NSMGET.getId()) {
          if (paket.hdr.id == 0) {
            NSMDataStore::Entry* en = g_dstore.get(paket.buf);
            if (en != NULL) {
              LogFile::info("Accepted UDP GET requst from:%s (%s) %d",
                            socket.getRemoteHostName().c_str(),
                            socket.getHostName().c_str(), paket.hdr.id);
              std::string str = StringUtil::form("%s\n%s", en->name, en->format);
              paket.hdr.flag = NSMCommand::NSMSET.getId();
              paket.hdr.max = en->size;
              paket.hdr.id = en->id;
              memset(paket.buf, 0, BUFFER_MAX);
              strcpy(paket.buf, str.c_str());
              paket.hdr.size = str.size() + 1;
              UDPSocket socket_r(UDP_TCP_PORT, remotehost);
              socket_r.write(&paket, sizeof(NSMDataPaket::Header) + paket.hdr.size);
              socket_r.close();
            }
          } else {
            NSMDataStore::Entry* en = g_dstore.get(paket.hdr.id);
            if (en != NULL) {
              if (g_mem_m.find(en->name) == g_mem_m.end()) {
                SharedMemory cmem;
                std::string path = StringUtil::form("127.0.0.1:%s", en->name);
                cmem.open(path, en->size);
                cmem.map();
                g_mutex.lock();
                g_mem_m.insert(MemoryList::value_type(en->name, cmem));
                g_mutex.unlock();
              }
              SharedMemory& mem(g_mem_m[en->name]);
              paket.hdr.flag = NSMCommand::NSMSET.getId();
              paket.hdr.offset = 0;
              paket.hdr.max = en->size;
              paket.hdr.paketid = 0;
              UDPSocket socket_r(UDP_TCP_PORT, remotehost);
              char* buf = (char*)mem.map();
              while (paket.hdr.offset < paket.hdr.max) {
                int size = (paket.hdr.max - paket.hdr.offset);
                paket.hdr.size = (size < BUFFER_MAX) ? size : BUFFER_MAX;
                memcpy(paket.buf, (buf + paket.hdr.offset), paket.hdr.size);
                socket_r.write(&paket, sizeof(NSMDataPaket::Header) + paket.hdr.size);
                paket.hdr.offset += paket.hdr.size;
                ++paket.hdr.paketid;
              }
              socket_r.close();
            } else {
              LogFile::debug("no data for %d", paket.hdr.id);
            }
          }
        }
      }
    }
  };

  class TCPListener {
  public:
    TCPListener(const TCPSocket& socket)
      : m_socket(socket) {}

  private:
    TCPSocket m_socket;

  public:
    void run() {
      try {
        LogFile::info("Accepted connection from : %s", m_socket.getIP().c_str());
        TCPSocketReader reader(m_socket);
        while (true) {
          NSMDataPaket paket;
          paket.hdr.flag = reader.readInt();
          paket.hdr.max = reader.readInt();
          paket.hdr.revision = reader.readInt();
          std::string name = reader.readString();
          g_mutex.lock();
          UDPSocket udp;
          try {
            if (paket.hdr.flag == NSMCommand::NSMGET.getId()) {
              NSMDataStore::Entry* en = g_dstore.get(name);
              if (en == NULL) {
                LogFile::debug("Sending UDP request with broadcast");
                udp = UDPSocket(UDP_TCP_PORT, g_hostname, true);
                paket.hdr.id = 0;
                strcpy(paket.buf, name.c_str());
                udp.write(&paket, sizeof(NSMDataPaket::Header) + name.size() + 1);
              } else if (en->addr == 0) {
                LogFile::warning("Should be local memory :%s", name.c_str());
              } else {
                udp = UDPSocket(UDP_TCP_PORT, en->addr);
                paket.hdr.id = en->rid;
                udp.write(&paket, sizeof(NSMDataPaket::Header));
              }
            }
            udp.close();
            g_mutex.unlock();
          } catch (const IOException& e) {
            udp.close();
            m_socket.close();
            g_mutex.unlock();
            return;
          }
        }
      } catch (const IOException& e) {
        m_socket.close();
      }
    }
  };
}

using namespace Belle2;

void signaleHandler(int)
{
  g_dstore.unlink();
  exit(0);
}

int main(int argc, char** argv)
{
  signal(SIGINT, signaleHandler);
  signal(SIGQUIT, signaleHandler);
  if (argc < 3) {
    LogFile::debug("Usage : %s hostname port", argv[0]);
    return 1;
  }
  g_hostname = argv[1];
  int max = (argc > 2) ? atoi(argv[2]) : 0;
  TCPServerSocket server;
  server.open("0.0.0.0", UDP_TCP_PORT);
  g_dstore.open(max);
  g_dstore.init();
  PThread(new UDPListener());
  while (true) {
    PThread(new TCPListener(server.accept()));
  }

  return 0;
}
