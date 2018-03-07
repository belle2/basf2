//+
// File : DqmMemFile.cc
// Description : Basic tools to pass ROOT histograms over shared memory
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Jan - 2016
//-

#include "daq/dqm/DqmMemFile.h"
#include <framework/pcore/MsgHandler.h>

using namespace Belle2;
using namespace std;

// Constructor
DqmMemFile::DqmMemFile(string name, string mode, int size)
{
  // Record parameters
  m_size = size;
  if (mode != "write" && mode != "WRITE")
    m_mode = 0;
  else
    m_mode = 1;
  m_name = name;
  m_memfile = NULL;

  // Allocate memory space for TMemFile
  m_buf = (char*) new int[size];

  // Allocate shared memory
  m_shm = new SharedMem((char*)name.c_str(), size);

  // Open TMemFile if write mode selected
  if (m_mode == 1) {
    m_memfile = new TMemFile(name.c_str(), m_buf, size * sizeof(int), "RECREATE");
    printf("DqmMemFile : TMemFile is opened in WRITE mode.\n");
  } else
    printf("DqmMemFile : TMemFile is opend in READ mode.\n");
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
  if (m_mode == 0) return -1;
  m_memfile->Write(0, TObject::kOverwrite);
  m_shm->lock();
  m_memfile->CopyTo((char*)(m_shm->ptr()), m_memfile->GetSize());
  m_shm->unlock();
  return 0;
}

TMemFile* DqmMemFile::LoadMemFile()
{
  if (m_mode == 1) return NULL;

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
  int nkeys = 0;
  int nobjs = 0;
  while ((key = (TKey*)nextkey())) {
    nkeys++;
    TObject* obj = curdir->FindObjectAny(key->GetName());
    if (obj->IsA()->InheritsFrom("TH1")) {
      TH1* h1 = (TH1*) obj;
      //      printf ( "Key = %s, entry = %f\n", key->GetName(), h1->GetEntries() );
      if (h1->GetEntries() > 0) {    // Do not send empty histograms
        msg->add(h1, h1->GetName());
        nobjs++;
        numobjs++;
      }
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







