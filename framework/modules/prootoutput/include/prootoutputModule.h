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
#include <framework/core/Framework.h>
#include <framework/pcore/pEventServer.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/pcore/RingBuffer.h>

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include <framework/datastore/DataStore.h>

#include <TFile.h>
#include <TTree.h>

/*! RINBGBUF_SIZE and MAXEVTSIZE defines maximum size of IPC shared memory for RingBuffer */
/*!
  Defaulted to 4MBytes which is the default maximum of standard Linux setting.
  The size should be increased for better performance. To increase the size, system parameter
  has to be changed by adding
      kernel.shmmax = MAX_SHAREDMEM_SIZE
  in /etc/sysctl.conf and issue
   % sysctl -p
  in root account. The default Linux setting(4MB) comes from ancient limitation on the IPC memory
  size of old UNIX which is now completely obsolete. You can set any values to it.
  100MBytes could be a recommended value for it.
*/
#define RINGBUF_SIZE 1000000*4     // 1Mwords=4MB for ring buffer
#define MAXEVTSIZE   1000000*4     // 1Mwords=4MB for maximum event size

namespace Belle2 {

  /*! Class definition for pRootOutput module which substitutes SimpleOutput module. */
  /*! This module is capable of parallel processing. */

  class pRootOutputModule : public pOutputServer {

    // Public functions
  public:

    //! Constructor / Destructor
    pRootOutputModule();
    virtual ~pRootOutputModule();

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
    void setupTTree(const DataStore::EDurability&);

    //! Fill TTree in single process mode
    void fillTree(const DataStore::EDurability&);

    //! Fill RingBuffer in multiprocess mode
    void fillRingBuf(const DataStore::EDurability&);

    //! Get size of object
    size_t getSizeOfObj(const DataStore::EDurability&);
    //! Get size of array
    size_t getSizeOfArray(const DataStore::EDurability&);

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
    StoreIter* m_obj_iter[DataStore::c_NDurabilityTypes];
    StoreIter* m_array_iter[DataStore::c_NDurabilityTypes];

    //! TFile for output.
    TFile* m_file;

    //!  TTree for output.
    TTree* m_tree[DataStore::c_NDurabilityTypes];

    //! Namess of TTrees and Branches
    std::string m_treeNames[DataStore::c_NDurabilityTypes];
    std::vector<std::string> m_branchNames[DataStore::c_NDurabilityTypes];

    //! List of objects in TTree
    std::vector<std::string> m_objnames[DataStore::c_NDurabilityTypes];
    std::vector<TBranch*> m_objbrs[DataStore::c_NDurabilityTypes];
    std::vector<TObject*> m_objects[DataStore::c_NDurabilityTypes];

    //! List of arrays in TTree
    std::vector<std::string> m_arraynames[DataStore::c_NDurabilityTypes];
    std::vector<TBranch*> m_arraybrs[DataStore::c_NDurabilityTypes];
    std::vector<TClonesArray*> m_arrays[DataStore::c_NDurabilityTypes];

    //! Has branch creation already happened?
    bool m_done[DataStore::c_NDurabilityTypes];

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
