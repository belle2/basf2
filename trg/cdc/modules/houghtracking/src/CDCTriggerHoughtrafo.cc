/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <trg/cdc/modules/houghtracking/CDCTrigger2DFinderModule.h>

#include <framework/logging/Logger.h>

#include <cmath>
#include <array>

#include <root/TMatrix.h>

/* defines */
#define CDC_SUPER_LAYERS 45

using namespace Belle2;

unsigned short
CDCTrigger2DFinderModule::countSL(bool* layer)
{
  if (m_requireSL0 && !layer[0]) return 0;
  unsigned short lcnt = 0;
  for (int i = 0; i < CDC_SUPER_LAYERS; ++i) {
    if (layer[i] == true) ++lcnt;
  }
  return lcnt;
}

bool
CDCTrigger2DFinderModule::shortTrack(bool* layer)
{
  unsigned short lcnt = 0;
  // check axial super layers (even layer number),
  // break at first layer without hit
  for (int i = 0; i < CDC_SUPER_LAYERS; i += 2) {
    if (layer[i] == true) ++lcnt;
    else break;
  }
  return (lcnt >= m_minHitsShort);
}

/*
* Find the intercept in hough space.
* In each iteration the Hough plane is divided in quarters.
* For each quarter containing enough hits, the process is repeated
* until the quarters correspond to single Hough cells.
* Return zero on success.
* x = m
* y = a
*/
int
CDCTrigger2DFinderModule::fastInterceptFinder(CDCHitMap& hits,
                                              double x1_s, double x2_s,
                                              double y1_s, double y2_s,
                                              unsigned iterations,
                                              unsigned ix_s, unsigned iy_s)
{
  std::string indent = "";
  for (unsigned i = 0; i < iterations; ++i) indent += " ";
  B2DEBUG(150, indent << "intercept finder iteration " << iterations
          << " x1 " << x1_s * 180 / M_PI << " x2 " << x2_s * 180 / M_PI
          << " y1 " << y1_s << " y2 " << y2_s
          << " ix " << ix_s << " iy " << iy_s);

  int i, j, iHit;
  double unitx, unity;
  double y1, y2;
  double m, a;
  CDCPair hp;
  unsigned short iSL;
  // list of hit indices crossing the current rectangle
  std::vector<unsigned> idx_list;
  idx_list.reserve(hits.size());

  // limits of divided rectangles
  double x1_d, x2_d, y1_d, y2_d;

  // cell indices in full plane
  unsigned ix, iy;

  unitx = ((x2_s - x1_s) / 2.0);
  unity = ((y2_s - y1_s) / 2.0);

  // divide x axis in half
  for (i = 0; i < 2 ; ++i) {
    // divide y axis in half
    for (j = 0; j < 2; ++j) {
      x1_d = x1_s + i * unitx;
      x2_d = x1_s + (i + 1) * unitx;
      y1_d = y1_s + j * unity;
      y2_d = y1_s + (j + 1) * unity;

      ix = 2 * ix_s + i;
      iy = 2 * iy_s + j;

      // skip extra cells outside of the Hough plane
      if (x2_d <= -M_PI || x1_d >= M_PI || y2_d <= -m_maxR + m_shiftR || y1_d >= m_maxR + m_shiftR) {
        B2DEBUG(150, indent << "skip Hough cell outside of plane limits");
        continue;
      }

      // if the cell size in phi is too large, the hit calculation is not reliable
      // -> continue to next iteration without hit check
      if (iterations != m_maxIterations && unitx > M_PI / 2.) {
        fastInterceptFinder(hits, x1_d, x2_d, y1_d, y2_d, iterations + 1, ix, iy);
        continue;
      }

      const double sin_x1d = std::sin(x1_d - 1e-10);
      const double cos_x1d = std::cos(x1_d - 1e-10);
      const double sin_x2d = std::sin(x2_d + 1e-10);
      const double cos_x2d = std::cos(x2_d + 1e-10);

      idx_list.clear();
      bool layerHit[CDC_SUPER_LAYERS] = {false}; /* For layer filter */
      for (auto it = hits.begin(); it != hits.end(); ++it) {
        iHit = it->first;
        hp = it->second;
        iSL = hp.first;
        m = hp.second.X();
        a = hp.second.Y();

        // calculate Hough curve with slightly enlarged limits to avoid errors due to rounding
        y1 = m * sin_x1d - a * cos_x1d;
        y2 = m * sin_x2d - a * cos_x2d;
        // skip decreasing half of the sine (corresponds to curl back half of the track)
        if (iterations == m_maxIterations && y1 > y2) continue;
        if (!((y1 > y2_d && y2 > y2_d) || (y1 < y1_d && y2 < y1_d))) {
          if (iterations == m_maxIterations) {
            idx_list.push_back(iHit);
          }
          layerHit[iSL] = true; /* layer filter */
        }
      }
      unsigned short nSL = countSL(layerHit);
      B2DEBUG(150, indent << "i " << i << " j " << j
              << " layerHit " << int(layerHit[0]) << int(layerHit[2])
              << int(layerHit[4]) << int(layerHit[6]) << int(layerHit[8])
              << " nSL " << nSL);
      //std::cout << indent << "i " << i << " j " << j
      //        << " layerHit " << int(layerHit[0]) << int(layerHit[2])
      //        << int(layerHit[4]) << int(layerHit[6]) << int(layerHit[8])
      //        << " nSL " << nSL << std::endl;
      if (nSL >= m_minHits || shortTrack(layerHit)) {
        if (iterations != m_maxIterations) {
          fastInterceptFinder(hits, x1_d, x2_d, y1_d, y2_d, iterations + 1, ix, iy);
        } else {
          ROOT::Math::XYVector v1(x1_d, y1_d);
          ROOT::Math::XYVector v2(x2_d, y2_d);
          m_houghCand.push_back(CDCTriggerHoughCand(idx_list, std::make_pair(v1, v2),
                                                    nSL, m_houghCand.size()));
          if (m_storePlane > 0) {
            (*m_houghPlane)[ix - (m_nCells - m_nCellsPhi) / 2][iy - (m_nCells - m_nCellsR) / 2] = nSL;
          }
        }
      } else if (m_storePlane > 1) {
        // to store the full Hough plane, we need to continue the iteration
        // to minimal cell size everywhere
        for (unsigned min = m_minHits - 1; min > 0; --min) {
          if (nSL >= min) {
            if (iterations != m_maxIterations) {
              fastInterceptFinder(hits, x1_d, x2_d, y1_d, y2_d, iterations + 1, ix, iy);
            } else {
              (*m_houghPlane)[ix - (m_nCells - m_nCellsPhi) / 2][iy - (m_nCells - m_nCellsR) / 2] = nSL;
            }
            break;
          }
        }
      }
    }
  }

  return 0;
}

