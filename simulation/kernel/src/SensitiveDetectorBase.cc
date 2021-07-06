/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <simulation/kernel/SensitiveDetectorBase.h>
#include <framework/logging/Logger.h>

using namespace std;
namespace Belle2 {
  namespace Simulation {
    bool SensitiveDetectorBase::s_active(false);
    map<string, RelationArray::EConsolidationAction> SensitiveDetectorBase::s_mcRelations;

    void SensitiveDetectorBase::registerMCParticleRelation(const std::string& name, RelationArray::EConsolidationAction ignoreAction)
    {
      std::pair<std::map<std::string, RelationArray::EConsolidationAction>::iterator, bool> insert = s_mcRelations.insert(std::make_pair(
            name, ignoreAction));
      //If the relation already exists and the ignoreAction is different we do have a problem
      if (!insert.second && insert.first->second != ignoreAction) {
        B2FATAL("MCParticle Relation " << name << " already registered with different ignore action.");
      }
    }
  }
}
