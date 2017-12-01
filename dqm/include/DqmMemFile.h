#ifndef DQM_MEMFILE
#define DQM_MEMFILE
//+
// File : DqmMemFile.h
// Description : Basic tools to pass ROOT histograms over shared memory
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 6 - Jan - 2016
//-

#include <string>

#include <dqm/SharedMem.h>
#include <framework/pcore/EvtMessage.h>

#include "TMemFile.h"
#include "TObject.h"
#include "TText.h"
#include "TChain.h"
#include "TH1.h"
#include "TTree.h"
#include "TKey.h"
#include "TIterator.h"
#include "TDirectory.h"


#define MEMFILESIZE 128000000

namespace Belle2 {
  class MsgHandler;

  class DqmMemFile {
  public:
    DqmMemFile(std::string name = "dqmhisto",
               std::string mode = "readonly", int size = MEMFILESIZE);
    ~DqmMemFile();

    // Sender function
    TMemFile* GetMemFile();
    int UpdateSharedMem();
    TMemFile* LoadMemFile();

    // Receiver functions
    EvtMessage* StreamMemFile();

  private:
    int StreamHistograms(TDirectory* curdir, MsgHandler* msg, int& nobjs);

  private:
    SharedMem* m_shm;
    TMemFile* m_memfile;
    char* m_buf;
    std::string m_name;
    int m_size;
    int m_mode;
  };
}
#endif

