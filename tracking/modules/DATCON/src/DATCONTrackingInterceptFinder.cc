/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell, Christian Wessel                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/DATCON/DATCONTrackingModule.h>

using namespace std;
using namespace Belle2;

/*
* Very simple sensor filter / layer filter / sensor layer filter.
*/
bool
DATCONTrackingModule::layerFilter(bool* layer)
{
  unsigned int layercount = 0;

  /* Count number of found layers */
  for (int i = 0; i < 4; ++i) {
    if (layer[i] == true) {
      ++layercount;
    }
  }
  // return true, if lines in sector are from at least minLines different layers
  if (layercount >= m_minimumLines) {
    return (true);
  }

  return (false);
}

/*
* Find the intercept in hough, or hess hough space.  // does "hess hough space" here mean "conformal mapped hough space"?
* Return zero on success.
* Parameter for uSide:
* x = phi0
* y = r
* Parameter for v_side:
* x = m
* y = a
*/
int
DATCONTrackingModule::fastInterceptFinder2d(houghMap& hits, bool uSide, TVector2 v1_s,
                                            TVector2 v2_s,
                                            __attribute__((unused)) TVector2 v3_s,
                                            TVector2 v4_s,
                                            unsigned int iterations, unsigned int maxIterations)
{
  int hitID;
  unsigned int countLayer;
  double unitX, unitY;
  double y1 = 0., y2 = 0.;
  double m, a;
  houghPair hp;
  VxdID sensor;
  vector<unsigned int> candidateIDList;
  double xdiff = -10.0, ydiff = -10.0;  /**< differences used to fill the ArrayOfActiveSectors */
  int sectorX, sectorY;
  double baseX, baseY;
  int maxSectorX, maxSectorY;
  houghMap containedHits;

  TVector2 v1, v2, v3, v4;

  unitX = ((v2_s.X() - v1_s.X()) / 2.0);
  unitY = ((v1_s.Y() - v4_s.Y()) / 2.0);

  if (uSide) {
    baseX = -M_PI;
    baseY = -m_rectSizeU;
    maxSectorX = (int)pow(2, m_maxIterationsU + 1) - 1;
    maxSectorY = (int)pow(2, m_maxIterationsU + 1) - 1;
  } else {
    baseX = -M_PI;
    baseY = -m_rectSizeV;
    maxSectorX = (int)pow(2, m_maxIterationsV + 1) - 1;
    maxSectorY = (int)pow(2, m_maxIterationsV + 1) - 1;
  }

  countLayer = 0;
  for (int i = 0; i < 2 ; ++i) {
    for (int j = 0; j < 2; ++j) {
      v1.Set((v4_s.X() + (double) i * unitX), (v4_s.Y() + (double) j * unitY + unitY));
      v2.Set((v4_s.X() + (double) i * unitX + unitX), (v4_s.Y() + (double) j * unitY + unitY));
      v3.Set((v4_s.X() + (double) i * unitX + unitX), (v4_s.Y() + (double) j * unitY));
      v4.Set((v4_s.X() + (double) i * unitX), (v4_s.Y() + (double) j * unitY));

      candidateIDList.clear();
      bool layerHit[4] = {false}; /* For layer filter */
      for (auto& hit : hits) {
        hitID = hit.first;
        hp = hit.second;
        sensor = hp.first;

        if (!uSide) {
          m = hp.second.X();
          a = hp.second.Y();
          y1 = m * cos(v1.X()) + a * sin(v1.X());
          y2 = m * cos(v2.X()) + a * sin(v2.X());
        } else {
          m = hp.second.X();
          a = hp.second.Y();
          y1 = 2. * (m * cos(v1.X()) + a * sin(v1.X()));
          y2 = 2. * (m * cos(v2.X()) + a * sin(v2.X()));
          if (m_usePhase2Simulation) {
            m = hp.second.X();
            a = hp.second.Y();
            y1 = m * cos(v1.X()) + a * sin(v1.X());
            y2 = m * cos(v2.X()) + a * sin(v2.X());
          }
        }

        /* Check if HS-parameter curve is inside (or outside) actual sub-HS */
        if (y1 <= v1.Y() && y2 >= v3.Y() && y2 >= y1) {
          if (iterations == maxIterations) {
            candidateIDList.push_back(hitID);
          }

          layerHit[sensor.getLayerNumber() - 3] = true; /* layer filter */
          ++countLayer;
          containedHits.insert(hit);
        }
      }

      if (countLayer >= m_minimumLines) {
        if (layerFilter(layerHit)) {
          // recursive / iterative call of fastInterceptFinder2d, until iterations = critIterations (critIterations-1),
          // actual values for v1...v4 are new startingpoints
          if (iterations != maxIterations /*critIterations*/) {
//             fastInterceptFinder2d(hits, uSide, v1, v2, v3, v4,
//                                   iterations + 1, maxIterations);
            fastInterceptFinder2d(containedHits, uSide, v1, v2, v3, v4,
                                  iterations + 1, maxIterations);
          } else {
            if (!uSide) {
              vHoughCand.push_back(DATCONHoughCand(candidateIDList, make_pair(v1, v3)));
              if (m_useHoughSpaceClustering) {
                xdiff = v4.X() - baseX;
                ydiff = v4.Y() - baseY;
                sectorX = round(xdiff / unitX);
                sectorY = maxSectorY - round(ydiff / unitY);
                if (sectorX > maxSectorX) {
                  sectorX = maxSectorX;
                } else if (sectorX < 0) {
                  sectorX = 0;
                }
                if (sectorY > maxSectorY) {
                  sectorY = maxSectorY;
                } else if (sectorY < 0) {
                  sectorY = 0;
                }
                ArrayOfActiveSectorsThetaHS[sectorY][sectorX] = -1;
                vHoughSpaceClusterCand.push_back(DATCONHoughSpaceClusterCand(candidateIDList, TVector2(sectorX, sectorY)));

                activeSectorVectorTheta.push_back(true);
              }

            } else {
              uHoughCand.push_back(DATCONHoughCand(candidateIDList, make_pair(v1, v3)));
              if (m_useHoughSpaceClustering) {
                baseX = -M_PI;
                baseY = -m_rectSizeU;
                xdiff = v4.X() - baseX;
                ydiff = v4.Y() - baseY;
                maxSectorX = (int)pow(2, m_maxIterationsU + 1) - 1;
                maxSectorY = (int)pow(2, m_maxIterationsU + 1) - 1;
                sectorX = round(xdiff / unitX);
                sectorY = maxSectorY - round(ydiff / unitY);
                if (sectorX > maxSectorX) {
                  sectorX = maxSectorX;
                } else if (sectorX < 0) {
                  sectorX = 0;
                }
                if (sectorY > maxSectorY) {
                  sectorY = maxSectorY;
                } else if (sectorY < 0) {
                  sectorY = 0;
                }
                ArrayOfActiveSectorsPhiHS[sectorY][sectorX] = -1;
                uHoughSpaceClusterCand.push_back(DATCONHoughSpaceClusterCand(candidateIDList, TVector2(sectorX, sectorY)));
              }

              if (m_useHoughSpaceClustering) {
                activeSectorVectorPhi.push_back(true);
              }
            }
          }
        }
      } else {
        if (m_useHoughSpaceClustering) {
          if (uSide) {
            activeSectorVectorPhi.push_back(false);
          } else {
            activeSectorVectorTheta.push_back(false);
          }
        }
      }

      countLayer = 0;
    }
  }

  return 0;
}


