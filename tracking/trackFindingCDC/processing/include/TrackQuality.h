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

      static void maskHitsWithPoorQuality(CDCTrack& track)
      {
        double apogeeArcLenght = fabs(track.getStartTrajectory3D().getGlobalCircle().perimeter()) / 4.;

        std::vector<int> nHitsStartingArm;
        std::vector<int> nHitsEndingArm;

        std::vector<double> startingArmSLayers;
        std::vector<double> endingArmSLayers;

        for (int ii = 0; ii <= 8; ii++) {
          startingArmSLayers.push_back(0);
          endingArmSLayers.push_back(0);
          if (ii == 0) {
            nHitsStartingArm.push_back(10);
            nHitsEndingArm.push_back(10);
          } else {
            nHitsStartingArm.push_back(6);
            nHitsEndingArm.push_back(6);
          }
        }

        for (const CDCRecoHit3D& hit : track) {
          if ((hit.getArcLength2D() <= apogeeArcLenght) && (hit.getArcLength2D() > 0)) {
            startingArmSLayers[hit->getISuperLayer()]++;
          } else {
            endingArmSLayers[hit->getISuperLayer()]++;
          }
        }

        for (int ii = 0; ii <= 8; ii++) {
          startingArmSLayers[ii] = (double)startingArmSLayers[ii] / nHitsStartingArm[ii];
          endingArmSLayers[ii] = (double)endingArmSLayers[ii] / nHitsEndingArm[ii];
        }


        int startingSlayer = startingSLayer(startingArmSLayers);
        int endingSlayer = endingSLayer(startingArmSLayers);

        std::vector<int> emptyStartingSLayers;
        std::vector<int> emptyEndingSLayers;

        if (hasHoles(startingArmSLayers, startingSlayer, endingArmSLayers, endingSlayer, emptyStartingSLayers, emptyEndingSLayers)) {
          sort(emptyStartingSLayers.begin(), emptyStartingSLayers.end());

          for (int breakSLayer : emptyStartingSLayers) {
            for (CDCRecoHit3D& hit : track) {
              if (hit.getArcLength2D() >= apogeeArcLenght || hit.getArcLength2D() < 0) {
                hit.getWireHit().getAutomatonCell().setMaskedFlag();
              }
              if (hit.getISuperLayer() >= breakSLayer) {
                hit.getWireHit().getAutomatonCell().setMaskedFlag();
              }
            }

          }
        }

      };

    private:
      static int startingSLayer(const std::vector<double>& startingArmSLayers)
      {
        std::vector<double>::const_iterator startSlIt;
        startSlIt = std::find_if(startingArmSLayers.begin(), startingArmSLayers.end(), [](double val) {return val > 0;});

        if (startSlIt != startingArmSLayers.end())
          return startSlIt - startingArmSLayers.begin();
        else
          return 8;
      }


      static int endingSLayer(const std::vector<double>& startingArmSLayers)
      {
        std::vector<double>::const_reverse_iterator endSlIt;
        endSlIt = std::find_if(startingArmSLayers.rbegin(), startingArmSLayers.rend(), [](double val) {return val > 0;});

        if (endSlIt != startingArmSLayers.rend())
          return 8 - (endSlIt - startingArmSLayers.rbegin());
        else
          return 0;
      }

      static bool isTwoSided(const std::vector<double>& startingArmSLayers, const std::vector<double>& endingArmSLayers)
      {
        if ((std::accumulate(startingArmSLayers.begin(), startingArmSLayers.end(), 0) > 0) &&
            (std::accumulate(endingArmSLayers.begin(), endingArmSLayers.end(), 0) > 0)) return true;
        else return false;
      }

      static bool hasHoles(const std::vector<double>& startingArmSLayers, int startingSlayer,
                           const std::vector<double>& endingArmSLayers, int endingSlayer,
                           std::vector<int>& emptyStartingSLayers, std::vector<int>& emptyEndingSLayers)
      {
        std::vector<double>::const_iterator first = startingArmSLayers.begin() + startingSlayer;
        std::vector<double>::const_iterator last = startingArmSLayers.begin() + endingSlayer;

        for (; first <= last;) {
          if (*first == 0)emptyStartingSLayers.push_back(first - startingArmSLayers.begin());
          first = first + 2;
        }

        if (isTwoSided(startingArmSLayers, endingArmSLayers)) {
          std::vector<double>::const_iterator rfirst = endingArmSLayers.begin() + startingSlayer;
          std::vector<double>::const_iterator rlast = endingArmSLayers.begin() + endingSlayer;

          for (; rfirst <= rlast; rfirst = rfirst + 2) {
            if (*rfirst == 0)emptyEndingSLayers.push_back(rfirst - endingArmSLayers.begin());
          }
        }

        if (emptyStartingSLayers.size() > 0) return true;
        else return false;
      }

    };

  }

}

