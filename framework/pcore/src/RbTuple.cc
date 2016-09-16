//+
// File : rbtuple.cc
// Description : Collect histograms from multiple processes
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 21 - Apr - 2009
//-

#include <framework/core/HistoModule.h>
#include <framework/pcore/RbTuple.h>
#include <framework/pcore/ProcHandler.h>

#include <framework/logging/Logger.h>

#include <TFileMerger.h>
#include <TFile.h>
#include <Riostream.h>

#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

using namespace std;
using namespace Belle2;

RbTupleManager* RbTupleManager::s_instance = 0;

// Constructor and Destructor
RbTupleManager::RbTupleManager()
{
}

RbTupleManager::~RbTupleManager()
{
}

RbTupleManager::RbTupleManager(int nproc, const char* file, const char* workdir)
{
  m_filename = file;
  m_nproc = nproc;
  m_workdir = workdir;
}

// Access to Singleton
RbTupleManager& RbTupleManager::Instance()
{
  if (!s_instance) {
    s_instance = new RbTupleManager;
  }
  return *s_instance;
}

// Global initialization
void RbTupleManager::init(int nprocess, const char* filename, const char* workdir)
{
  m_filename = filename;
  m_nproc = nprocess;
  m_workdir = workdir;

  if (ProcHandler::EvtProcID() == -1 && m_nproc > 0) {

    // should be called only from main
    // Open current directory
    std::string dir = m_workdir;
    DIR* dp;
    struct dirent* dirp;
    if ((dp = opendir(dir.c_str())) == NULL) {
      B2ERROR("Error to open directory" << dir);
      return;
    }

    // Scan the directory and delete temporary files
    std::string compfile = m_filename + ".";
    while ((dirp = readdir(dp)) != NULL) {
      std::string curfile = std::string(dirp->d_name);
      if (curfile.compare(0, compfile.size(), compfile) == 0) {
        //        unlink(curfile.c_str());
        unlink((m_workdir + "/" + curfile).c_str());
      }
    }
    closedir(dp);
    cout << "HistoManager : old temporary histogram files deleted" << endl;
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
    std::string fileNamePlusId = m_workdir + "/" +
                                 m_filename + '.' + std::to_string(procid);
    m_root = new TFile(fileNamePlusId.c_str(), "update");
    //    printf("RbTupleManager: histo file opened for process %d (pid=%d)\n",
    //           procid, getpid());
    B2INFO("RbTupleManager : histo file opened for process " << procid << "(" << getpid() << ")");
  } else {
    m_root = new TFile(m_filename.c_str(), "recreate");
    //    printf("RbTupleManager: initialized for single-process\n");
    B2INFO("RbTupleManager :  initialized for single process");
  }
  if (m_root == NULL) return -1;
  //  printf ( "RbTupleManager::TFile opened\n" );

  for (vector<Module*>::iterator it = m_histdefs.begin(); it != m_histdefs.end(); ++it) {
    HistoModule* hmod = (HistoModule*) * it;
    hmod->defineHisto();
  }

  //  printf ( "RbTupleManager::Histograms defined in proc %d\n", procid );

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

int RbTupleManager::dump()
{
  if (m_root != NULL) {
    m_root->Write();
  }
  return 0;
}

// Functions called from main process
int RbTupleManager::hadd(bool deleteflag)
{
  // No need to call this function when nprocess=0
  if (m_nproc == 0) {
    //    if ( m_root != NULL ) terminate();
    return 0;
  }

  printf("RbTupleManager::hadd started\n");

  // Set up merger with output file
  TFileMerger merger(false, false);
  if (!merger.OutputFile(m_filename.c_str())) {
    B2ERROR("RbTupleManager:: error to open output file " << m_filename);
    return -1;
  }

  // Open current directory
  //  std::string dir = ".";
  std::string dir = m_workdir;
  DIR* dp;
  struct dirent* dirp;
  if ((dp = opendir(dir.c_str())) == NULL) {
    B2ERROR("Error to open directory" << dir);
    return errno;
  }

  std::vector<std::string> filenames;
  // Scan the directory and register all histogram files
  //  std::string compfile = dir + "/" + m_filename + ".";
  std::string compfile = m_filename + ".";
  printf("compfile = %s\n", compfile.c_str());
  while ((dirp = readdir(dp)) != NULL) {
    std::string curfile = std::string(dirp->d_name);
    //    printf("Checking %s with compfile%s\n", curfile.c_str(), compfile.c_str());
    if (curfile.compare(0, compfile.size(), compfile) == 0) {
      printf("RbTupleManager:: adding  file =%s\n", curfile.c_str());
      merger.AddFile((m_workdir + "/" + curfile).c_str());
      filenames.push_back(m_workdir + "/" + curfile);
    }
  }
  closedir(dp);

  // Do Merge
  if (!merger.Merge()) {
    //    printf ( "RbTupleManager:: error to merge files\n" );
    B2ERROR("RbTupleManager:: error to merge files");
    return -1;
  }

  if (deleteflag) {
    // Delete temporary files
    vector<string>::iterator it;
    for (it = filenames.begin(); it != filenames.end(); ++it) {
      string& hfile = *it;
      unlink(hfile.c_str());
    }
  }

  printf("RbTupleManager: histogram files are added\n");
  //  B2INFO ( "RbTupleManager : histogram files are added" );

  return 0;
}
