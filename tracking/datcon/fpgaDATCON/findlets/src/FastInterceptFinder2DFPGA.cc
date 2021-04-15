/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/fpgaDATCON/findlets/FastInterceptFinder2DFPGA.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <vxd/dataobjects/VxdID.h>

#include <framework/core/ModuleParamList.h>
#include <framework/core/ModuleParamList.templateDetails.h>
// #include <iostream>

using namespace Belle2;
using namespace TrackFindingCDC;

FastInterceptFinder2DFPGA::FastInterceptFinder2DFPGA() : Super()
{
}

void FastInterceptFinder2DFPGA::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "isUFinder"),
                                m_param_isUFinder,
                                "Intercept finder for u-side or v-side?",
                                m_param_isUFinder);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "writeGnuplotOutput"),
                                m_writeGnuplotOutput,
                                "Write gnuplot debugging output to file?",
                                m_writeGnuplotOutput);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "gnuplotHSOutputFileName"),
                                m_gnuplotHSOutputFileName,
                                "Name of the gnuplot debug file.",
                                m_gnuplotHSOutputFileName);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "gnuplotHSRectOutputFileName"),
                                m_gnuplotHSRectOutputFileName,
                                "Name of the gnuplot debug HS sectors file.",
                                m_gnuplotHSRectOutputFileName);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "gnuplotHSCoGOutputFileName"),
                                m_gnuplotHSCoGOutputFileName,
                                "Name of the gnuplot debug cluster CoG file.",
                                m_gnuplotHSCoGOutputFileName);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumRecursionLevel"),
                                m_maxRecursionLevel,
                                "Maximum recursion level for the fast Hough trafo algorithm.",
                                m_maxRecursionLevel);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "nAngleSectors"),
                                m_nAngleSectors,
                                "Number of angle sectors (= x-axis) dividing the Hough space.",
                                m_nAngleSectors);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "nVerticalSectors"),
                                m_nVerticalSectors,
                                "Number of vertical sectors (= y-axis) dividing the Hough space.",
                                m_nVerticalSectors);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "verticalHoughSpaceSize"),
                                m_verticalHoughSpaceSize,
                                "data type: long. Vertical size of the Hough space.",
                                m_verticalHoughSpaceSize);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minimumX"),
                                m_minimumX,
                                "Minimum x value of the Hough space.",
                                m_minimumX);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumX"),
                                m_maximumX,
                                "Maximum x value of the Hough space.",
                                m_maximumX);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minimumHSClusterSize"),
                                m_MinimumHSClusterSize,
                                "Maximum x value of the Hough space.",
                                m_MinimumHSClusterSize);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumHSClusterSize"),
                                m_MaximumHSClusterSize,
                                "Maximum x value of the Hough space.",
                                m_MaximumHSClusterSize);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumHSClusterSizeX"),
                                m_MaximumHSClusterSizeX,
                                "Maximum x value of the Hough space.",
                                m_MaximumHSClusterSizeX);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumHSClusterSizeY"),
                                m_MaximumHSClusterSizeY,
                                "Maximum x value of the Hough space.",
                                m_MaximumHSClusterSizeY);

}

void FastInterceptFinder2DFPGA::initialize()
{
  Super::initialize();

  m_maxRecursionLevel = ceil(log2(std::max(m_nAngleSectors, m_nVerticalSectors))) - 1;
  B2ASSERT("The maximum number of currentRecursion in u must not be larger than 14, but it is " << m_maxRecursionLevel,
           m_maxRecursionLevel <= 14);
  m_unitX = (m_maximumX - m_minimumX) / m_nAngleSectors;
  if (not m_param_isUFinder) {
    m_unitX = (tan(m_maximumX) - tan(m_minimumX)) / m_nAngleSectors;
  }
  for (uint i = 0; i < m_nAngleSectors; i++) {
    double x = m_minimumX + m_unitX * (double)i;
    double xc = x + 0.5 * m_unitX;
    if (not m_param_isUFinder) {
      x = atan(tan(m_minimumX) + m_unitX * i);
      xc = atan(tan(m_minimumX) + m_unitX * ((double)i + 0.5));
    }

    m_HSXLUT[i] = x;
    m_HSSinValuesLUT[i] = convertToInt(sin(x), 3);
    m_HSCosValuesLUT[i] = convertToInt(cos(x), 3);
    m_HSCenterSinValuesLUT[i] = convertToInt(sin(xc), 3);
    m_HSCenterCosValuesLUT[i] = convertToInt(cos(xc), 3);
    m_HSXCenterLUT[i] = xc;
  }
  m_HSXLUT[m_nAngleSectors] = m_maximumX;
  m_HSSinValuesLUT[m_nAngleSectors] = convertToInt(sin(m_maximumX), 3);
  m_HSCosValuesLUT[m_nAngleSectors] = convertToInt(cos(m_maximumX), 3);

  m_unitY = 2. * m_verticalHoughSpaceSize / m_nVerticalSectors;
  for (uint i = 0; i <= m_nVerticalSectors; i++) {
    m_HSYLUT[i] = m_verticalHoughSpaceSize - m_unitY * i;
    m_HSYCenterLUT[i] = m_verticalHoughSpaceSize - m_unitY * i - 0.5 * m_unitY;
  }
  B2DEBUG(29, "HS size x: " << (m_maximumX - m_minimumX) << " HS size y: " << m_verticalHoughSpaceSize << " unitX: " << m_unitX <<
          " unitY: " << m_unitY);
}

