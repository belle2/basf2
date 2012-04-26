//+
// File : txsocket.h
// Description : Module to send DataStore via EvtSocket
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
#include <daq/dataflow/EvtSocket.h>

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include <framework/datastore/DataStore.h>


namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class TxSocketModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    TxSocketModule();
    virtual ~TxSocketModule();

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

    // Parameters for EvtSocket

    //! Destination Host
    std::string m_dest;

    //! Destination port
    int m_port;

    //! EvtSocket
    EvtSocketSend* m_sock;

    //! Messaage handler
    MsgHandler* m_msghandler;

    //! No. of sent events
    int m_nsent;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
