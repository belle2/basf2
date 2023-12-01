/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <string>

#include <daq/dqm/DqmSharedMem.h>
#include <framework/pcore/EvtMessage.h>

#include "TMemFile.h"
#include "TDirectory.h"

namespace Belle2 {
  class MsgHandler;

  class DqmMemFile {
  public:
    enum {c_memFileSize = 128000000};
  public:
    DqmMemFile(std::string name = "dqmhisto",
               const std::string& mode = "readonly", int size = c_memFileSize);
    DqmMemFile(int shm_id, int sem_id, int size = c_memFileSize);
    ~DqmMemFile();

    // Sender function
    TMemFile* GetMemFile();
    int UpdateSharedMem();
    int ClearSharedMem();
    TMemFile* LoadMemFile();

    // Receiver functions
    EvtMessage* StreamMemFile();

    bool SaveToFile(std::string outfile);

  private:
    int StreamHistograms(TDirectory* curdir, MsgHandler* msg, int& nobjs);

  private:
    DqmSharedMem* m_shm;
    TMemFile* m_memfile;
    char* m_buf;
    std::string m_name;
    int m_size;
    bool m_writeMode;
  };
}

