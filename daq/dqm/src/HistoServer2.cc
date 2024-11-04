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
#include <arpa/inet.h>

using namespace Belle2;
using namespace std;

// Constructor / Destructor
HistoServer2::HistoServer2(int port, const string& filename)
{
  m_port = port;
  m_force_exit = 0;
  m_filename = filename;
}

HistoServer2::~HistoServer2()
{
  delete m_sock;
}

// Initialize socket

int HistoServer2::init()
{
  m_sock = new EvtSocketRecv(m_port, false);
  m_man = new EvtSocketManager(m_sock);
  m_hman = new HistoManager2(m_filename);
  return 0;

}
// Server function to collect histograms

int HistoServer2::server()
{
  SocketIO sio;
  MsgHandler msghdl(0);
  char mbstr[100];
  time_t now;
  char* buffer = new char[c_maxBufSize];
  //  vector<int> recvsock;
  int loop_counter = 0;
  bool updated = false;
  write_state();
  while (m_force_exit == 0) {
    fflush(stdout);
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
          m_unit_last_conn_time[nr] = now;

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
          //    printf ( "EvtMessage received : size = %d from fd=%d\n", is, fd );

          EvtMessage* hmsg = new EvtMessage(buffer);
          vector<TObject*> objlist;
          vector<string> strlist;
          msghdl.decode_msg(hmsg, objlist, strlist);
          int nobjs = (hmsg->header())->reserved[1];
          //    string subdir = "ROOT";
          string subdir = "";
          now = time(0);
          strftime(mbstr, sizeof(mbstr), "%F %T", localtime(&now));
          m_unit_last_packet_time[nr] = now;
          printf("[%s] HistoServer2 : received nobjs = %d from %s\n", mbstr, nobjs, address);
          if (nobjs > 0) m_unit_last_content_time[nr] = now;
          for (int i = 0; i < nobjs; i++) {
            //      printf ( "Object : %s received, class = %s\n", (strlist.at(i)).c_str(),
            //                     (objlist.at(i))->ClassName() );
            string objname = strlist.at(i);
            if (objname == string("DQMRC:CLEAR")) {
              m_hman->clear();
              m_hman->merge();
              updated = false; // have merged, thus reset updated
              m_last_merge_time = time(0);
              strftime(mbstr, sizeof(mbstr), "%F %T", localtime(&m_last_merge_time));
              printf("[%s] HistoServer2: CLEAR\n", mbstr);
              continue;
            }
            if (objname == string("DQMRC:MERGE")) {
              m_hman->merge();
              updated = false; // have merged, thus reset updated
              m_last_merge_time = time(0);
              strftime(mbstr, sizeof(mbstr), "%F %T", localtime(&m_last_merge_time));
              printf("[%s] HistoServer2: MERGE\n", mbstr);
              continue;
            }
            auto lpos = objname.find("DQMRC:SAVE:");
            if (lpos != string::npos) {
              auto filename = objname.substr(11);
              m_hman->filedump(filename);
              continue;
            }
            lpos = objname.find("SUBDIR:");
            if (lpos != string::npos) {
              subdir = objname.substr(7);
              if (subdir == "EXIT") subdir = "";
              //              printf("HistoServer2 : subdirectory set to %s (%s)\n", subdir.c_str(), objname.c_str());
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
      strftime(mbstr, sizeof(mbstr), "%F %T", localtime(&m_unit_last_conn_time[nr]));
      strftime(mbstr2, sizeof(mbstr2), "%F %T", localtime(&m_unit_last_packet_time[nr]));
      strftime(mbstr3, sizeof(mbstr3), "%F %T", localtime(&m_unit_last_content_time[nr]));
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
