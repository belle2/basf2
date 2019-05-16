/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/DecayDescriptor/ParticleListHelper.h>
#include <analysis/DecayDescriptor/ParticleListName.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

namespace Belle2 {
  void ParticleListHelper::registerList(const std::string& listname, bool save)
  {
    DecayDescriptor decayDescriptor;
    bool valid = decayDescriptor.init(listname);
    if (!valid)
      throw std::runtime_error("Invalid ParticleList name: '" + listname + "' Should be EVTPDLNAME[:LABEL], e.g. B+ or B+:mylist.");
    ParticleListHelper::registerList(decayDescriptor, save);
  }

  void ParticleListHelper::registerList(const DecayDescriptor& decay, bool save)
  {
    auto listname = decay.getMother()->getFullName();
    m_pdg = decay.getMother()->getPDGCode();
    auto storeFlags = save ? DataStore::c_WriteOut : DataStore::c_DontWriteOut;
    m_particles.registerInDataStore(storeFlags);
    m_list.registerInDataStore(listname, storeFlags);
    auto antiListName = ParticleListName::antiParticleListName(listname);
    if (antiListName != listname) {
      m_antiList.emplace(antiListName);
      m_antiList->registerInDataStore(storeFlags);
    }
  }
  void ParticleListHelper::create()
  {
    m_list.create();
    m_list->initialize(m_pdg, m_list.getName());
    if (m_antiList) {
      m_antiList->create();
      (*m_antiList)->initialize(-m_pdg, m_antiList->getName());
      m_list->bindAntiParticleList(**m_antiList);
    }
  }
}
