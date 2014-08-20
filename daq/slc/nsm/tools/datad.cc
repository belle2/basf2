#include <daq/slc/nsm/NSMNodeDaemon.h>
#include <daq/slc/nsm/NSMData.h>
#include <daq/slc/nsm/NSMDataPaket.h>

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

Belle2::NSMDataStore& g_dstore(Belle2::NSMDataStore::getStore());

using namespace Belle2;

void signaleHandler(int)
{
  g_dstore.unlink();
  exit(0);
}

int main(int argc, char** argv)
{
  if (argc < 1) {
    LogFile::debug("Usage : %s [nmemories] [-d]", argv[0]);
    return 1;
  }
  signal(SIGINT, signaleHandler);
  signal(SIGQUIT, signaleHandler);
  signal(SIGTERM, signaleHandler);

  unsigned int maxmem = 50;
  UDPSocket socket(NSMDataPaket::PORT);
  try {
    socket.bind();
  } catch (const IOException& e) {
    LogFile::error("UDP port %u is already in use. Exiting process", NSMDataPaket::PORT);
    return 1;
  }

  while (argc > 0) {
    if (isdigit(*argv[0]))
      maxmem = atoi(*argv);
    else if (strcmp(*argv, "-d") == 0)
      daemon(0, 0);
    argv++;
    argc--;
  }

  LogFile::open("datad");
  LogFile::info("Binded UDP port %u", NSMDataPaket::PORT);
  g_dstore.open(maxmem);
  LogFile::info("Opend NSMData with %u of maximum memories", maxmem);
  g_dstore.init();

  typedef std::map<std::string, SharedMemory> MemoryList;
  MemoryList mem_m;
  while (true) {
    NSMDataPaket paket;
    memset(&paket, 0, sizeof(paket));
    socket.read((char*)&paket, sizeof(paket));
    std::string remotehost = socket.getRemoteHostName();
    unsigned int addr = socket.getRemoteAddress();
    //if (g_hostname == remotehost) continue;
    if (paket.hdr.flag == NSMCommand::NSMSET.getId()) {
      NSMDataStore::Entry* en = g_dstore.get(addr, paket.hdr.id);
      if (en == NULL) {
        StringList str_v = StringUtil::split(paket.buf, '\n');
        const std::string name = str_v[0];
        const std::string format = str_v[1];
        LogFile::info("Accepted UDP SET requst from:%s. Creating new memory %s:%s:%d",
                      socket.getRemoteHostName().c_str(),
                      name.c_str(), format.c_str(), (int)paket.hdr.revision);
        en = g_dstore.add(addr, paket.hdr.max, paket.hdr.revision,
                          name, format, paket.hdr.id);
        SharedMemory cmem;
        std::string path = StringUtil::form("%s:%s", remotehost.c_str(),
                                            name.c_str());
        cmem.open(path, paket.hdr.max);
        cmem.map();
        mem_m.insert(MemoryList::value_type(name, cmem));
      } else {
        en->size = paket.hdr.max;
        if (mem_m.find(en->name) == mem_m.end()) {
          SharedMemory cmem;
          std::string path = StringUtil::form("%s:%s", remotehost.c_str(),
                                              en->name);
          cmem.open(path, en->size);
          cmem.map();
          mem_m.insert(MemoryList::value_type(en->name, cmem));
        }
        SharedMemory& mem(mem_m[en->name]);
        char* buf = (char*)mem.map();
        memcpy((buf + paket.hdr.offset), paket.buf, paket.hdr.size);
        g_dstore.lock();
        if (paket.hdr.size + paket.hdr.offset == paket.hdr.max) {
          en->utime = Time().getSecond();
          g_dstore.signal();
        }
        g_dstore.unlock();
      }
    } else if (paket.hdr.flag == NSMCommand::NSMGET.getId()) {
      if (paket.hdr.id == 0) {
        NSMDataStore::Entry* en = g_dstore.get(paket.buf);
        if (en != NULL) {
          LogFile::info("Accepted UDP GET requst from %s for %s (id=%d)",
                        socket.getRemoteHostName().c_str(),
                        en->name, (int)en->id);
          std::string str = StringUtil::form("%s\n%s", en->name, en->format);
          paket.hdr.flag = NSMCommand::NSMSET.getId();
          paket.hdr.max = en->size;
          paket.hdr.id = en->id;
          memset(paket.buf, 0, NSMDataPaket::BUFFER_MAX);
          strcpy(paket.buf, str.c_str());
          paket.hdr.size = str.size() + 1;
          UDPSocket socket_r(NSMDataPaket::PORT, remotehost);
          socket_r.write(&paket, sizeof(NSMDataPaket::Header) + paket.hdr.size);
          socket_r.close();
        }
      } else {
        NSMDataStore::Entry* en = g_dstore.get(paket.hdr.id);
        if (en != NULL) {
          if (mem_m.find(en->name) == mem_m.end()) {
            SharedMemory cmem;
            std::string path = StringUtil::form("127.0.0.1:%s", en->name);
            cmem.open(path, en->size);
            cmem.map();
            mem_m.insert(MemoryList::value_type(en->name, cmem));
          }
          SharedMemory& mem(mem_m[en->name]);
          paket.hdr.flag = NSMCommand::NSMSET.getId();
          paket.hdr.offset = 0;
          paket.hdr.max = en->size;
          paket.hdr.paketid = 0;
          UDPSocket socket_r(NSMDataPaket::PORT, remotehost);
          char* buf = (char*)mem.map();
          while (paket.hdr.offset < paket.hdr.max) {
            int size = (paket.hdr.max - paket.hdr.offset);
            paket.hdr.size = (size < NSMDataPaket::BUFFER_MAX) ? size : NSMDataPaket::BUFFER_MAX;
            memcpy(paket.buf, (buf + paket.hdr.offset), paket.hdr.size);
            socket_r.write(&paket, sizeof(NSMDataPaket::Header) + paket.hdr.size);
            paket.hdr.offset += paket.hdr.size;
            ++paket.hdr.paketid;
          }
          socket_r.close();
          //} else {
          //LogFile::debug("no data for %d", paket.hdr.id);
        }
      }
    }
  }

  return 0;
}
