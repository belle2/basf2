/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/evtgen/EvtGenModelRegister.h>

#include <memory>

using namespace std;

namespace Belle2 {

  EvtGenModelRegister& EvtGenModelRegister::getInstance()
  {
    static unique_ptr<EvtGenModelRegister> instance(new EvtGenModelRegister());
    return *instance;
  }

  list<EvtDecayBase*> EvtGenModelRegister::getModels()
  {
    list<EvtDecayBase*> modelList;
    for (auto factory : getInstance().m_models) {
      modelList.push_back(factory());
    }
    return modelList;
  }
}
