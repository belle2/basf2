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
//#include <framework/core/Framework.h>
//#include <framework/pcore/pEventServer.h>
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

  /*! A class definition of an input module for Sequential ROOT I/O */

  class TxModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    TxModule();
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
    //! Namess of TTrees and Branches
    std::string m_treeNames[DataStore::c_NDurabilityTypes];
    std::vector<std::string> m_branchNames[DataStore::c_NDurabilityTypes];

    //! List of objects in TTree
    std::vector<std::string> m_objnames[DataStore::c_NDurabilityTypes];
    std::vector<TObject*> m_objects[DataStore::c_NDurabilityTypes];

    //! List of arrays in TTree
    std::vector<std::string> m_arraynames[DataStore::c_NDurabilityTypes];
    std::vector<TClonesArray*> m_arrays[DataStore::c_NDurabilityTypes];

    //! DataStore iterators
    StoreIter* m_obj_iter[DataStore::c_NDurabilityTypes];
    StoreIter* m_array_iter[DataStore::c_NDurabilityTypes];

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

#endif // MODULEHELLO_H
