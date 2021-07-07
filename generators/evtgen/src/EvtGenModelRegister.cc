/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
