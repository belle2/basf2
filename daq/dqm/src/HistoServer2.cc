/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/dqm/HistoServer2.h>

#include <framework/pcore/MsgHandler.h>
#include <ctime>
#include <unistd.h>
#include <arpa/inet.h>

#include <TText.h>

using namespace Belle2;
using namespace std;

// Constructor / Destructor
HistoServer2::HistoServer2(int port, const string& filename)
{
  TMessage::EnableSchemaEvolutionForAll(kTRUE);
  m_port = port;
  m_force_exit = 0;
  m_filename = filename;
}

HistoServer2::~HistoServer2()
{
  if (m_root_socket) {
    m_root_socket->Close();
    delete m_root_socket;
    m_root_socket = nullptr;
  }

  if (m_root_server_sock) {
    m_root_server_sock->Close();
    delete m_root_server_sock;
    m_root_server_sock = nullptr;
  }

  delete m_sock;
}

// Initialize socket

int HistoServer2::init()
{
  m_sock = new EvtSocketRecv(m_port, false);
  m_man = new EvtSocketManager(m_sock);
  m_hman = new HistoManager2(m_filename);

  // ROOT socket
  int rootPort = m_port + 1;
  m_root_server_sock = new TServerSocket(rootPort, kTRUE);
  printf("before ROOT socket connection\n");
  if (!m_root_server_sock->IsValid()) {
    return -1;
  }
  m_root_server_sock->SetOption(kNoBlock, 1);
  printf("ROOT socket connected\n");

  return 0;

}
// Server function to collect histograms

int HistoServer2::server()
{
  SocketIO sio;
  MsgHandler msghdl(0);
  char mbstr[100];
  time_t now = time(0);
  char* buffer = new char[c_maxBufSize];
  //  vector<int> recvsock;
  int loop_counter = 0;
  bool updated = false;
  write_state();
  while (m_force_exit == 0) {
    fflush(stdout);
    {
      vector<TObject*> objlist;
      vector<string> strlist;
      int root_fd = -1;
      if (recv_root_message(objlist, strlist, root_fd)) {
        printf("HistoServer2: ROOT received nobjs=%zu\n", objlist.size());
        handle_objects(objlist, strlist, root_fd, updated);
      }
      // Objects were cloned in recvRootMessage().
      for (auto* obj : objlist) {
        delete obj;
      }
    }
    int exam_stat = m_man->examine();
    if (exam_stat == 0) {
    } else if (exam_stat == 1) { //
      // printf ( "Histo data ready on socket\n" );
      vector<int>& recvsock = m_man->connected_socket_list();
      for (vector<int>::iterator it = recvsock.begin();
           it != recvsock.end(); ++it) {
        int fd = *it;
        if (m_man->connected(fd)) {
          struct sockaddr_in isa;
          socklen_t isize = sizeof(isa);
          getpeername(fd, (struct sockaddr*)&isa, &isize);
          char address[INET_ADDRSTRLEN];
          strcpy(address, inet_ntoa(isa.sin_addr));
          char* ptr = strrchr(address, '.');
          int nr = -1;
          if (ptr) {
            nr = atoi(ptr + 1);
          }
          now = time(0);
          m_unit_last_conn_time[address] = now;

          int is = sio.get(fd, buffer, c_maxBufSize);
          if (is <= 0) {
            now = time(0);
            strftime(mbstr, sizeof(mbstr), "%F %T", localtime(&now));
            printf("[%s] HistoServer2: fd %d / %s disconnected\n", mbstr, fd, address);
            m_man->remove(fd);
            m_units_connected[address] = std::pair(nr, false);
            break;
          }
          m_units_connected[address] = std::pair(nr, true);
          // printf ( "EvtMessage received : size = %d from fd=%d\n", is, fd );

          EvtMessage* hmsg = new EvtMessage(buffer);
          vector<TObject*> objlist;
          vector<string> strlist;
          msghdl.decode_msg(hmsg, objlist, strlist);
          int nobjs = (hmsg->header())->reserved[1];
          // string subdir = "ROOT";
          string subdir = "";
          now = time(0);
          strftime(mbstr, sizeof(mbstr), "%F %T", localtime(&now));
          m_unit_last_packet_time[address] = now;
          printf("[%s] HistoServer2 : received nobjs = %d from %s\n", mbstr, nobjs, address);
          if (nobjs > 0) m_unit_last_content_time[address] = now;
          for (int i = 0; i < nobjs; i++) {
            // printf ( "Object : %s received, class = %s\n", (strlist.at(i)).c_str(),
            // (objlist.at(i))->ClassName() );
            string objname = strlist.at(i);
            auto lpos = objname.find("SUBDIR:");
            if (lpos != string::npos) {
              subdir = objname.substr(7);
              if (subdir == "EXIT") subdir = "";
              // printf("HistoServer2 : subdirectory set to %s (%s)\n", subdir.c_str(), objname.c_str());
              // no update to histograms ...
            } else {
              m_hman->update(subdir, strlist.at(i), fd, (TH1*)objlist.at(i));
              updated = true; // histograms have been updated
            }
          }
        }
      }
    }
    usleep(1000);
    loop_counter++;
    if (loop_counter % c_mergeIntervall == 0) {
      if (updated) {
        m_last_merge_time = time(0);
        strftime(mbstr, sizeof(mbstr), "%F %T", localtime(&m_last_merge_time));
        printf("[%s] HistoServer2: merging histograms\n", mbstr);
        m_hman->merge();
        updated = false; // have merged, thus reset updated
      }
      write_state();
    }
  }
  return 0;
}

