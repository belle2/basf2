/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "daq/dqm/DqmMemFile.h"
#include <framework/pcore/MsgHandler.h>

#include <TH1.h>
#include <TKey.h>
#include <TText.h>

using namespace Belle2;
using namespace std;

// Constructor
DqmMemFile::DqmMemFile(string name, const string& mode, int size)
{
  // Record parameters
  m_size = size;
  if (mode != "write" && mode != "WRITE")
    m_writeMode = false;
  else
    m_writeMode = true;
  m_name = name;
  m_memfile = NULL;

  // Allocate memory space for TMemFile
  m_buf = (char*) new int[size];

  // Allocate shared memory
  m_shm = new DqmSharedMem((char*)name.c_str(), size, m_writeMode);

  // Clear/Open TMemFile if write mode selected
  // it will check write mode by itself!
  if (ClearSharedMem() == 0) {
    printf("DqmMemFile : TMemFile is opened in WRITE mode.\n");
  } else {
    printf("DqmMemFile : TMemFile is opend in READ mode.\n");
  }
}

DqmMemFile::DqmMemFile(int shm_id, int sem_id, const string& mode, int size)
{
  // Record parameters
  m_size = size;
  if (mode != "write" && mode != "WRITE")
    m_writeMode = false;
  else
    m_writeMode = true;
  m_name = "dqm_mem_file";
  m_memfile = NULL;

  // Allocate memory space for TMemFile
  m_buf = (char*) new int[size];

  // Allocate shared memory
  m_shm = new DqmSharedMem(shm_id, sem_id, size);// size is not used in here
  // Clear/Open TMemFile if write mode selected
  // it will check write mode by itself!
  if (ClearSharedMem() == 0) {
    printf("DqmMemFile : TMemFile is opened in WRITE mode.\n");
  } else {
    printf("DqmMemFile : TMemFile is opend in READ mode.\n");
  }
}

// Destructor
DqmMemFile::~DqmMemFile()
{
  if (m_memfile != NULL)
    delete m_memfile;
  delete m_shm;
  delete m_buf;
}

// Returns pointer to TMemFile
TMemFile* DqmMemFile::GetMemFile()
{
  return m_memfile;
}

// Copy TMemFile contents to Shared Memory
int DqmMemFile::UpdateSharedMem()
{
  if (!m_writeMode) return -1;
  m_memfile->Write(0, TObject::kOverwrite);
  m_shm->lock();
  m_memfile->CopyTo((char*)(m_shm->ptr()), m_memfile->GetSize());
  m_shm->unlock();
  return 0;
}

int DqmMemFile::ClearSharedMem()
{
  if (!m_writeMode) return -1;

  if (m_memfile != NULL) delete m_memfile;
  m_memfile = new TMemFile(m_name.c_str(), m_buf, m_size * sizeof(int), "RECREATE");

  m_shm->lock();
  m_memfile->CopyTo((char*)(m_shm->ptr()), m_memfile->GetSize());
  m_shm->unlock();

  return 0;
}

TMemFile* DqmMemFile::LoadMemFile()
{
  if (m_writeMode) return NULL;

  if (m_memfile != NULL) {
    delete m_memfile;
  }

  m_shm->lock();
  memcpy(m_buf, m_shm->ptr(), m_size * sizeof(int));
  m_shm->unlock();
  //  m_memfile = new TMemFile ( m_name.c_str(), m_buf, m_size*sizeof(int), "RECREATE" );
  m_memfile = new TMemFile(m_name.c_str(), m_buf, MEMFILESIZE);
  return m_memfile;
}

// Copy Shared Memory to local and stream
EvtMessage* DqmMemFile::StreamMemFile()
{
  TMemFile* memfile = LoadMemFile();
  if (memfile == NULL) return NULL;
  //  memfile->ls();
  //  memfile->Print();
  memfile->cd();
  //  gDirectory->ls();
  //  TList* keylist = memfile->GetListOfKeys();
  //  keylist->ls();
  MsgHandler hdl(0);
  int numobjs = 0;
  //  StreamHistograms ( memfile->GetDirectory(NULL), &hdl, numobjs );
  StreamHistograms(gDirectory, &hdl, numobjs);
  //  printf ( "DqmMemFile::StreamMemFile : streamed %d histograms in EvtMessage\n", numobjs );
  EvtMessage* msg = hdl.encode_msg(MSG_EVENT);
  (msg->header())->reserved[0] = 0;
  (msg->header())->reserved[1] = numobjs;
  return msg;
}

int DqmMemFile::StreamHistograms(TDirectory* curdir, MsgHandler* msg, int& numobjs)
{
  TList* keylist = curdir->GetListOfKeys();
  //  keylist->ls();

  TIter nextkey(keylist);
  TKey* key = 0;
  int nkeys [[maybe_unused]] = 0;
  int nobjs [[maybe_unused]] = 0;
  while ((key = (TKey*)nextkey())) {
    nkeys++;
    TObject* obj = curdir->FindObjectAny(key->GetName());
    if (obj->IsA()->InheritsFrom("TH1")) {
      TH1* h1 = (TH1*) obj;
      //      printf ( "Key = %s, entry = %f\n", key->GetName(), h1->GetEntries() );
      msg->add(h1, h1->GetName());
      nobjs++;
      numobjs++;
    } else if (obj->IsA()->InheritsFrom(TDirectory::Class())) {
      //      printf ( "New directory found  %s, Go into subdir\n", obj->GetName() );
      TDirectory* tdir = (TDirectory*) obj;
      //      m_msg->add(tdir, tdir->GetName());
      TText subdir(0, 0, tdir->GetName());
      msg->add(&subdir, "SUBDIR:" + string(obj->GetName())) ;
      nobjs++;
      numobjs++;
      tdir->cd();
      StreamHistograms(tdir, msg, numobjs);
      TText command(0, 0, "COMMAND:EXIT");
      msg->add(&command, "SUBDIR:EXIT");
      nobjs++;
      numobjs++;
      curdir->cd();
    }
  }
  return 0;
}







