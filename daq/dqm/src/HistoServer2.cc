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

int HistoServer2:: init()
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
          int is = sio.get(fd, buffer, c_maxBufSize);
          if (is <= 0) {
            now = time(0);
            strftime(mbstr, sizeof(mbstr), "%F %T", localtime(&now));
            printf("[%s] HistoServer2: fd %d disconnected\n", mbstr, fd);
            m_man->remove(fd);
            break;
          }
          updated = true;
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
          printf("[%s] HistoServer2 : received nobjs = %d\n", mbstr, nobjs);
          for (int i = 0; i < nobjs; i++) {
            //      printf ( "Object : %s received, class = %s\n", (strlist.at(i)).c_str(),
            //                     (objlist.at(i))->ClassName() );
            string objname = strlist.at(i);
            if (objname == string("DQMRC:CLEAR")) {
              m_hman->clear();
              m_hman->merge();
              now = time(0);
              strftime(mbstr, sizeof(mbstr), "%F %T", localtime(&now));
              printf("[%s] HistoServer2: CLEAR\n", mbstr);
              updated = false;
              continue;
            }
            if (objname == string("DQMRC:MERGE")) {
              m_hman->merge();
              now = time(0);
              strftime(mbstr, sizeof(mbstr), "%F %T", localtime(&now));
              printf("[%s] HistoServer2: MERGE\n", mbstr);
              updated = false;
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
            } else {
              m_hman->update(subdir, strlist.at(i), fd, (TH1*)objlist.at(i));
            }
          }
        }
      }
    }
    usleep(1000);
    loop_counter++;
    if (loop_counter % c_mergeIntervall == 0 && updated) {
      now = time(0);
      strftime(mbstr, sizeof(mbstr), "%F %T", localtime(&now));
      printf("[%s] HistoServer2: merging histograms\n", mbstr);
      m_hman->merge();
      updated = false;
    }
  }
  return 0;
}