void FastInterceptFinder2DFPGA::apply(std::vector<std::pair<VxdID, std::pair<long, long>>>& hits,
                                      std::vector<std::pair<double, double>>& tracks)
{
  m_SectorArray.assign(m_nAngleSectors * m_nVerticalSectors, 0);
  m_activeSectorArray.clear();
  m_activeSectorArray.reserve(4096);
  m_trackCandidates.clear();

  if (m_writeGnuplotOutput) {
    m_rectcounter = 1;
    m_rectoutstream.open(m_gnuplotHSRectOutputFileName.c_str(), std::ios::trunc);
    m_cogoutstream.open(m_gnuplotHSCoGOutputFileName.c_str(), std::ios::trunc);
    gnuplotoutput(hits);
  }

  fastInterceptFinder2d(hits, 0, m_nAngleSectors, 0, m_nVerticalSectors, 0);
//   for (uint y = 0; y < m_nVerticalSectors; y++) {
//     for (uint x = 0; x < m_nAngleSectors; x++) {
//       short cellContent = m_SectorArray[y * m_nAngleSectors + x];
//       if (cellContent < -1) {
//         std::cout << "-" << abs(cellContent) << " ";
//       } else if (cellContent >= 0 && cellContent < 10) {
//         std::cout << " " <<  cellContent << " ";
//       } else if (cellContent >= 10) {
//         std::cout << cellContent << " ";
//       }
//     }
//     std::cout << std::endl;
//   }
  FindHoughSpaceCluster();
//   for (uint y = 0; y < m_nVerticalSectors; y++) {
//     for (uint x = 0; x < m_nAngleSectors; x++) {
//       short cellContent = m_SectorArray[y * m_nAngleSectors + x];
//       if (cellContent < -1) {
//         std::cout << "-" << abs(cellContent) << " ";
//       } else if (cellContent >= 0 && cellContent < 10) {
//         std::cout << " " <<  cellContent << " ";
//       } else if (cellContent >= 10) {
//         std::cout << cellContent << " ";
//       }
//     }
//     std::cout << std::endl;
//   }
//   std::cout << std::endl;

  for (auto& trackCand : m_trackCandidates) {
    tracks.emplace_back(trackCand);
  }

  B2DEBUG(29, "m_activeSectorArray.size: " << m_activeSectorArray.size() << " m_trackCandidates.size: " << m_trackCandidates.size());

  if (m_writeGnuplotOutput) {
    m_rectoutstream.close();
    m_cogoutstream.close();
  }

}