/*
* Find the intercept in hough, or hess hough space.  // does "hess hough space" here mean "conformal mapped hough space"?
* Return zero on success.
* Parameter for uSide:
* x = phi0
* y = r
* Parameter for v_side:
* x = m
* y = a
*/
int
DATCONTrackingModule::slowInterceptFinder2d(houghMap& hits, bool uSide)
{
  int hitID;
  unsigned int countLayer;
  double unitX, unitY;
  double y1 = 0, y2 = 0;
  double m, a;
  houghPair hp;
  VxdID sensor;
  vector<unsigned int> candidateIDList;
  unsigned short angleSectors, vertSectors;
  double angleRange, vertRange;
  double left, right, up, down;

  TVector2 v1, v2, v3, v4;
  if (m_usePhase2Simulation) {
    // ATTENTION TODO FIXME : This still has to be implemented!!!
    // So far no phase 2 specific algorithms have been implemented and tested!
    B2WARNING("This mode is not yet implemented, nothing will happen! Return...");
    return 0;
  } else {
    if (uSide) {
      angleSectors = m_nAngleSectorsU;
      vertSectors  = m_nVertSectorsU;
      left         = -M_PI;
      right        =  M_PI;
      up           =  m_rectSizeU;
      down         = -m_rectSizeU;
    } else {
      angleSectors = m_nAngleSectorsV;
      vertSectors  = m_nVertSectorsV;
      left         = -M_PI;
      right        =  0.;
      up           =  m_rectSizeV;
      down         = -m_rectSizeV;
    }
  }

  angleRange = (right - left);
  vertRange = (up - down);
  unitX = angleRange / (double)(angleSectors);
  unitY = vertRange / (double)(vertSectors);

  countLayer = 0;
  for (int i = vertSectors - 1; i >= 0; i--) {
    for (int j = 0; j < angleSectors; j++) {
      v1.Set((left + (double) j * unitX), (up - (double) i * unitY));
      v2.Set((left + (double) j * unitX + unitX), (up - (double) i * unitY));
      v3.Set((left + (double) j * unitX + unitX), (up - (double) i * unitY - unitY));
      v4.Set((left + (double) j * unitX), (up - (double) i * unitY - unitY));


      candidateIDList.clear();
      bool layerHit[4] = {false}; /* For layer filter */
      for (auto& hit : hits) {
        hitID = hit.first;
        hp = hit.second;
        sensor = hp.first;

        if (!uSide) {
          m = hp.second.X();
          a = hp.second.Y();
          y1 = m * cos(v1.X()) + a * sin(v1.X());
          y2 = m * cos(v2.X()) + a * sin(v2.X());
        } else {
          m = hp.second.X();
          a = hp.second.Y();
          y1 = 2 * (m * cos(v1.X()) + a * sin(v1.X()));
          y2 = 2 * (m * cos(v2.X()) + a * sin(v2.X()));
        }

        if (y1 <= v1.Y() && y2 >= v3.Y() && y2 > y1) {
          candidateIDList.push_back(hitID);
          layerHit[sensor.getLayerNumber() - 3] = true; /* layer filter */
          ++countLayer;
        }
      }
      if (countLayer >= m_minimumLines) {
        if (layerFilter(layerHit)) {
          if (!uSide) {
            vHoughCand.push_back(DATCONHoughCand(candidateIDList, make_pair(v1, v3)));
            ArrayOfActiveSectorsThetaHS[i][j] = -1;
            vHoughSpaceClusterCand.push_back(DATCONHoughSpaceClusterCand(candidateIDList, TVector2(j, i)));
            activeSectorVectorTheta.push_back(true);
          } else {
            uHoughCand.push_back(DATCONHoughCand(candidateIDList, make_pair(v1, v3)));
            ArrayOfActiveSectorsPhiHS[i][j] = -1;
            uHoughSpaceClusterCand.push_back(DATCONHoughSpaceClusterCand(candidateIDList, TVector2(j, i)));
            activeSectorVectorPhi.push_back(true);
          }
        }
      } else {
        if (uSide) {
          activeSectorVectorPhi.push_back(false);
        } else {
          activeSectorVectorTheta.push_back(false);
        }
      }

      countLayer = 0;
    }   // for (int j ...
  }     // for (int i ...

  return 0;
}