/*
* Peak finding method: connected regions & center of gravity
*/
void
CDCTrigger2DFinderModule::connectedRegions()
{
  std::vector<std::vector<CDCTriggerHoughCand>> regions;
  std::vector<CDCTriggerHoughCand> cpyCand = m_houghCand;

  // debug: print candidate list
  B2DEBUG(50, "m_houghCand number " << cpyCand.size());
  for (unsigned icand = 0; icand < m_houghCand.size(); ++icand) {
    Coord2dPair coord = m_houghCand[icand].getCoord();
    B2DEBUG(100, m_houghCand[icand].getID()
            << " nSL " << m_houghCand[icand].getSLcount()
            << " x1 " << coord.first.X() << " x2 " << coord.second.X()
            << " y1 " << coord.first.Y() << " y2 " << coord.second.Y());
  }

  regions.reserve(cpyCand.size());
  // combine connected cells to regions
  while (cpyCand.size() > 0) {
    B2DEBUG(100, "make new region");
    std::vector<CDCTriggerHoughCand> region;
    std::vector<CDCTriggerHoughCand> rejected;
    region.reserve(cpyCand.size());
    rejected.reserve(cpyCand.size());
    CDCTriggerHoughCand start = cpyCand[0];
    cpyCand.erase(cpyCand.begin());
    region.push_back(start);
    addNeighbors(start, cpyCand, region, rejected, start.getSLcount());
    regions.push_back(region);
    for (auto cand = cpyCand.begin(); cand != cpyCand.end();) {
      if (inList(*cand, region) || inList(*cand, rejected))
        cpyCand.erase(cand);
      else
        ++cand;
    }
  }

  std::vector<ROOT::Math::XYVector> cellIds = {};

  // find center of gravity for each region
  for (unsigned ir = 0; ir < regions.size(); ++ir) {
    B2DEBUG(50, "region " << ir << " (" << regions[ir].size() << " cells).");
    // skip regions with size below cut
    if (regions[ir].size() < m_minCells) {
      B2DEBUG(50, "Skipping region with " << regions[ir].size() << " cells.");
      continue;
    }
    double xfirst = regions[ir][0].getCoord().first.X();
    double x = 0;
    double y = 0;
    int n = 0;
    std::vector<unsigned> mergedList;
    int xmin = m_nCellsPhi;
    int xmax = 0;
    int ymin = m_nCellsR;
    int ymax = 0;
    cellIds.clear();
    cellIds.reserve(regions[ir].size());
    for (unsigned ir2 = 0; ir2 < regions[ir].size(); ++ir2) {
      Coord2dPair hc = regions[ir][ir2].getCoord();
      B2DEBUG(100, "  " << regions[ir][ir2].getID()
              << " nSL " << regions[ir][ir2].getSLcount()
              << " x1 " << hc.first.X() << " x2 " << hc.second.X()
              << " y1 " << hc.first.Y() << " y2 " << hc.second.Y());
      int ix = floor((hc.first.X() + M_PI) / 2. / M_PI * m_nCellsPhi + 0.5);
      int iy = floor((hc.first.Y() + m_maxR - m_shiftR) / 2. / m_maxR * m_nCellsR + 0.5);
      x += (hc.first.X() + hc.second.X());
      if (xfirst - hc.first.X() > M_PI) {
        x += 4 * M_PI;
        ix += m_nCellsPhi;
      } else if (hc.first.X() - xfirst > M_PI) {
        x -= 4 * M_PI;
        ix -= m_nCellsPhi;
      }
      y += (hc.first.Y() + hc.second.Y());
      n += 1;
      std::vector<unsigned> idList = regions[ir][ir2].getIdList();
      mergeIdList(mergedList, mergedList, idList);
      xmin = std::min(xmin, ix);
      xmax = std::max(xmax, ix);
      ymin = std::min(ymin, iy);
      ymax = std::max(ymax, iy);
      cellIds.push_back(ROOT::Math::XYVector(ix, iy));
    }
    x *= 0.5 / n;
    if (x > M_PI)
      x -= 2 * M_PI;
    else if (x <= -M_PI)
      x += 2 * M_PI;
    y *= 0.5 / n;
    B2DEBUG(50, "x " << x << " y " << y);

    // select 1 hit per super layer
    std::vector<unsigned> selectedList = {};
    std::vector<unsigned> unselectedList = {};
    selectHits(mergedList, selectedList, unselectedList);

    // save track
    const CDCTriggerTrack* track =
      m_tracks.appendNew(x, 2. * y, 0.);
    // relations to selected hits
    for (unsigned i = 0; i < selectedList.size(); ++i) {
      unsigned its = selectedList[i];
      track->addRelationTo(m_segmentHits[its]);
    }
    // relations to additional hits get a negative weight
    for (unsigned i = 0; i < unselectedList.size(); ++i) {
      unsigned its = unselectedList[i];
      track->addRelationTo(m_segmentHits[its], -1.);
    }
    // save detail information about the cluster
    const CDCTriggerHoughCluster* cluster =
      m_clusters.appendNew(xmin, xmax, ymin, ymax, cellIds);
    track->addRelationTo(cluster);
  }
}

