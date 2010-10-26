//+
// File : prootoutput.h
// Description : ROOT I/O output server for pbasf2
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#ifndef PROOT_OUTPUT_H
#define PROOT_OUTPUT_H

#include <framework/core/Module.h>
#include <framework/core/ModuleManager.h>
#include <framework/core/Framework.h>
#include <framework/pcore/pEventServer.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/pcore/RingBuffer.h>
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
  class pRootOutput : public pOutputServer {

    // Public functions
  public:

    //! Constructor / Destructor
    pRootOutput();
    virtual ~pRootOutput();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    //! Module function to be called from event server process
    virtual void output_server();

  private:
    //! Open a TFile
    void setupTFile();

    //! Initialize TTree by looking at the record
    void setupTTree(const EDurability&);

    //! Fill TTree in single process mode
    void fillTree(const EDurability&);

    //! Fill RingBuffer in multiprocess mode
    void fillRingBuf(const EDurability&);

    //! Get size of object
    size_t getSizeOfObj(const EDurability&);
    //! Get size of array
    size_t getSizeOfArray(const EDurability&);

    // Data members
  private:
    //! File name
    std::string m_outputFileName;

    //! String vector with steering parameter Names for m_treeNames.
    std::vector<std::string>  m_steerTreeNames;

    //! String vector with steering parameter Names for m_branchNames.
    std::vector<std::string>  m_steerBranchNames;

    //! Compression level
    int m_compressionLevel;

    //! DataStore iterators
    StoreIter* m_obj_iter[c_NDurabilityTypes];
    StoreIter* m_array_iter[c_NDurabilityTypes];

    //! TFile for output.
    TFile* m_file;

    //!  TTree for output.
    TTree* m_tree[c_NDurabilityTypes];

    //! Namess of TTrees and Branches
    std::string m_treeNames[c_NDurabilityTypes];
    std::vector<std::string> m_branchNames[c_NDurabilityTypes];

    //! List of objects in TTree
    std::vector<std::string> m_objnames[c_NDurabilityTypes];
    std::vector<TBranch*> m_objbrs[c_NDurabilityTypes];
    std::vector<TObject*> m_objects[c_NDurabilityTypes];

    //! List of arrays in TTree
    std::vector<std::string> m_arraynames[c_NDurabilityTypes];
    std::vector<TBranch*> m_arraybrs[c_NDurabilityTypes];
    std::vector<TClonesArray*> m_arrays[c_NDurabilityTypes];

    //! Has branch creation already happened?
    bool m_done[c_NDurabilityTypes];

    // Parallel processing parameters

    //! Nr. of processes
    int m_nproc;

    //! RingBuffer ID
    RingBuffer* m_rbuf;

    //! Total nr. of events in the file
    int m_nevt;

    //! Messaage handler
    MsgHandler* m_msghandler;

    //! No. of sent events
    int m_nsent;

    //! No. of received events
    int m_nrecv;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
