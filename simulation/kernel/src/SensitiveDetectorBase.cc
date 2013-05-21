/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Ritter                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/kernel/SensitiveDetectorBase.h>
#include <framework/logging/Logger.h>

using namespace std;
namespace Belle2 {
  namespace Simulation {
    bool SensitiveDetectorBase::m_active(false);
    map<string, RelationArray::EConsolidationAction> SensitiveDetectorBase::m_mcRelations;

    void SensitiveDetectorBase::registerMCParticleRelation(const std::string& name, RelationArray::EConsolidationAction ignoreAction)
    {
      std::pair<std::map<std::string, RelationArray::EConsolidationAction>::iterator, bool> insert =  m_mcRelations.insert(std::make_pair(name, ignoreAction));
      //If the relation already exists and the ignoreAction is different we do have a problem
      if (!insert.second && insert.first->second != ignoreAction) {
        B2FATAL("MCParticle Relation " << name << " already registered with different ignore action.");
      }
    }
  }
}
