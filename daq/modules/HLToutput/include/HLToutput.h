/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HLTOUTPUT_H
#define HLTOUTPUT_H

#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

#include <framework/core/Module.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreDefs.h>

#include <framework/core/EvtMessage.h>
#include <framework/core/MsgHandler.h>
#include <framework/core/RingBuffer.h>

#include <daq/hlt/EvtSender.h>

#define MAXPACKET 10000000 * 4

namespace Belle2 {

  class HLTOutput : public Module {

  public:
    HLTOutput();
    virtual ~HLTOutput();

    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    void putData(const std::string);
    void putData(const EDurability&);

  protected:


  private:
    EvtSender* m_evtSender;
    RingBuffer* m_outBuf;
    pid_t m_pidEvtSender;
    int m_port;
    std::string m_dest;

    std::vector<std::string> m_branchNames[c_NDurabilityTypes];
    bool m_done[c_NDurabilityTypes];
    StoreIter* m_obj_iter[c_NDurabilityTypes];
    StoreIter* m_array_iter[c_NDurabilityTypes];

    MsgHandler* m_msgHandler;
  };

} // end namespace Belle2

#endif // SIMPLEINPUT_H