void FastInterceptFinder2DFPGA::fastInterceptFinder2d(std::vector<std::pair<VxdID, std::pair<long, long>>>& hits,
                                                      uint xmin, uint xmax, uint ymin, uint ymax, uint currentRecursion)
{
  std::vector<std::pair<VxdID, std::pair<long, long>>> containedHits;

  if (currentRecursion == m_maxRecursionLevel + 1) return;

  // these int-divisions can cause {min, center} or {center, max} to be the same, which is a desired behaviour
  const uint centerx = xmin + (uint)((xmax - xmin) / 2);
  const uint centery = ymin + (uint)((ymax - ymin) / 2);
  uint xIndexCache[3] = {xmin, centerx, xmax};
  uint yIndexCache[3] = {ymin, centery, ymax};

  for (int i = 0; i < 2 ; ++i) {
    const uint left  = xIndexCache[i];
    const uint right = xIndexCache[i + 1];
    const uint localIndexX = left;

    if (left == right) continue;

    const double& localLeft   = m_HSXLUT[left];
    const double& localRight  = m_HSXLUT[right];
    const short&  sinLeft     = m_HSSinValuesLUT[left];
    const short&  cosLeft     = m_HSCosValuesLUT[left];
    const short&  sinRight    = m_HSSinValuesLUT[right];
    const short&  cosRight    = m_HSCosValuesLUT[right];

    // the sin and cos of the current center can't be stored in a LUT, as the number of possible centers
    // is quite large and the logic would become rather complex
//     const short sinCenter   = convertToInt(sin((localLeft + localRight) / 2.), 3);
//     const short cosCenter   = convertToInt(cos((localLeft + localRight) / 2.), 3);
    const short&  sinCenter   = m_HSCenterSinValuesLUT[(left + right) / 2];
    const short&  cosCenter   = m_HSCenterCosValuesLUT[(left + right) / 2];

    for (int j = 0; j < 2; ++j) {

      const uint lowerIndex = yIndexCache[j];
      const uint upperIndex = yIndexCache[j + 1];

      const uint localIndexY = lowerIndex;
      const long& localUpperCoordinate = m_HSYLUT[lowerIndex];
      const long& localLowerCoordinate = m_HSYLUT[upperIndex];

      if (lowerIndex == upperIndex) continue;

      std::vector<bool> layerHits(7); /* For layer filter */
      containedHits.clear();
      for (auto& hit : hits) {
        const VxdID& sensor = hit.first;

        const long& m = hit.second.first;
        const long& a = hit.second.second;

        long yLeft   = m * cosLeft   + a * sinLeft;
        long yRight  = m * cosRight  + a * sinRight;
        long yCenter = m * cosCenter + a * sinCenter;
        long derivativeyLeft   = m * -sinLeft   + a * cosLeft;
        long derivativeyRight  = m * -sinRight  + a * cosRight;
        long derivativeyCenter = m * -sinCenter + a * cosCenter;

        // Only interested in the rising arm of the sinosoidal curves.
        // Thus if derivative on both sides of the cell is negative, ignore and continue.
        if (derivativeyLeft < 0 and derivativeyRight < 0 and derivativeyCenter < 0) continue;

        /* Check if HS-parameter curve is inside (or outside) actual sub-HS */
//         if (((yLeft <= localUpperCoordinate && yRight >= localLowerCoordinate) ||
//              (yCenter <= localUpperCoordinate && yCenter >= localLowerCoordinate && derivativeyCenter >= 0)) &&
//             (derivativeyLeft >= 0 || derivativeyRight >= 0 || derivativeyCenter >= 0)) {
        if ((yLeft <= localUpperCoordinate and yRight >= localLowerCoordinate) or
            (yCenter <= localUpperCoordinate and yLeft >= localLowerCoordinate and yRight >= localLowerCoordinate) or
            (yCenter >= localLowerCoordinate and yLeft <= localUpperCoordinate and yRight <= localUpperCoordinate)) {
          layerHits[sensor.getLayerNumber()] = true; /* layer filter */
          containedHits.emplace_back(hit);
        }
      }

      if (layerFilter(layerHits) > 0) {
        // recursive call of fastInterceptFinder2d, until currentRecursion == m_maxRecursionLevel
        if (currentRecursion < m_maxRecursionLevel) {

          m_rectoutstream << "set object " << m_rectcounter << " rect from " << localLeft << ", " << localLowerCoordinate <<
                          " to " << localRight << ", " << localUpperCoordinate << " fc rgb \"" <<
                          m_const_rectColor[currentRecursion % 8] << "\" fs solid 0.5 behind" << std::endl;
          m_rectcounter++;

          fastInterceptFinder2d(containedHits, left, right, lowerIndex, upperIndex, currentRecursion + 1);
        } else {
          m_SectorArray[localIndexY * m_nAngleSectors + localIndexX] = -layerFilter(layerHits);
          m_activeSectorArray.push_back(std::make_pair(localIndexX, localIndexY));
//           int nActiveSectors = std::count_if(m_SectorArray.begin(), m_SectorArray.end(), [](int c) {return c < 0;});
//           B2DEBUG(29, "number of active sectors: " << nActiveSectors << " localIndexX: " << localIndexX << " localIndexY: " << localIndexY << " totalIndex: " << localIndexY * m_nAngleSectors + localIndexX);
        }
      }
    }
  }
}

