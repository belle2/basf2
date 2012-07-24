//+
// File : txmodule.h
// Description : Module to put DataStore in RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#ifndef TXMODULE_H
#define TXMODULE_H

#include <framework/core/Module.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/pcore/RingBuffer.h>
#include <framework/pcore/RbCtlMgr.h>
#include <framework/pcore/SeqFile.h>

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include <framework/datastore/DataStore.h>


namespace Belle2 {

  /** Module for encoding data store contents into a RingBuffer. */
  class TxModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    TxModule();
    //! Use the given RingBuffer for data
    TxModule(RingBuffer*);
    virtual ~TxModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    // Data members
  private:
    //! Names of TTrees
    std::string m_treeNames[DataStore::c_NDurabilityTypes];
    //! Names of all branches for given durability
    std::vector<std::string> m_branchNames[DataStore::c_NDurabilityTypes];

    //! List of object names in TTree
    std::vector<std::string> m_objnames[DataStore::c_NDurabilityTypes];
    //! List of objects in TTree
    std::vector<TObject*> m_objects[DataStore::c_NDurabilityTypes];

    //! List of array names in TTree
    std::vector<std::string> m_arraynames[DataStore::c_NDurabilityTypes];
    //! List of arrays in TTree
    std::vector<TClonesArray*> m_arrays[DataStore::c_NDurabilityTypes];

    //!Compression parameter
    int m_compressionLevel;


    // Parallel processing parameters

    //! RingBuffer ID
    RingBuffer* m_rbuf;

    //! Messaage handler
    MsgHandler* m_msghandler;

    //! Process contrl manager
    RbCtlMgr* m_rbctl;

    //! No. of sent events
    int m_nsent;

  };

} // end namespace Belle2

#endif
