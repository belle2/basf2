/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef DQM_MEMFILE
#define DQM_MEMFILE

#include <string>

#include <daq/rfarm/manager/SharedMem.h>
#include <framework/pcore/EvtMessage.h>

#include "TMemFile.h"
#include "TDirectory.h"

#define MEMFILESIZE 128000000

namespace Belle2 {
  class MsgHandler;

  class DqmMemFile {
  public:
    DqmMemFile(std::string name = "dqmhisto",
               std::string mode = "readonly", int size = MEMFILESIZE);
    DqmMemFile(int shm_id, int sem_id,
               std::string mode = "readonly", int size = MEMFILESIZE);
    ~DqmMemFile();

    // Sender function
    TMemFile* GetMemFile();
    int UpdateSharedMem();
    int ClearSharedMem();
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