void
CDCTrigger2DFinderModule::addNeighbors(const CDCTriggerHoughCand& center,
                                       const std::vector<CDCTriggerHoughCand>& candidates,
                                       std::vector<CDCTriggerHoughCand>& merged,
                                       std::vector<CDCTriggerHoughCand>& rejected,
                                       unsigned short nSLmax) const
{
  for (unsigned icand = 0; icand < candidates.size(); ++icand) {
    B2DEBUG(120, "compare center " << center.getID()
            << " to " << candidates[icand].getID());
    if (inList(candidates[icand], merged) || inList(candidates[icand], rejected)) {
      B2DEBUG(120, "  " << candidates[icand].getID() << " already in list");
      continue;
    }
    bool reject = inList(center, rejected);
    if (connected(center, candidates[icand])) {
      if (m_onlyLocalMax && candidates[icand].getSLcount() < nSLmax) {
        B2DEBUG(100, "  lower than highest SLcount, rejected");
        rejected.push_back(candidates[icand]);
      } else if (m_onlyLocalMax && !reject && candidates[icand].getSLcount() > nSLmax) {
        B2DEBUG(100, "  new highest SLcount, clearing list");
        nSLmax = candidates[icand].getSLcount();
        for (unsigned imerged = 0; imerged < merged.size(); ++imerged) {
          rejected.push_back(merged[imerged]);
        }
        merged.clear();
        merged.push_back(candidates[icand]);
      } else if (m_onlyLocalMax && candidates[icand].getSLcount() > center.getSLcount()) {
        B2DEBUG(100, "  connected to rejected cell, skip");
        continue;
      } else if (m_onlyLocalMax && reject) {
        B2DEBUG(100, "  connected to rejected cell, rejected");
        rejected.push_back(candidates[icand]);
      } else {
        B2DEBUG(100, "  connected");
        merged.push_back(candidates[icand]);
      }
      std::vector<CDCTriggerHoughCand> cpyCand = candidates;
      cpyCand.erase(cpyCand.begin() + icand);
      addNeighbors(candidates[icand], cpyCand, merged, rejected, nSLmax);
    }
  }
}