void FastInterceptFinder2DFPGA::FindHoughSpaceCluster()
{
  // cell content meanings:
  // -3, -4  : active sector, not yet visited
  // 0       : non-active sector (will never be visited, only checked)
  // 1,2,3...: index of the clusters

  m_clusterCount = 1;

  // this sorting makes sure the clusters can be searched from bottom left of the HS to top right
  // normally, a C++ array looks like a matrix:
  // (0   , 0) ... (maxX, 0   )
  //    ...            ...
  // (0, maxY) ... (maxX, maxY)
  // but for sorting we want it to be like regular coordinates
  // (0, maxY) ... (maxX, maxY)
  //    ...            ...
  // (0, 0   ) ... (maxX, 0   )
  // By setting the offset to the maximum allowed number of cells (2^14) and simplifying
  // (16384 - a.second) * 16384 + a.first < (16384 - b.second) * 16384 + b.first
  // we get the formula below
  auto sortSectors = [](const std::pair<uint, uint> a, const std::pair<uint, uint> b) {
    return ((int)b.second - (int)a.second) * 16384 < (int)b.first - (int)a.first;
  };
  std::sort(m_activeSectorArray.begin(), m_activeSectorArray.end(), sortSectors);

  for (auto& currentCell : m_activeSectorArray) {
    const uint currentIndex = currentCell.second * m_nAngleSectors + currentCell.first;
    if (m_SectorArray[currentIndex] > -1) continue;

    m_clusterInitialPosition = currentCell;
    m_clusterCoG = currentCell;
    m_clusterSize = 1;
    m_SectorArray[currentIndex] = m_clusterCount;
    // Check for HS sectors connected to each other which could form a cluster
    DepthFirstSearch(currentCell.first, currentCell.second);
    // if cluster valid (i.e. not too small and not too big): finalize!
    if (m_clusterSize >= m_MinimumHSClusterSize and m_clusterSize <= m_MaximumHSClusterSize) {
      double CoGX = (((double)m_clusterCoG.first / (double)m_clusterSize) + 0.5) * m_unitX + m_minimumX;
      if (not m_param_isUFinder) {
        CoGX = atan(tan(m_minimumX) + m_unitX * (((double)m_clusterCoG.first / (double)m_clusterSize) + 0.5));
      }
      double CoGY = m_verticalHoughSpaceSize - ((double)m_clusterCoG.second / (double)m_clusterSize - 0.5) * m_unitY;

      if (m_param_isUFinder) {
        double trackPhi = CoGX + M_PI_2;
        if (trackPhi < -M_PI) trackPhi += 2 * M_PI;
        if (trackPhi >  M_PI) trackPhi -= 2 * M_PI;

        // 1./CoGY * 1e10 yields trackRadius in mm. To convert to µm, which all other values are in,
        // multiplication by another 1e3 is required -> total of 1e13
        double trackRadius = 1. / CoGY * 1e+13;

        m_trackCandidates.emplace_back(std::make_pair(trackPhi, trackRadius));
      } else {
        m_trackCandidates.emplace_back(std::make_pair(CoGX, CoGY));
      }
      B2DEBUG(29, "m_clusterCoG.first: " << m_clusterCoG.first << " " << ((double)m_clusterCoG.first / (double)m_clusterSize) <<
              " m_clusterCoG.second: " << m_clusterCoG.second << " " << ((double)m_clusterCoG.second / (double)m_clusterSize) << " CoGX: " << CoGX
              << " CoGY: " << CoGY);

      if (m_writeGnuplotOutput) {
        m_cogoutstream << CoGX << " " << CoGY << std::endl;
      }
    }
    m_clusterCount++;
  }
}

