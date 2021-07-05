/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/dqm/HistoServer.h>

#include <framework/pcore/MsgHandler.h>
#include <ctime>

using namespace Belle2;
using namespace std;

// Constructor / Destructor
HistoServer::HistoServer(int port, string filename)
{
  m_port = port;
  m_force_exit = 0;
  m_filename = filename;
}

HistoServer::~HistoServer()
{
  delete m_sock;
}

// Initialize socket

int HistoServer:: init()
{
  m_sock = new EvtSocketRecv(m_port, false);
  m_man = new EvtSocketManager(m_sock);
  //  m_mapfile = TMapFile::Create(m_filename.c_str(), "RECREATE", MAPFILESIZE);
  m_memfile = new DqmMemFile(m_filename, "write", MEMFILESIZE);
  m_hman = new HistoManager(m_memfile);

  // Semaphore to ensure exclusive access to shm
  //  m_mapfile->CreateSemaphore();
  //  m_mapfile->ReleaseSemaphore();
  return 0;

}
// Server function to collect histograms

int HistoServer::server()
{
  SocketIO sio;
  MsgHandler msghdl(0);
  char mbstr[100];
  time_t now;
  char* buffer = new char[MAXBUFSIZE];
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
          int is = sio.get(fd, buffer, MAXBUFSIZE);
          if (is <= 0) {
            now = time(0);
            strftime(mbstr, sizeof(mbstr), "%c", localtime(&now));
            printf("[%s] HistoServer: fd %d disconnected\n", mbstr, fd);
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
          int narys = (hmsg->header())->reserved[2];
          //    string subdir = "ROOT";
          string subdir = "";
          now = time(0);
          strftime(mbstr, sizeof(mbstr), "%c", localtime(&now));
          printf("[%s] HistoServer : received nobjs = %d\n", mbstr, nobjs);
          for (int i = 0; i < nobjs; i++) {
            //      printf ( "Object : %s received, class = %s\n", (strlist.at(i)).c_str(),
            //                     (objlist.at(i))->ClassName() );
            string objname = strlist.at(i);
            if (objname == string("DQMRC:CLEAR")) {
              m_hman->clear();
              m_hman->merge();
              now = time(0);
              strftime(mbstr, sizeof(mbstr), "%c", localtime(&now));
              printf("[%s] HistoServer: CLEAR\n", mbstr);
              updated = false;
              continue;
            }
            if (objname == string("DQMRC:MERGE")) {
              m_hman->merge();
              now = time(0);
              strftime(mbstr, sizeof(mbstr), "%c", localtime(&now));
              printf("[%s] HistoServer: MERGE\n", mbstr);
              updated = false;
              continue;
            }
            int lpos = objname.find("SUBDIR:");
            if (lpos != string::npos) {
              subdir = objname.substr(7);
              if (subdir == "EXIT") subdir = "";
              //              printf("HistoServer : subdirectory set to %s (%s)\n", subdir.c_str(), objname.c_str());
            } else {
              m_hman->update(subdir, strlist.at(i), fd, (TH1*)objlist.at(i));
            }
          }
        }
      }
    }
    usleep(1000);
    loop_counter++;
    if (loop_counter % MERGE_INTERVAL == 0 && updated) {
      now = time(0);
      strftime(mbstr, sizeof(mbstr), "%c", localtime(&now));
      printf("[%s] HistoServer: merging histograms\n", mbstr);
      //      m_mapfile->AcquireSemaphore();
      m_hman->merge();
      //      m_mapfile->ReleaseSemaphore();
      updated = false;
    }
  }
  return 0;
}


