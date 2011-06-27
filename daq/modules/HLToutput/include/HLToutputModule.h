/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HLTOUTPUTMODULE_H
#define HLTOUTPUTMODULE_H

#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

#include <framework/core/Module.h>

#include <framework/datastore/DataStore.h>

#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>

#include <daq/hlt/EvtSender.h>
#include <daq/hlt/HLTBuffer.h>

#define MAXPACKET 10000000 * 4

namespace Belle2 {

  class HLTOutputModule : public Module {

  public:
    HLTOutputModule();
    virtual ~HLTOutputModule();

    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    void putData(const std::string);
    void putData(const DataStore::EDurability&);

  protected:


  private:
    HLTBuffer* m_outBuf;
    HLTBuffer* m_testBuf;
    std::string m_outBufferName;

    std::vector<std::string> m_branchNames[DataStore::c_NDurabilityTypes];
    bool m_done[DataStore::c_NDurabilityTypes];
    StoreIter* m_obj_iter[DataStore::c_NDurabilityTypes];
    StoreIter* m_array_iter[DataStore::c_NDurabilityTypes];

    MsgHandler* m_msgHandler;
  };

} // end namespace Belle2

#endif // HLTOUTPUTMODULE_H
