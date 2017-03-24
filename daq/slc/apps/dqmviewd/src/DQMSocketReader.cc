#include <daq/slc/apps/dqmviewd/DQMSocketReader.h>

#include <daq/slc/apps/dqmviewd/DQMViewCallback.h>

#include <daq/slc/system/TCPServerSocket.h>
#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>

#include <map>
#include <cstdlib>

using namespace Belle2;

DQMSocketReader::DQMSocketReader(int port, DQMViewCallback* callback)
{
  m_port = port;
  m_callback = callback;
}

DQMSocketReader::~DQMSocketReader()
{
}

// Initialize socket

int DQMSocketReader:: init()
{
}

void DQMSocketReader::run()
{
  TCPServerSocket server("0.0.0.0", m_port);
  server.open();
  char* buf = new char[MAXBUFSIZE];
  while (true) {
    TCPSocket socket;
    MsgHandler handler(0);
    bool configured = false;
    std::map<std::string, TH1*> hist_m;
    try {
      socket = server.accept();
      LogFile::info("Accepted new connection for data");
    } catch (const IOException& e) {
      LogFile::fatal(e.what());
      exit(1);
    }
    bool newconf = false;
    try {
      while (true) {
        int size;
        socket.read(&size, sizeof(int));
        size = ntohl(size);
        //LogFile::info("size=%d", size);
        socket.read(buf, size);
        EvtMessage hmsg(buf);
        std::vector<TObject*> objlist;
        std::vector<std::string> strlist;
        handler.decode_msg(&hmsg, objlist, strlist);
        int nobjs = (hmsg.header())->reserved[1];
        int narys = (hmsg.header())->reserved[2];
        std::string dir = "";
        int nhist = 0;
        std::vector<TH1*> hists;
        for (int i = 0; i < nobjs; i++) {
          std::string objname = strlist.at(i);
          int lpos = objname.find_first_not_of("SUBDIR:", 0);
          if (lpos != 0) {
            dir = StringUtil::replace(objname, "SUBDIR:", "");//objname.substr(lpos-1);
            //LogFile::info("new dir: " + dir);
            if (dir == "EXIT") dir = "";
          } else {
            TObject* obj = objlist.at(i);
            TString class_name = obj->ClassName();
            if (class_name.Contains("TH1") ||  class_name.Contains("TH2")) {
              TH1* h = (TH1*)obj;
              std::string name = h->GetName();
              if (dir.size() > 0) name = dir + "/" + name;
              if (hist_m.find(name) == hist_m.end()) {
                std::string vname = StringUtil::form("hist[%d].", nhist);
                LogFile::info("new hist: " + name);
                m_callback->add(new NSMVHandlerText(vname + "path", true, false, name));
                hist_m.insert(std::pair<std::string, TH1*>(name, h));
                nhist++;
                newconf = true;
              } else {
                hist_m[name] = h;
                nhist++;
              }
              h->SetName(name.c_str());
              hists.push_back(h);
            }
          }
        }
        if (!configured) {
          m_callback->add(new NSMVHandlerInt("nhists", true, false, nhist));
        } else {
          m_callback->set("nhists", nhist);
        }
        std::vector<HistSender>& senders(m_callback->getSenders());
        std::vector<HistSender>::iterator it = senders.begin();
        while (it != senders.end()) {
          if (!it->update(hists, newconf)) {
            it = senders.erase(it);
          } else {
            it++;
          }
        }
        newconf = false;
      }
    } catch (const std::exception& e) {
      LogFile::error(e.what());
    }
    socket.close();
  }
  return ;
}