void HistoServer2::write_state(void)
{
  char mbstr[100];
  char mbstr2[100];
  char mbstr3[100];
  std::string name = "/tmp/dqm_hserver_state_" + m_filename;
  FILE* fh = fopen(name.c_str(), "wt+");
  if (fh) {
    time_t now = time(0);
    strftime(mbstr, sizeof(mbstr), "%F %T", localtime(&now));
    strftime(mbstr2, sizeof(mbstr2), "%F %T", localtime(&m_last_merge_time));
    fprintf(fh, "%s,%s,%s\n", m_filename.c_str(), mbstr, mbstr2);

    for (auto& it : m_units_connected) {
      int nr = it.second.first;
      int con = it.second.second;
      strftime(mbstr, sizeof(mbstr), "%F %T", localtime(&m_unit_last_conn_time[it.first]));
      strftime(mbstr2, sizeof(mbstr2), "%F %T", localtime(&m_unit_last_packet_time[it.first]));
      strftime(mbstr3, sizeof(mbstr3), "%F %T", localtime(&m_unit_last_content_time[it.first]));
      if (it.first == "127.0.0.1") {
        fprintf(fh, "RUNCONTROL,");
      } else {
        if (nr >= 0 and nr < 20) {
          fprintf(fh, "HLT%d,", nr);
        } else if (nr > 100 and nr < 110) {
          fprintf(fh, "ERECO%d,", nr - 100);
        } else {
          fprintf(fh, "UNKNOWN,");
        }
      }
      fprintf(fh, "%s,%d,%s,%s,%s\n", it.first.c_str(), con, mbstr, mbstr2, mbstr3);
    }
    fclose(fh);
  }
}