bool
CDCTrigger2DFinderModule::inList(const CDCTriggerHoughCand& a,
                                 const std::vector<CDCTriggerHoughCand>& list) const
{
  for (unsigned i = 0; i < list.size(); ++i) {
    if (a == list[i]) return true;
  }
  return false;
}

bool
CDCTrigger2DFinderModule::connected(const CDCTriggerHoughCand& a,
                                    const CDCTriggerHoughCand& b) const
{
  double ax1 = a.getCoord().first.X();
  double ax2 = a.getCoord().second.X();
  double ay1 = a.getCoord().first.Y();
  double ay2 = a.getCoord().second.Y();
  double bx1 = b.getCoord().first.X();
  double bx2 = b.getCoord().second.X();
  double by1 = b.getCoord().first.Y();
  double by2 = b.getCoord().second.Y();
  // direct neighbors
  bool direct = ((ax2 == bx1 && ay1 == by1) || // right
                 (ax1 == bx2 && ay1 == by1) || // left
                 (ax1 == bx1 && ay2 == by1) || // above
                 (ax1 == bx1 && ay1 == by2) || // below
                 (ax1 + 2. * M_PI == bx2 && ay1 == by1) ||
                 (ax2 == bx1 + 2. * M_PI && ay1 == by1));
  // diagonal connections
  bool diagRise = ((ax2 == bx1 && ay2 == by1) || // right above
                   (ax1 == bx2 && ay1 == by2) || // left below
                   (ax1 + 2. * M_PI == bx2 && ay1 == by2) ||
                   (ax2 == bx1 + 2. * M_PI && ay2 == by1));
  bool diagFall = ((ax1 == bx2 && ay2 == by1) || // left above
                   (ax2 == bx1 && ay1 == by2) || // right below
                   (ax2 == bx1 + 2. * M_PI && ay1 == by2) ||
                   (ax1 + 2. * M_PI == bx2 && ay2 == by1));
  if (m_connect == 4) return direct;
  else if (m_connect == 6) return (direct || diagRise);
  else if (m_connect == 8) return (direct || diagRise || diagFall);
  else B2WARNING("Unknown option for connect " << m_connect << ", using default.");
  return (direct || diagRise);
}

/*
* Merge Id lists.
*/
void
CDCTrigger2DFinderModule::mergeIdList(std::vector<unsigned>& merged,
                                      std::vector<unsigned>& a,
                                      std::vector<unsigned>& b)
{
  bool found;

  merged.reserve(a.size() + b.size());
  for (auto it = a.begin(); it != a.end(); ++it) {
    found = false;
    for (auto it_in = merged.begin(); it_in != merged.end(); ++it_in) {
      if (*it_in == *it) {
        found = true;
        break;
      }
    }
    if (!found) {
      merged.push_back(*it);
    }
  }

  for (auto it = b.begin(); it != b.end(); ++it) {
    found = false;
    for (auto it_in = merged.begin(); it_in != merged.end(); ++it_in) {
      if (*it_in == *it) {
        found = true;
        break;
      }
    }
    if (!found) {
      merged.push_back(*it);
    }
  }
}

