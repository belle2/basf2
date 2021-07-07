/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef RBUF2RBUFMODULE_H
#define RBUF2RBUFMODULE_H

#include <framework/core/Module.h>
#include <framework/pcore/RingBuffer.h>

#include <string>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#define RBUFSIZE 100000000
#define MAXEVTSIZE  400000000


namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class Rbuf2RbufModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    Rbuf2RbufModule();
    virtual ~Rbuf2RbufModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    // Data members
  private:

    //! Input RingBuffer ID
    std::string m_name_rbufin;
    RingBuffer* m_rbufin;

    //! Output RingBuffer ID
    std::string m_name_rbufout;
    RingBuffer* m_rbufout;

    //! No. of sent events
    int m_nrecv;

    StoreObjPtr<EventMetaData> m_eventMetaData; /**< EventMetaData. */
  };

} // end namespace Belle2

#endif // MODULEHELLO_H
