/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <list>
#include <vector>
#include <map>


namespace Belle2 {
  namespace TrackFindingCDC {

    class TrackQuality {

    public:

      TrackQuality(CDCTrack& track) : m_track(track)
      {
        double apogeeArcLenght = fabs(m_track.getStartTrajectory3D().getGlobalCircle().perimeter()) / 4.;
        std::vector<int> nHitsStartingArm;
        std::vector<int> nHitsEndingArm;


        for (int ii = 0; ii <= 8; ii++) {
          m_startingArmSLayers.push_back(0);
          m_endingArmSLayers.push_back(0);
          if (ii == 0) {
            nHitsStartingArm.push_back(10);
            nHitsEndingArm.push_back(10);
          } else {
            nHitsStartingArm.push_back(6);
            nHitsEndingArm.push_back(6);
          }
        }

        for (const CDCRecoHit3D& hit : m_track) {
//          B2INFO("arc: " << hit.getArcLength2D() << "; apogee: " << apogeeArcLenght);
          if ((hit.getArcLength2D() <= apogeeArcLenght) && (hit.getArcLength2D() > 0)) {
            m_startingArmSLayers[hit->getISuperLayer()]++;
          } else {
            m_endingArmSLayers[hit->getISuperLayer()]++;
          }
        }

        for (int ii = 0; ii <= 8; ii++) {
          m_startingArmSLayers[ii] = (double)m_startingArmSLayers[ii] / nHitsStartingArm[ii];
          m_endingArmSLayers[ii] = (double)m_endingArmSLayers[ii] / nHitsEndingArm[ii];
        }


        m_startingSlayer = startingSLayer();
        m_endingSlayer = endingSLayer();

        if (hasHoles()) {
//          B2WARNING("Track has holes!");

          sort(m_emptyStartingSLayers.begin(), m_emptyStartingSLayers.end());

          for (int breakSLayer : m_emptyStartingSLayers) {
//          int breakSLayer = m_emptyStartingSLayers.front();

//            B2INFO("Break layer: " << breakSLayer);
            /*
                        m_track.erase(
                        std::remove_if(m_track.begin(), m_track.end(), [&breakSLayer, &apogeeArcLenght](const CDCRecoHit3D & hit) {
                          if(hit.getArcLength2D() >= apogeeArcLenght || hit.getArcLength2D() < 0) return true;
                          if(hit.getISuperLayer() >= breakSLayer) return true;
                          return false;
                        }),
                        m_track.end());
                      }
            */
            for (CDCRecoHit3D& hit : m_track) {
              if (hit.getArcLength2D() >= apogeeArcLenght || hit.getArcLength2D() < 0) hit.getWireHit().getAutomatonCell().setMaskedFlag(true);
              if (hit.getISuperLayer() >= breakSLayer) hit.getWireHit().getAutomatonCell().setMaskedFlag(true);
            }

          }
        }

      };

      int startingSLayer()
      {
        std::vector<double>::iterator startSlIt;
        startSlIt = std::find_if(m_startingArmSLayers.begin(), m_startingArmSLayers.end(), [](double val) {return val > 0;});

        if (startSlIt != m_startingArmSLayers.end())
          return startSlIt - m_startingArmSLayers.begin();
        else
          return 8;
      }


      int endingSLayer()
      {
        std::vector<double>::reverse_iterator endSlIt;
        endSlIt = std::find_if(m_startingArmSLayers.rbegin(), m_startingArmSLayers.rend(), [](double val) {return val > 0;});

        if (endSlIt != m_startingArmSLayers.rend())
          return 8 - (endSlIt - m_startingArmSLayers.rbegin());
        else
          return 0;
      }

      bool isTwoSided()
      {
        if ((std::accumulate(m_startingArmSLayers.begin(), m_startingArmSLayers.end(), 0) > 0) &&
            (std::accumulate(m_endingArmSLayers.begin(), m_endingArmSLayers.end(), 0) > 0)) return true;
        else return false;
      }

      bool hasHoles()
      {
        std::vector<double>::iterator first = m_startingArmSLayers.begin() + m_startingSlayer;
        std::vector<double>::iterator last = m_startingArmSLayers.begin() + m_endingSlayer;

        for (; first <= last;) {
          if (*first == 0)m_emptyStartingSLayers.push_back(first - m_startingArmSLayers.begin());
          first = first + 2;
        }

        if (isTwoSided()) {
          std::vector<double>::iterator rfirst = m_endingArmSLayers.begin() + m_startingSlayer;
          std::vector<double>::iterator rlast = m_endingArmSLayers.begin() + m_endingSlayer;

          for (; rfirst <= rlast; rfirst = rfirst + 2) {
            if (*rfirst == 0)m_emptyEndingSLayers.push_back(rfirst - m_endingArmSLayers.begin());
          }
        }

        if (m_emptyStartingSLayers.size() > 0) return true;
        else return false;
      }

    private:

      std::vector<double> m_startingArmSLayers;
      std::vector<double> m_endingArmSLayers;

      int m_startingSlayer;
      int m_endingSlayer;

      std::vector<int> m_emptyStartingSLayers;
      std::vector<int> m_emptyEndingSLayers;


      CDCTrack& m_track;

    };

  }

}