void
CDCTrigger2DFinderModule::findAllCrossingHits(std::vector<unsigned>& list,
                                              double x1, double x2,
                                              double y1, double y2,
                                              const CDCHitMap& inputMap)
{
  double m, a, y1_h, y2_h;

  const double sin_x1 = std::sin(x1 - 1e-10);
  const double cos_x1 = std::cos(x1 - 1e-10);
  const double sin_x2 = std::sin(x2 + 1e-10);
  const double cos_x2 = std::cos(x2 + 1e-10);

  std::array<double, 3> phi = {0, 0, 0};

  for (int iHit = 0; iHit < m_segmentHits.getEntries(); iHit++) {
    unsigned short iSL = m_segmentHits[iHit]->getISuperLayer();
    if (iSL % 2) continue;
    // associate only the hits actually used to form the cluster
    if (m_suppressClone && inputMap.find(iHit) == inputMap.end()) continue;
    //TODO: add options: center cell / active priority cell / all priority cells
    phi[0] = m_segmentHits[iHit]->getSegmentID() - m_TSoffset[iSL];
    phi[1] = phi[0] + 0.5;
    phi[2] = phi[0] - 0.5;
    const auto& radiusISL = m_radius[iSL];
    for (unsigned i = 0; i < 3; ++i) {
      phi[i] *= 2. * M_PI / (m_TSoffset[iSL + 1] - m_TSoffset[iSL]);
      m = std::cos(phi[i]) / radiusISL[std::min(i, 1u)];
      a = std::sin(phi[i]) / radiusISL[std::min(i, 1u)];
      // calculate Hough curve with slightly enlarged limits to avoid errors due to rounding
      y1_h = m * sin_x1 - a * cos_x1;
      y2_h = m * sin_x2 - a * cos_x2;
      // skip decreasing half of the sine (corresponds to curl back half of the track)
      if (y1_h > y2_h) continue;
      if (!((y1_h > y2 && y2_h > y2) || (y1_h < y1 && y2_h < y1))) {
        list.push_back(iHit);
        break;
      }
    }
  }
}

/*
 * Select one hit per super layer
 */
void
CDCTrigger2DFinderModule::selectHits(std::vector<unsigned>& list,
                                     std::vector<unsigned>& selected,
                                     std::vector<unsigned>& unselected)
{
  std::array<int, 5> bestPerSL;
  bestPerSL.fill(-1);
  for (unsigned i = 0; i < list.size(); ++i) {
    unsigned iax = m_segmentHits[list[i]]->getISuperLayer() / 2;
    bool firstPriority = (m_segmentHits[list[i]]->getPriorityPosition() == 3);
    if (bestPerSL[iax] < 0) {
      bestPerSL[iax] = i;
    } else {
      unsigned itsBest = list[bestPerSL[iax]];
      bool firstBest = (m_segmentHits[itsBest]->getPriorityPosition() == 3);
      // selection rules:
      // first priority, higher ID
      if ((firstPriority && !firstBest) ||
          (firstPriority == firstBest &&
           m_segmentHits[list[i]]->getSegmentID() > m_segmentHits[itsBest]->getSegmentID())) {
        bestPerSL[iax] = i;
      }
    }
  }

  for (unsigned i = 0; i < list.size(); ++i) {
    unsigned iax = m_segmentHits[list[i]]->getISuperLayer() / 2;
    if (int(i) == bestPerSL[iax]) selected.push_back(list[i]);
    else unselected.push_back(list[i]);
  }
}

