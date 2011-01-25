//+
// File : rbtuple.cc
// Description : Collect histograms from multiple processes
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 21 - Apr - 2009
//-

#include <framework/pcore/RbTuple.h>

#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "TChain.h"
#include "TFile.h"
#include "TH1.h"
#include "TTree.h"
#include "TKey.h"
#include "Riostream.h"

using namespace std;
using namespace Belle2;

RbTupleManager* RbTupleManager::m_instance = 0;

// Constructor and Destructor
RbTupleManager::RbTupleManager()
{
}

RbTupleManager::~RbTupleManager()
{
}

// Access to Singleton
RbTupleManager& RbTupleManager::Instance(void)
{
  if (!m_instance) {
    m_instance = new RbTupleManager;
  }
  return *m_instance;
}

// Global initialization
void RbTupleManager::init(int nprocess, const char* filename)
{
  strcpy(m_filename, filename);
  m_nproc = nprocess;

  // Delete temporary files
  for (int i = 0; i < m_nproc; i++) {
    char filename[1024];
    sprintf(filename, "%s.%d", m_filename, i);
    unlink(filename);
  }
}

// Function to register histogram definitions
void RbTupleManager::register_module(Module* mod)
{
  m_histdefs.push_back(mod);
}

// Function called from event processes
int RbTupleManager::begin(int procid)
{
  if (m_nproc > 0) {
    char filename[1024];
    sprintf(filename, "%s.%d", m_filename, procid);
    m_root = new TFile(filename, "update");
    printf("RbTupleManager: histo file opened for process %d (pid=%d)\n",
           procid, getpid());
  } else {
    m_root = new TFile(m_filename, "recreate");
    printf("RbTupleManager: initialized for single-process\n");
  }
  if (m_root == NULL) return -1;
  //  printf ( "RbTupleManager::TFile opened\n" );

  for (vector<Module*>::iterator it = m_histdefs.begin(); it != m_histdefs.end(); ++it) {
    HistoModule* hmod = (HistoModule*) * it;
    hmod->defineHisto();
  }

  //  printf ( "RbTupleManager::Histograms defined\n" );

  return 0;
}

int RbTupleManager::terminate()
{
  if (m_root != NULL) {
    m_root->Write();
    m_root->Close();
    delete m_root;
    m_root = NULL;
  }
  return 0;
}

// Functions called from main process
int RbTupleManager::hadd(void)
{
  // No need to call this function when nprocess=0
  if (m_nproc == 0) {
    //    if ( m_root != NULL ) terminate();
    return 0;
  }

  // Make a list of ROOT file created by event processes
  TList* filelist = new TList();
  for (int i = 0; i < m_nproc; i++) {
    char filename[1024];
    sprintf(filename, "%s.%d", m_filename, i);
    //    filelist->Add(TFile::Open(filename));
    TFile* tf = new TFile(filename, "READ");
    filelist->Add(tf) ;
  }

  // Open a root file to merge histograms
  TFile* target = new TFile(m_filename, "recreate");

  //  printf ( "Just sleeping before merging..... pid=%d\n", getpid() );
  //  sleep ( 10 );
  MergeRootfile((TDirectory*) target, filelist);

  // Delete temporary files
  for (int i = 0; i < m_nproc; i++) {
    char filename[1024];
    sprintf(filename, "%s.%d", m_filename, i);
    unlink(filename);
  }
  printf("RbTupleManager: histogram files are added\n");

  return 0;
}



// Body of root file merger grabbed from "hadd"

void RbTupleManager::MergeRootfile(TDirectory *target, TList *sourcelist)
{

  //  cout << "Target path: " << target->GetPath() << endl;
  TString path((char*)strstr(target->GetPath(), ":"));
  path.Remove(0, 2);

  TFile *first_source = (TFile*)sourcelist->First();
  first_source->cd(path);
  TDirectory *current_sourcedir = gDirectory;
  //gain time, do not add the objects in the list in memory
  Bool_t status = TH1::AddDirectoryStatus();
  TH1::AddDirectory(kFALSE);

  // loop over all keys in this directory
  TChain *globChain = 0;
  TIter nextkey(current_sourcedir->GetListOfKeys());
  TKey *key, *oldkey = 0;
  while ((key = (TKey*)nextkey())) {

    //keep only the highest cycle number for each key
    if (oldkey && !strcmp(oldkey->GetName(), key->GetName())) continue;

    // read object from first source file
    first_source->cd(path);
    TObject *obj = key->ReadObj();

    if (obj->IsA()->InheritsFrom("TH1")) {
      // descendant of TH1 -> merge it

      //      cout << "Merging histogram " << obj->GetName() << endl;
      TH1 *h1 = (TH1*)obj;

      // loop over all source files and add the content of the
      // correspondant histogram to the one pointed to by "h1"
      TFile *nextsource = (TFile*)sourcelist->After(first_source);
      while (nextsource) {

        // make sure we are at the correct directory level by cd'ing to path
        nextsource->cd(path);
        TKey *key2 = (TKey*)gDirectory->GetListOfKeys()->FindObject(h1->GetName());
        if (key2) {
          TH1 *h2 = (TH1*)key2->ReadObj();
          h1->Add(h2);
          delete h2;
        }

        nextsource = (TFile*)sourcelist->After(nextsource);
      }
    } else if (obj->IsA()->InheritsFrom("TTree")) {

      // loop over all source files create a chain of Trees "globChain"
      const char* obj_name = obj->GetName();

      globChain = new TChain(obj_name);
      globChain->Add(first_source->GetName());
      TFile *nextsource = (TFile*)sourcelist->After(first_source);
      //      const char* file_name = nextsource->GetName();
      // cout << "file name  " << file_name << endl;
      while (nextsource) {

        globChain->Add(nextsource->GetName());
        nextsource = (TFile*)sourcelist->After(nextsource);
      }

    } else if (obj->IsA()->InheritsFrom("TDirectory")) {
      // it's a subdirectory

      cout << "Found subdirectory " << obj->GetName() << endl;

      // create a new subdir of same name and title in the target file
      target->cd();
      TDirectory *newdir = target->mkdir(obj->GetName(), obj->GetTitle());

      // newdir is now the starting point of another round of merging
      // newdir still knows its depth within the target file via
      // GetPath(), so we can still figure out where we are in the recursion
      MergeRootfile(newdir, sourcelist);

    } else {

      // object is of no type that we know or can handle
      cout << "Unknown object type, name: "
           << obj->GetName() << " title: " << obj->GetTitle() << endl;
    }

    // now write the merged histogram (which is "in" obj) to the target file
    // note that this will just store obj in the current directory level,
    // which is not persistent until the complete directory itself is stored
    // by "target->Write()" below
    if (obj) {
      target->cd();

      //!!if the object is a tree, it is stored in globChain...
      if (obj->IsA()->InheritsFrom("TTree"))
        globChain->Merge(target->GetFile(), 0, "keep");
      else
        obj->Write(key->GetName());
    }

  } // while ( ( TKey *key = (TKey*)nextkey() ) )

  // save modifications to target file
  target->SaveSelf(kTRUE);
  TH1::AddDirectory(status);
}