void FastInterceptFinder2DFPGA::DepthFirstSearch(uint lastIndexX, uint lastIndexY)
{
  if (m_clusterSize >= m_MaximumHSClusterSize) return;

  for (uint currentIndexY = lastIndexY; currentIndexY >= lastIndexY - 1; currentIndexY--) {
    if (abs((int)m_clusterInitialPosition.second - (int)currentIndexY) >= m_MaximumHSClusterSizeY or
        m_clusterSize >= m_MaximumHSClusterSize or currentIndexY > m_nVerticalSectors) return;
    for (uint currentIndexX = lastIndexX; currentIndexX <= lastIndexX + 1; currentIndexX++) {
      if (abs((int)m_clusterInitialPosition.first - (int)currentIndexX) >= m_MaximumHSClusterSizeX or
          m_clusterSize >= m_MaximumHSClusterSize or currentIndexX > m_nAngleSectors) return;

      // The cell (currentIndexX, currentIndexY) is the current one has already been checked, so continue
      if (lastIndexX == currentIndexX && lastIndexY == currentIndexY) continue;

      // first check bounds to avoid out-of-bound array access
      // as they are uints, they are always >= 0, and in case of an overflow they would be too large
      if (currentIndexX < m_nAngleSectors and currentIndexY < m_nVerticalSectors) {

        if (m_SectorArray[currentIndexY * m_nAngleSectors + currentIndexX] < 0 /*and m_clusterSize < m_MaximumHSClusterSize*/) {
          // Only continue searching if the current cluster is smaller than the maximum cluster size
          m_SectorArray[currentIndexY * m_nAngleSectors + currentIndexX] = m_clusterCount;
          m_clusterCoG = std::make_pair(m_clusterCoG.first + currentIndexX, m_clusterCoG.second + currentIndexY);
          m_clusterSize++;
          // search in the next Hough Space cells...
          DepthFirstSearch(currentIndexX, currentIndexY);
        }

      }
    }
  }
}

void FastInterceptFinder2DFPGA::gnuplotoutput(std::vector<std::pair<VxdID, std::pair<long, long>>>& hits)
{
  std::ofstream hsoutstream;
  hsoutstream.open(m_gnuplotHSOutputFileName.c_str(), std::ios::trunc);

  hsoutstream << "set pointsize 1.5\nset style line 42 lc rgb '#0060ad' pt 7   # circle" << std::endl;

  hsoutstream << "set style line 80 lt rgb \"#808080\"" << std::endl;
  hsoutstream << "set style line 81 lt 0" << std::endl;
  hsoutstream << "set style line 81 lt rgb \"#808080\"" << std::endl << std::endl;

  hsoutstream << "set style line 1 lt rgb \"#A00000\" lw 1 pt 1" << std::endl;
  hsoutstream << "set style line 2 lt rgb \"#00A000\" lw 1 pt 6" << std::endl;
  hsoutstream << "set style line 3 lt rgb \"#000000\" lw 1 pt 6" << std::endl;

  hsoutstream << "set style line 3 lt rgb 'black' lw 1 pt 6" << std::endl;
  hsoutstream << "set style line 4 lt rgb 'blue' lw 1 pt 6" << std::endl;
  hsoutstream << "set style line 5 lt rgb 'green' lw 1 pt 6" << std::endl;
  hsoutstream << "set style line 6 lt rgb 'red' lw 1 pt 6" << std::endl;

  hsoutstream << "# Description position\nset key top right" << std::endl << std::endl;
  hsoutstream << "# Grid and border style\nset grid back linestyle 81\nset border 3 back linestyle 80" << std::endl << std::endl;

  hsoutstream << "# No mirrors\nset xtics nomirror\nset ytics nomirror" << std::endl << std::endl;
  hsoutstream << "# Encoding\nset encoding utf8" << std::endl << std::endl;
  hsoutstream << "set xlabel \"x\"\nset ylabel \"y\"" << std::endl << std::endl;

  hsoutstream << "set xrange [-pi-0.5:pi+0.5]" << std::endl << std::endl;

  hsoutstream << "load '" << m_gnuplotHSRectOutputFileName << "'" << std::endl << std::endl;

  uint count = 0;
  for (auto& hit : hits) {
    const VxdID& id = hit.first;
    int layer = id.getLayerNumber();
    const long xc = 1000 * hit.second.first;
    const long yc = 1000 * hit.second.second;

    hsoutstream << "plot " << xc << " * -sin(x) + " << yc << " * cos(x) > 0 ? " << xc << " * cos(x) + " << yc <<
                " * sin(x) : 1/0 notitle linestyle " << layer << " # " << id << std::endl;
    if (count < hits.size() - 1) hsoutstream << "re";
    count++;
  }

  hsoutstream << std::endl;
  hsoutstream << "replot '" << m_gnuplotHSCoGOutputFileName << "' u 1:2 w p ls 42 notitle" << std::endl << std::endl;
  hsoutstream << "pause -1" << std::endl;
  hsoutstream.close();
}