/*
* Alternative peak finding with nested patterns
*/
void
CDCTrigger2DFinderModule::patternClustering(const CDCHitMap& inputMap)
{
  // fill a matrix of 2 x 2 squares
  TMatrix plane2(m_nCellsPhi / 2, m_nCellsR / 2);
  TMatrix planeIcand(m_nCellsPhi, m_nCellsR);
  for (unsigned icand = 0; icand < m_houghCand.size(); ++icand) {
    double x = (m_houghCand[icand].getCoord().first.X() +
                m_houghCand[icand].getCoord().second.X()) / 2.;
    unsigned ix = floor((x + M_PI) / 2. / M_PI * m_nCellsPhi);
    double y = (m_houghCand[icand].getCoord().first.Y() +
                m_houghCand[icand].getCoord().second.Y()) / 2.;
    unsigned iy = floor((y + m_maxR - m_shiftR) / 2. / m_maxR * m_nCellsR);
    plane2[ix / 2][iy / 2] += 1 << ((ix % 2) + 2 * (iy % 2));
    planeIcand[ix][iy] = icand;
    B2DEBUG(100, "candidate " << icand << " at ix " << ix << " iy " << iy);
  }
  // look for clusters of 2 x 2 squares in a (rX x rY) region
  unsigned nX = m_nCellsPhi / 2;
  unsigned nY = m_nCellsR / 2;
  unsigned rX = m_clusterSizeX;
  unsigned rY = m_clusterSizeY;
  std::vector<unsigned> pattern(rX * rY, 0);
  std::vector<ROOT::Math::XYVector> cellIds;
  cellIds.reserve(rX * rY);
  for (unsigned ix = 0; ix < nX; ++ix) {
    for (unsigned iy = 0; iy < nY; ++iy) {
      if (!plane2[ix][iy]) continue;
      // check if we are in a lower left corner
      unsigned ileft = (ix - 1 + nX) % nX;
      B2DEBUG(100, "ix " << ix << " iy " << iy);
      if (connectedLR(plane2[ileft][iy], plane2[ix][iy]) ||
          (iy > 0 && connectedUD(plane2[ix][iy - 1], plane2[ix][iy])) ||
          (iy > 0 && connectedDiag(plane2[ileft][iy - 1], plane2[ix][iy]))) {
        B2DEBUG(100, "skip connected square");
        continue;
      }
      // form cluster
      pattern.assign(rX * rY, 0);   // reset pattern to 0
      pattern[0] = plane2[ix][iy];
      cellIds.clear();
      cellIds.emplace_back(ROOT::Math::XYVector(2 * ix, 2 * iy));
      for (unsigned ix2 = 0; ix2 < rX; ++ix2) {
        for (unsigned iy2 = 0; iy2 < rY; ++iy2) {
          if (iy + iy2 >= nY) continue;
          unsigned ip = ix2 + rX * iy2;
          unsigned iright = (ix + ix2 + nX) % nX;
          ileft = (iright - 1 + nX) % nX;
          B2DEBUG(100, "ix2 " << ix2 << " ileft " << ileft << " iright " << iright);
          if ((ix2 > 0 && // check left/right connection
               pattern[ip - 1] &&
               connectedLR(plane2[ileft][iy + iy2], plane2[iright][iy + iy2])) ||
              (iy2 > 0 && // check up/down connection
               pattern[ip - rX] &&
               connectedUD(plane2[iright][iy + iy2 - 1], plane2[iright][iy + iy2])) ||
              (ix2 > 0 && iy2 > 0 && // check diagonal connection
               pattern[ip - rX - 1] &&
               connectedDiag(plane2[ileft][iy + iy2 - 1], plane2[iright][iy + iy2]))) {
            pattern[ip] = plane2[iright][iy + iy2];
            B2DEBUG(100, "connect cell " << iright << " " << iy + iy2);
            cellIds.push_back(ROOT::Math::XYVector(2 * (ix + ix2), 2 * (iy + iy2)));
          }
        }
      }
      B2DEBUG(100, "cluster starting at " << ix << " " << iy);
      // check if cluster continues beyond defined area
      bool overflowRight = false;
      bool overflowTop = false;
      for (unsigned iy2 = 0; iy2 < rY; ++iy2) {
        unsigned ip = rX - 1 + rX * iy2;
        if (!pattern[ip]) continue;
        unsigned iright = (ix + rX + nX) % nX;
        ileft = (iright - 1 + nX) % nX;
        if (connectedLR(plane2[ileft][iy + iy2], plane2[iright][iy + iy2]) ||
            ((iy + iy2 + 1 < nY) &&
             connectedDiag(plane2[ileft][iy + iy2], plane2[iright][iy + iy2 + 1]))) {
          setReturnValue(false);
          overflowRight = true;
        }
      }
      if (iy + rY < nY) {
        for (unsigned ix2 = 0; ix2 < rX; ++ix2) {
          unsigned ip = ix2 + rX * (rY - 1);
          if (!pattern[ip]) continue;
          unsigned iright = (ix + ix2 + 1 + nX) % nX;
          ileft = (iright - 1 + nX) % nX;
          if (connectedUD(plane2[ileft][iy + rY - 1], plane2[ileft][iy + rY]) ||
              connectedDiag(plane2[ileft][iy + rY - 1], plane2[iright][iy + rY])) {
            setReturnValue(false);
            overflowTop = true;
          }
        }
      }
      if (overflowRight && !overflowTop) {
        B2DEBUG(100, "cluster extends right of " << rX << " x " << rY << " area");
      } else if (overflowTop && !overflowRight) {
        B2DEBUG(100, "cluster extends above " << rX << " x " << rY << " area");
      } else if (overflowRight && overflowTop) {
        B2DEBUG(100, "cluster extends right and above " << rX << " x " << rY << " area");
      }
      // find corners of cluster
      unsigned topRight2 = topRightSquare(pattern);
      unsigned topRight = topRightCorner(pattern[topRight2]);
      unsigned bottomLeft = bottomLeftCorner(pattern[0]);
      B2DEBUG(100, "topRight2 " << topRight2 << " topRight " << topRight << " bottomLeft " << bottomLeft);
      // average over corners to find cluster center
      unsigned ixTR = 2 * (topRight2 % m_clusterSizeX) + (topRight % 2);
      unsigned ixBL = bottomLeft % 2;
      unsigned iyTR = 2 * (topRight2 / m_clusterSizeX) + (topRight / 2);
      unsigned iyBL = bottomLeft / 2;
      B2DEBUG(100, "ixTR " << ixTR << " ixBL " << ixBL << " iyTR " << iyTR << " iyBL " << iyBL);
      // skip size 1 clusters
      if (m_minCells > 1 && ixTR == ixBL && iyTR == iyBL) {
        B2DEBUG(100, "skipping cluster of size 1");
        continue;
      }
      float centerX = 2 * ix + (ixTR + ixBL) / 2.;
      if (centerX >= m_nCellsPhi) centerX -= m_nCellsPhi;
      float centerY = 2 * iy + (iyTR + iyBL) / 2.;
      B2DEBUG(100, "center at cell (" << centerX << ", " << centerY << ")");
      // convert to coordinates
      double x = -M_PI + (centerX + 0.5) * 2. * M_PI / m_nCellsPhi;
      double y = -m_maxR + m_shiftR + (centerY + 0.5) * 2. * m_maxR / m_nCellsR;
      B2DEBUG(100, "center coordinates (" << x << ", " << y << ")");
      // get list of related hits
      std::vector <unsigned> idList = {};
      if (m_hitRelationsFromCorners) {
        unsigned icandTR = planeIcand[(ixTR + 2 * ix) % m_nCellsPhi][iyTR + 2 * iy];
        unsigned icandBL = planeIcand[ixBL + 2 * ix][iyBL + 2 * iy];
        std::vector<unsigned> candIdListTR = m_houghCand[icandTR].getIdList();
        std::vector<unsigned> candIdListBL = m_houghCand[icandBL].getIdList();
        mergeIdList(idList, candIdListTR, candIdListBL);
        B2DEBUG(100, "merge id lists from candidates " << icandTR << " and " << icandBL);
      } else {
        double dx = M_PI / m_nCellsPhi;
        double dy = m_maxR / m_nCellsR;
        double x1 = (round(centerX) == centerX) ? x - dx : x - 2 * dx;
        double x2 = (round(centerX) == centerX) ? x + dx : x + 2 * dx;
        double y1 = (round(centerY) == centerY) ? y - dy : y - 2 * dy;
        double y2 = (round(centerY) == centerY) ? y + dy : y + 2 * dy;
        findAllCrossingHits(idList, x1, x2, y1, y2, inputMap);
      }
      if (idList.size() == 0) {
        setReturnValue(false);
        B2DEBUG(100, "id list empty");
      }

      // select 1 hit per super layer
      std::vector<unsigned> selectedList = {};
      std::vector<unsigned> unselectedList = {};
      selectHits(idList, selectedList, unselectedList);

      // save track
      const CDCTriggerTrack* track =
        m_tracks.appendNew(x, 2. * y, 0.);
      // relations to selected hits
      for (unsigned i = 0; i < selectedList.size(); ++i) {
        unsigned its = selectedList[i];
        track->addRelationTo(m_segmentHits[its]);
      }
      // relations to additional hits get a negative weight
      for (unsigned i = 0; i < unselectedList.size(); ++i) {
        unsigned its = unselectedList[i];
        track->addRelationTo(m_segmentHits[its], -1.);
      }
      // save detail information about the cluster
      const CDCTriggerHoughCluster* cluster =
        m_clusters.appendNew(2 * ix, 2 * (ix + m_clusterSizeX) - 1,
                             2 * iy, 2 * (iy + m_clusterSizeY) - 1,
                             cellIds);
      track->addRelationTo(cluster);
    }
  }
}

