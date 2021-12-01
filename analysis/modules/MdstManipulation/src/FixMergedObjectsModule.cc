/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/MdstManipulation/FixMergedObjectsModule.h>

#include <framework/gearbox/Const.h>

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(FixMergedObjects)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  FixMergedObjectsModule::FixMergedObjectsModule() : Module()
  {
    setDescription("Fix indices of mdst objects (Tracks, V0s, MCParticles) after DataStores were merged using and indepentent_path.");

    setPropertyFlags(c_ParallelProcessingCertified);
  }

  void FixMergedObjectsModule::initialize()
  {
    m_mergedArrayIndices.isRequired();
    m_tracks.isOptional();
    m_v0s.isOptional();
    m_mcParticles.isOptional();
  }

  void FixMergedObjectsModule::event()
  {
    if (m_tracks.isValid() && m_mergedArrayIndices->getIndex("Tracks") != -1) {
      for (int t_idx = m_mergedArrayIndices->getIndex("Tracks"); t_idx < m_tracks.getEntries(); t_idx++) {
        for (unsigned int i = 0; i < Const::ChargedStable::c_SetSize; i++) {
          // hypothesis not fitted
          if (m_tracks[t_idx]->m_trackFitIndices[i] == -1) {
            continue;
          }
          // declared friends class, so this can be done
          m_tracks[t_idx]->m_trackFitIndices[i] += m_mergedArrayIndices->getIndex("TrackFitResults");
        }
      }
    }

    if (m_v0s.isValid() && m_mergedArrayIndices->getIndex("V0s") != -1) {
      for (int v_idx = m_mergedArrayIndices->getIndex("V0s"); v_idx < m_v0s.getEntries(); v_idx++) {
        // declared friends class, so this can be done
        m_v0s[v_idx]->m_trackIndexPositive += m_mergedArrayIndices->getIndex("Tracks");
        m_v0s[v_idx]->m_trackIndexNegative += m_mergedArrayIndices->getIndex("Tracks");
        m_v0s[v_idx]->m_trackFitResultIndexPositive += m_mergedArrayIndices->getIndex("TrackFitResults");
        m_v0s[v_idx]->m_trackFitResultIndexNegative += m_mergedArrayIndices->getIndex("TrackFitResults");
      }
    }

    // MCParticles not yet implemented - clear them for now
    m_mcParticles.clear();
  }
} // end Belle2 namespace

