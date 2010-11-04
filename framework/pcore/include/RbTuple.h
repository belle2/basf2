#ifndef RBTUPLE_H
#define RBTUPLE_H
//+
// File : RbTuple.h
// Description : Collect ROOT histogram/Ntuples from event processes
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 16 - Apr - 2009
// Modified : 27 - Jul - 2010, for pbasf2
//-

#include <vector>

#include "TChain.h"
#include "TFile.h"
#include "TH1.h"
#include "TTree.h"
#include "TKey.h"
#include "Riostream.h"

#include <framework/core/Module.h>
#include <framework/pcore/HistoModule.h>

namespace Belle2 {
  /*! Class to manage histograms defined in registered modules */
  /*
     Modules with histogram definitions are supposed to be derived from
     HistoModule class and histograms are defined in HistoModule::defineHisto() function.
     It is supposed to be hooked by register_module(*this) function.
  */
  class RbTupleManager {
  public:

    /*! Access to singleton */
    static RbTupleManager& Instance(void);

    /*! Global initialization */
    void init(int nprocess, const char* filename);

    /*! Functions called by analysis modules in mother process */
    //  void define ( void (*func)( void ) );
    void register_module(Module*);


    /*! Function called by HistoManager module for the first event */
    int begin(int pid);

    /*! Function called by HistoManager module at the end */
    int terminate(void);

    // Functions to collect histograms from event processing on the fly
    //  int collect ();

    /*! Functions to add up all histogram files */
    int hadd(void);

  private:
    // Constructor and destructor are private for singleton access
    RbTupleManager(void);
    ~RbTupleManager(void);


    void MergeRootfile(TDirectory*, TList*);

  private:
    static RbTupleManager* m_instance;

    std::vector<Module*> m_histdefs;
    int m_nproc;
    char m_filename[1024];
    TFile* m_root;
  };

} // Belle2 namespace
#endif
