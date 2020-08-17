//+
// File : DesSerPrePCMain.h
// Description : Base class for Module to receive data from outside and store it to DataStore
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef DESSERMAIN_H
#define DESSERMAIN_H

#include <string>

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class DesSerPrePCMainModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    DesSerPrePCMainModule();
    virtual ~DesSerPrePCMainModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from main process
    virtual void event();

  protected :
    //! hostname
    std::string m_host_recv;

    //! hostname
    std::string m_host_send;

    //! port number
    int m_port_recv;

    //! port number
    int m_port_send;

    //! Compression Level
    int m_compressionLevel;

    //! Node(PC or COPPER) ID
    int m_nodeid;

    //! Node name
    std::string m_nodename;

    //! Use shared memory -> 1; Without shm -> 0
    int m_shmflag;

    // Event Meta Data
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

  private:


  public:

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
