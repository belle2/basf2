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
#include <boost/foreach.hpp>

using namespace std;

namespace Belle2 {

  EvtGenModelRegister& EvtGenModelRegister::getInstance()
  {
    static auto_ptr<EvtGenModelRegister> instance(new EvtGenModelRegister());
    return *instance;
  }

  list<EvtDecayBase*> EvtGenModelRegister::getModels()
  {
    list<EvtDecayBase*> modelList;
    BOOST_FOREACH(ModelFactory * factory, getInstance().m_models) {
      modelList.push_back(factory());
    }
    return modelList;
  }
}