/*
* connection definitions for 2 x 2 squares
*/
bool
CDCTrigger2DFinderModule::connectedLR(unsigned patternL, unsigned patternR)
{
  // connected if
  // . x | x .  or  . . | . .
  // . . | . .      . x | x .
  bool connectDirect = (((patternL >> 3) & 1) && ((patternR >> 2) & 1)) ||
                       (((patternL >> 1) & 1) && ((patternR >> 0) & 1));
  // connected if
  // . . | x .
  // . x | . .
  bool connectRise = ((patternL >> 1) & 1) && ((patternR >> 2) & 1);
  // connected if
  // . x | . .
  // . . | x .
  bool connectFall = ((patternL >> 3) & 1) && ((patternR >> 0) & 1);

  if (m_connect == 4) return connectDirect;
  else if (m_connect == 6) return (connectDirect || connectRise);
  else if (m_connect == 8) return (connectDirect || connectRise || connectFall);
  else B2WARNING("Unknown option for connect " << m_connect << ", using default.");
  return (connectDirect || connectRise);
}

bool
CDCTrigger2DFinderModule::connectedUD(unsigned patternD, unsigned patternU)
{
  // connected if
  // . .      . .
  // x .      . x
  // ---  or  ---
  // x .      . x
  // . .      . .
  bool connectDirect = (((patternU >> 0) & 1) && ((patternD >> 2) & 1)) ||
                       (((patternU >> 1) & 1) && ((patternD >> 3) & 1));
  // connected if
  // . .
  // . x
  // ---
  // x .
  // . .
  bool connectRise = ((patternU >> 1) & 1) && ((patternD >> 2) & 1);
  // connected if
  // . .
  // x .
  // ---
  // . x
  // . .
  bool connectFall = ((patternU >> 0) & 1) && ((patternD >> 3) & 1);

  if (m_connect == 4) return connectDirect;
  else if (m_connect == 6) return (connectDirect || connectRise);
  else if (m_connect == 8) return (connectDirect || connectRise || connectFall);
  else B2WARNING("Unknown option for connect " << m_connect << ", using default.");
  return (connectDirect || connectRise);
}

