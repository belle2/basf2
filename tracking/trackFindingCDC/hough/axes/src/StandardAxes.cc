/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/trackFindingCDC/hough/axes/StandardAxes.h>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <tracking/trackFindingCDC/topology/CDCWireLayer.h>
#include <tracking/trackFindingCDC/geometry/GeneralizedCircle.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CurvWithArcLength2DCache::CurvWithArcLength2DCache(float curv)
  : m_curv(curv)
  , m_arcLength2DByICLayer{{0}}
{
  CDCWireTopology& wireTopology = CDCWireTopology::getInstance();

  for (const CDCWireLayer& wireLayer : wireTopology.getWireLayers()) {
    ILayer iCLayer = wireLayer.getICLayer();
    double cylindricalR = (wireLayer.getOuterCylindricalR() + wireLayer.getInnerCylindricalR()) / 2;
    double factor = GeneralizedCircle::arcLengthFactor(cylindricalR, curv);

    // Fall back when the closest approach to the layer is the apogee
    double arcLength2D = cylindricalR * std::fmin(factor, M_PI);
    double r = 1.0 / fabs(m_curv);

    m_arcLength2DByICLayer[iCLayer] = arcLength2D;
    m_secondaryArcLength2DByICLayer[iCLayer] = 2 * M_PI * r - arcLength2D;
  }
}
