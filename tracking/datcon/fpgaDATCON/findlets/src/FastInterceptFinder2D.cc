/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/fpgaDATCON/findlets/FastInterceptFinder2D.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <vxd/dataobjects/VxdID.h>

#include <framework/core/ModuleParamList.h>
#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

FastInterceptFinder2D::FastInterceptFinder2D() : Super()
{
//   this->addProcessingSignalListener(&m_trackFitter);
}

void FastInterceptFinder2D::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
//   m_trackFitter.exposeParameters(moduleParamList, prefix);

//   Super::exposeParameters(moduleParamList, prefix);

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

}

void FastInterceptFinder2D::initialize()
{
  Super::initialize();

  m_maxRecursionLevel = ceil(log2(std::max(m_nAngleSectors, m_nVerticalSectors))) - 1;
  B2ASSERT("The maximum number of iterations in u must not be larger than 14, but it is " << m_maxRecursionLevel,
           m_maxRecursionLevel <= 14);
  const float uUnitX = (m_maximumX - m_minimumX) / m_nAngleSectors;
  for (uint i = 0; i < m_nAngleSectors; i++) {
    double x = m_minimumX + uUnitX * i;
    double xc = x + 0.5 * uUnitX;

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

  const long uUnitY = 2 * m_verticalHoughSpaceSize / m_nVerticalSectors;
  for (uint i = 0; i <= m_nVerticalSectors; i++) {
    m_HSYLUT[i] = m_verticalHoughSpaceSize - uUnitY * i;
    m_HSYCenterLUT[i] = m_verticalHoughSpaceSize - uUnitY * i - 0.5 * uUnitY;
  }
}

void FastInterceptFinder2D::apply(std::vector<std::pair<VxdID, std::pair<long, long>>>& hits)
{
  fastInterceptFinder2d(hits, 0, m_nAngleSectors, 0, m_nVerticalSectors, 0);
}

void FastInterceptFinder2D::fastInterceptFinder2d(std::vector<std::pair<VxdID, std::pair<long, long>>>& hits,
                                                  uint xmin, uint xmax, uint ymin, uint ymax, uint iterations)
{
//   vector<unsigned int> candidateIDList;
  std::vector<std::pair<VxdID, std::pair<long, long>>> containedHits;

  if (iterations == m_maxRecursionLevel + 1) return;

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
    const short   sinCenter   = convertToInt(sin((localLeft + localRight) / 2.), 3);
    const short   cosCenter   = convertToInt(cos((localLeft + localRight) / 2.), 3);

    for (int j = 0; j < 2; ++j) {

      const uint lowerIndex = yIndexCache[j];
      const uint upperIndex = yIndexCache[j + 1];

      const uint localIndexY = lowerIndex;
      const long& localUpperCoordinate = m_HSYLUT[lowerIndex];
      const long& localLowerCoordinate = m_HSYLUT[upperIndex];

      if (lowerIndex == upperIndex) continue;

      bool layerHit[4] = {false}; /* For layer filter */
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

        /* Check if HS-parameter curve is inside (or outside) actual sub-HS */
        if (((yLeft <= localUpperCoordinate && yRight >= localLowerCoordinate) || (yCenter <= localUpperCoordinate
             && yCenter >= localLowerCoordinate && derivativeyCenter >= 0)) && (derivativeyLeft >= 0 || derivativeyRight >= 0
                 || derivativeyCenter >= 0)) {
//           if (iterations == m_maxRecursionLevel) {
//           }

          layerHit[sensor.getLayerNumber() - 3] = true; /* layer filter */
          containedHits.emplace_back(hit);
        }
      }

      if (layerFilter(layerHit) > 0) {
        // recursive / iterative call of fastInterceptFinder2d, until iterations == m_maxRecursionLevel,
        // actual values for v1...v4 are new startingpoints
        if (iterations != m_maxRecursionLevel) {
          fastInterceptFinder2d(containedHits, left, right, lowerIndex, upperIndex, iterations + 1);
        } else {
// //             ArrayOfActiveSectorsPhiHS[localIndexY * m_nAngleSectorsU + localIndexX] = -layerFilter(layerHit);
// //             uHoughSpaceClusterCands.push_back(DATCONSimpleHoughSpaceClusterCand(candidateIDList, make_pair(localIndexX, localIndexY)));

        }
      }
    }
  }
}