bool
CDCTrigger2DFinderModule::connectedDiag(unsigned patternLD, unsigned patternRU)
{
  if (m_connect == 4) return false;

  // connected if
  //     . .
  //     x .
  // . x
  // . .
  return (((patternRU >> 0) & 1) && ((patternLD >> 3) & 1));
}

unsigned
CDCTrigger2DFinderModule::topRightSquare(std::vector<unsigned>& pattern)
{
  // scan from top right corner until an active square is found
  for (unsigned index = pattern.size() - 1; index > 0; --index) {
    if (!pattern[index]) continue;
    // check for ambiguity
    unsigned ix = index % m_clusterSizeX;
    unsigned iy = index / m_clusterSizeX;
    if (ix < m_clusterSizeX - 1 && iy > 0) {
      bool unique = true;
      for (unsigned index2 = index - 1; index2 > 0; --index2) {
        if (!pattern[index2]) continue;
        unsigned ix2 = index2 % m_clusterSizeX;
        unsigned iy2 = index2 / m_clusterSizeX;
        if (iy2 < iy && ix2 > ix) {
          unique = false;
          break;
        }
      }
      if (!unique) {
        setReturnValue(false);
        B2DEBUG(100, "topRightSquare not unique");
      }
    }
    return index;
  }
  return 0;
}

unsigned
CDCTrigger2DFinderModule::topRightCorner(unsigned pattern)
{
  // scan pattern from right to left:
  // 2 3
  // 0 1
  if ((pattern >> 3) & 1) return 3;
  if ((pattern >> 1) & 1) {
    if ((pattern >> 2) & 1) {
      setReturnValue(false);
      B2DEBUG(100, "topRightCorner not unique");
    }
    return 1;
  }
  if ((pattern >> 2) & 1) return 2;
  return 0;
}

unsigned
CDCTrigger2DFinderModule::bottomLeftCorner(unsigned pattern)
{
  // scan pattern from left to right:
  // 2 3
  // 0 1
  if (pattern & 1) return 0;
  if ((pattern >> 2) & 1) {
    if ((pattern >> 1) & 1) {
      setReturnValue(false);
      B2DEBUG(100, "bottomLeftCorner not unique");
    }
    return 2;
  }
  if ((pattern >> 1) & 1) return 1;
  return 3;
}
