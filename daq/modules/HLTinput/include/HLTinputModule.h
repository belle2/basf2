/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HLTINPUTMODULE_H
#define HLTINPUTMODULE_H

#include <vector>

#include <framework/core/Module.h>

#include <framework/datastore/DataStore.h>

#include <framework/core/Module.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/RingBuffer.h>

#include <daq/hlt/EvtReceiver.h>
#include <daq/hlt/HLTDefs.h>

#define MAXPACKET 10000000 * 4

namespace Belle2 {

  class HLTInputModule : public Module {

  public:
    HLTInputModule();
    virtual ~HLTInputModule();

    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    int readData(const DataStore::EDurability&);

  protected:


  private:
    EvtReceiver* m_evtReceiver;
    RingBuffer* m_inBuf;
    pid_t m_pidEvtReceiver;
    int m_port;

    std::vector<std::string> m_objnames[DataStore::c_NDurabilityTypes];
    std::vector<std::string> m_arraynames[DataStore::c_NDurabilityTypes];

    MsgHandler* m_msgHandler;
  };

} // end namespace Belle2

#endif // HLTINPUTMODULE_H
