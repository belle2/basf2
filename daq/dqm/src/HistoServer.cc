//+
// File : HistoServer.cc
// Description : Fetch EvtSocket and receive histograms periodically
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 18 - Dec - 2012
//-
#include <daq/dqm/HistoServer.h>

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
  char* buffer = new char[MAXBUFSIZE];
  //  vector<int> recvsock;
  int loop_counter = 0;
  bool updated = false;
  while (m_force_exit == 0) {
    int exam_stat = m_man->examine();
    if (exam_stat == 0) {
      printf("Initial connection request detected!\n");
      //      int fd = recvsock[;
    } else if (exam_stat == 1) { //
      // printf ( "Histo data ready on socket\n" );
      vector<int>& recvsock = m_man->connected_socket_list();
      for (vector<int>::iterator it = recvsock.begin();
           it != recvsock.end(); ++it) {
        int fd = *it;
        if (m_man->connected(fd)) {
          int is = sio.get(fd, buffer, MAXBUFSIZE);
          if (is <= 0) {
            printf("HistoServer: fd %d disconnected\n", fd);
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
          printf("HistoServer : received nobjs = %d\n", nobjs);
          for (int i = 0; i < nobjs; i++) {
            //      printf ( "Object : %s received, class = %s\n", (strlist.at(i)).c_str(),
            //                     (objlist.at(i))->ClassName() );
            string objname = strlist.at(i);
            int lpos = objname.find_first_not_of("SUBDIR:", 0);
            //      printf ( "lpos = %d\n", lpos );
            //            if (lpos != (int)string::npos) {
            if (lpos > 1) {
              //              subdir = objname.substr(lpos - 1);
              subdir = objname.substr(lpos - 1);
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
      printf("HistoServer: merging histograms\n");
      //      m_mapfile->AcquireSemaphore();
      m_hman->merge();
      //      m_mapfile->ReleaseSemaphore();
      updated = false;
    }
  }
  return 0;
}


