/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <mdst/dataobjects/MCParticleGraph.h>
#include <vector>

// Forward declaration - keeps <HepMC/GenEvent.h> out of this header; full include in HerwigFragHelper.cc.
namespace HepMC { class GenEvent; }

namespace Belle2 {

  /**
   * Helper class for Herwig fragmentation modules.
   * Merges particles from an already-loaded HepMC2 GenEvent into an
   * existing MCParticleGraph.
   */
  class HerwigFragHelper {
  public:
    HerwigFragHelper();
    ~HerwigFragHelper();

    /**
     * Add an already-loaded HepMC::GenEvent to existing MCParticleGraph.
     * @param[in]     evt          Pointer to the HepMC2 event
     * @param[in,out] graph        MCParticleGraph to extend; KKMC particles already present, Herwig particles appended after them
     * @param[in]     quarkIndices Graph indices of the original KKMC quarks, used for fallback parent linking
     * @return Number of particles added, or -1 on error
     */
    int addHepMCToGraph(HepMC::GenEvent* evt,
                        MCParticleGraph& graph,
                        const std::vector<int>& quarkIndices);

  };

} // namespace Belle2
