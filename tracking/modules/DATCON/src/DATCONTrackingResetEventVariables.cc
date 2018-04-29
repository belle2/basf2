/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/DATCON/DATCONTrackingModule.h>

using namespace std;
using namespace Belle2;

void DATCONTrackingModule::resetEventVariables()
{

  /* Reset cluster index variables */
  indexU = 0;
  indexV = 0;

  uClusters.clear();
  vClusters.clear();
  uHough.clear();
  uHoughCand.clear();
  uTrackCand.clear();
  vHough.clear();
  vHoughCand.clear();
  vTrackCand.clear();
  uHoughSpaceClusterCand.clear();
  vHoughSpaceClusterCand.clear();
  DATCONTracks.clear();

  if (m_useHoughSpaceClustering) {
    if (m_independentSectors) {
      for (int i = 0; i < m_nVertSectorsU; i++) {
        for (int j = 0; j < m_nAngleSectorsU; j++) {
          ArrayOfActiveSectorsPhiHS[i][j] = 0;
        }
      }
      for (int i = 0; i < m_nVertSectorsV; i++) {
        for (int j = 0; j < m_nAngleSectorsV; j++) {
          ArrayOfActiveSectorsThetaHS[i][j] = 0;
        }
      }

      activeSectorVectorPhi.clear();
      activeSectorVectorTheta.clear();
    } else {
      for (int i = 0; i < (int)pow(2, m_maxIterationsU + 1); i++) {
        for (int j = 0; j < (int)pow(2, m_maxIterationsU + 1); j++) {
          ArrayOfActiveSectorsPhiHS[i][j] = 0;
        }
      }
      for (int i = 0; i < (int)pow(2, m_maxIterationsV + 1); i++) {
        for (int j = 0; j < (int)pow(2, m_maxIterationsV + 1); j++) {
          ArrayOfActiveSectorsThetaHS[i][j] = 0;
        }
      }

      activeSectorVectorPhi.clear();
      activeSectorVectorTheta.clear();
    }
  }

}
