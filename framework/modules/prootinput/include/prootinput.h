//+
// File : prootinput.h
// Description : ROOT I/O event server for pbasf2

#ifndef PROOT_INPUT_H
#define PROOT_INPUT_H

#include <framework/core/Module.h>
#include <framework/core/ModuleManager.h>
#include <framework/core/Framework.h>
#include <framework/pcore/pEventServer.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/pcore/RingBuffer.h>
#include <framework/pcore/RbCtlMgr.h>
#include <framework/pcore/pFramework.h>

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreDefs.h>

#include <TFile.h>
#include <TTree.h>

#define RINGBUF_SIZE 4000000*4     // 4Mwords for ring buffer
#define MAXEVTSIZE   4000000*4     // 4Mwords for maximum event size


namespace Belle2 {
  class pRootInput : public pEventServer {

    // Public functions
  public:

    //! Constructor / Destructor
    pRootInput();
    virtual ~pRootInput();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    //! Module function to be called from event server process
    virtual void event_server();

    // Private functions
  private:
    //! gives back Null, if the branch isn't good for read out.
    TBranch* validBranch(int& ibranch, TObjArray* branches);

    //! Some TFile setup.
    void setupTFile();

    //! Function that actually performs the reading from tree into m_objects.
    int readTree(const EDurability& durability);

    //! Function to read ringbuffer and place objects in DataStore
    int readRingBuf(const EDurability& durability);


    // Data members
  private:
    //! Steering parameters

    //! File name
    std::string m_inputFileName;

    //! Event Number.
    int m_eventNumber;

    //! String vector with steering parameter Names for m_treeNames.
    std::vector<std::string>  m_steerTreeNames;

    //! String vector with steering parameter Names for m_branchNames.
    std::vector<std::string>  m_steerBranchNames;

    //! Compression level
    int m_complevel;

    //! Internal variables

    //! TFile for input.
    TFile* m_file;

    //!  TTree for input.
    TTree* m_tree[c_NDurabilityTypes];


    //! Namess of TTrees and Branches
    std::string m_treeNames[c_NDurabilityTypes];
    std::vector<std::string> m_branchNames[c_NDurabilityTypes];

    //! List of objects in TTree
    std::vector<std::string> m_objnames[c_NDurabilityTypes];
    std::vector<TObject*> m_objects[c_NDurabilityTypes];

    //! List of arrays in TTree
    std::vector<std::string> m_arraynames[c_NDurabilityTypes];
    std::vector<TClonesArray*> m_arrays[c_NDurabilityTypes];

    //! Paralle processing parameters

    //! Nr. of processes
    int m_nproc;

    //! RingBuffer ID
    RingBuffer* m_rbuf;

    //! Total nr. of events in the file
    int m_nevt;

    //! Messaage handler
    MsgHandler* m_msghandler;

    //! Process Control Manager
    RbCtlMgr* m_rbctl;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