bool HistoServer2::recv_root_message(
  vector<TObject*>& objlist,
  vector<string>& strlist,
  int& fd)
{
  fd = -1;

  // No ROOT client connected yet.
  // Try to accept one.

  if (!m_root_socket) {

    TSocket* sock = m_root_server_sock->Accept();

    if (!sock || sock == reinterpret_cast<TSocket*>(-1)) {
      return false;
    }

    if (!sock->IsValid()) {
      delete sock;
      return false;
    }

    m_root_socket = sock;

    printf("HistoServer2: ROOT client connected fd=%d\n",
           m_root_socket->GetDescriptor());

    time_t now = time(0);

    m_units_connected["127.0.0.1"] = std::pair(0, true);
    m_unit_last_conn_time["127.0.0.1"] = now;

    return false;
  }

  int ready = m_root_socket->Select(TSocket::kRead, 0);

  if (ready == 0) {
    return false;
  }

  if (ready < 0) {
    printf("HistoServer2: ROOT Select failed fd=%d ready=%d\n",
           m_root_socket->GetDescriptor(), ready);

    m_root_socket->Close();
    delete m_root_socket;
    m_root_socket = nullptr;

    return false;
  }

  TMessage* msg = nullptr;

  int recv_size = m_root_socket->Recv(msg);

  printf("HistoServer2: ROOT Recv fd=%d ret=%d msg=%p\n",
         m_root_socket->GetDescriptor(),
         recv_size,
         static_cast<void*>(msg));

  if (recv_size <= 0 || !msg) {

    printf("HistoServer2: ROOT client disconnected/error fd=%d ret=%d\n",
           m_root_socket->GetDescriptor(),
           recv_size);

    m_units_connected["127.0.0.1"] = std::pair(0, false);

    delete msg;

    m_root_socket->Close();
    delete m_root_socket;
    m_root_socket = nullptr;

    return false;
  }

  time_t now = time(0);
  m_unit_last_packet_time["127.0.0.1"] = now;

  printf("HistoServer2: ROOT message fd=%d what=%d len=%d\n",
         m_root_socket->GetDescriptor(),
         msg->What(),
         msg->Length());

  if (msg->What() != kMESS_OBJECT) {

    printf("HistoServer2: unexpected ROOT message type %d\n",
           msg->What());

    delete msg;
    return false;
  }

  TObject* received = msg->ReadObject(msg->GetClass());

  TList* list = dynamic_cast<TList*>(received);

  if (!list) {

    if (received) {
      printf("HistoServer2: expected TList, got %s\n",
             received->ClassName());
    }

    delete received;
    delete msg;

    return false;
  }

  // Convert TList into the same form as decode_msg():
  //
  //   vector<TObject*>
  //   vector<string>

  TIter next(list);
  TObject* obj = nullptr;

  while ((obj = next())) {

    TObject* copy = obj->Clone();

    if (!copy) continue;

    if (auto* hist = dynamic_cast<TH1*>(copy); hist != nullptr) {

      hist->SetDirectory(nullptr);

      objlist.push_back(hist);
      strlist.push_back(hist->GetName());

    } else if (auto* text = dynamic_cast<TText*>(copy); text != nullptr) {

      objlist.push_back(text);
      strlist.push_back(text->GetTitle());

    } else {

      objlist.push_back(copy);
      strlist.push_back(copy->GetName());
    }
  }

  now = time(0);

  m_unit_last_packet_time["127.0.0.1"] = now;

  if (!objlist.empty()) {
    m_unit_last_content_time["127.0.0.1"] = now;
  }

  delete list;
  delete msg;

  return !objlist.empty();
}

void HistoServer2::handle_objects(
  vector<TObject*>& objlist,
  vector<string>& strlist,
  int fd,
  bool& updated)
{
  string subdir = "";

  int nobjs = objlist.size();

  for (int i = 0; i < nobjs; i++) {
    string objname = strlist.at(i);

    if (objname == "DQMRC:CLEAR") {
      m_hman->clear();
      m_hman->merge();
      updated = false;
      m_last_merge_time = time(0);
      continue;
    }

    if (objname == "DQMRC:MERGE") {
      m_hman->merge();
      updated = false;
      m_last_merge_time = time(0);
      continue;
    }

    auto lpos = objname.find("DQMRC:SAVE:");

    if (lpos != string::npos) {
      auto filename = objname.substr(11);
      m_hman->filedump(filename);
      printf("DQMRC:SAVE? %s\n", filename.c_str());
      continue;
    }

    lpos = objname.find("SUBDIR:");

    if (lpos != string::npos) {
      subdir = objname.substr(7);
      if (subdir == "EXIT")
        subdir = "";
      continue;
    }

    if (objname == "COMMAND:EXIT") {
      subdir = "";
      continue;
    }

    TH1* histo = dynamic_cast<TH1*>(objlist.at(i));

    if (histo != nullptr) {
      m_hman->update(subdir, objname, fd, histo);
      objlist.at(i) = nullptr;
    } else {
      printf("HistoServer2: invalid object '%s', class=%s\n",
             objname.c_str(), objlist.at(i)->ClassName());
      continue;
    }

    updated = true;
  }
}
