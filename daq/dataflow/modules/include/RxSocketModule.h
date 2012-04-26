//+
// File : rxmodule.h
// Description : Module to restore DataStore in RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#ifndef RXMODULE_H
#define RXMODULE_H

#include <framework/core/Module.h>
//#include <framework/core/Framework.h>
//#include <framework/pcore/pEventServer.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>
#include <daq/dataflow/EvtSocket.h>

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include <framework/datastore/DataStore.h>


namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class RxSocketModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    RxSocketModule();
    virtual ~RxSocketModule();

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

    // Parallel processing parameters

    //! Receiver Port
    int m_port;

    //! Reciever Socket
    EvtSocketRecv* m_recv;

    //! Messaage handler
    MsgHandler* m_msghandler;

    //! Compression Level
    int m_compressionLevel;

    //! No. of sent events
    int m_nsent;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
