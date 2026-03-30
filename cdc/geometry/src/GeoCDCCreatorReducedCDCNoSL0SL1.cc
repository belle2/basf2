/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cdc/geometry/GeoCDCCreatorReducedCDCNoSL0SL1.h>
#include <cdc/dbobjects/CDCGeometry.h>

#include <geometry/CreatorFactory.h>

namespace Belle2 {

  using namespace geometry;

  namespace CDC {

    /**
     * Register the GeoCreator.
     */

    geometry::CreatorFactory<GeoCDCCreatorReducedCDCNoSL0SL1> GeoCDCFactoryReducedCDCNoSL0SL1("CDCCreatorReducedCDCNoSL0SL1");

    bool GeoCDCCreatorReducedCDCNoSL0SL1::getEndplateInformation(const CDCGeometry& geo, const uint iSLayer,
        double& rMinLeft, double& rMaxLeft, double& zBackLeft, double& zForLeft,
        double& rMinMiddle, double& rMaxMiddle, double& zBackMiddle, double& zForMiddle,
        double& rMinRight, double& rMaxRight, double& zBackRight, double& zForRight) const
    {
      const auto& endplate = geo.getEndPlate(iSLayer);
      const int nEPLayer = endplate.getNEndPlateLayers();

      if (iSLayer == 0) {
        const auto& epLayerBwd = endplate.getEndPlateLayer(1);
        const auto& epLayerFwd = endplate.getEndPlateLayer((nEPLayer / 2) + 1);
        const auto& senseLayer = geo.getSenseLayer(iSLayer);
        const auto& fieldLayer = geo.getFieldLayer(iSLayer);

        rMinLeft = epLayerBwd.getRmax();
        rMaxLeft = fieldLayer.getR();
        zBackLeft = senseLayer.getZbwd();
        zForLeft = epLayerBwd.getZfwd();

        rMinMiddle = (geo.getInnerWall(0)).getRmax();
        rMaxMiddle = fieldLayer.getR();
        zBackMiddle = epLayerBwd.getZfwd();
        zForMiddle = epLayerFwd.getZbwd();

        rMinRight = epLayerFwd.getRmax();
        rMaxRight = fieldLayer.getR();
        zBackRight = epLayerFwd.getZbwd();
        zForRight = senseLayer.getZfwd();
      } else if (iSLayer >= 1 && iSLayer <= 4) {
        const auto& epLayerBwd = endplate.getEndPlateLayer(1);
        const auto& epLayerFwd = endplate.getEndPlateLayer(nEPLayer / 2);
        const auto& senseLayer = geo.getSenseLayer(iSLayer);
        const auto& fieldLayerIn = geo.getFieldLayer(iSLayer - 1);
        const auto& fieldLayerOut = geo.getFieldLayer(iSLayer);

        rMinLeft = epLayerBwd.getRmax();
        rMaxLeft = fieldLayerOut.getR();
        zBackLeft = senseLayer.getZbwd();
        zForLeft = epLayerBwd.getZfwd();

        rMinMiddle = fieldLayerIn.getR();
        rMaxMiddle = fieldLayerOut.getR();
        zBackMiddle = epLayerBwd.getZfwd();
        zForMiddle = epLayerFwd.getZbwd();

        rMinRight = epLayerFwd.getRmax();
        rMaxRight = fieldLayerOut.getR();
        zBackRight = epLayerFwd.getZbwd();
        zForRight = senseLayer.getZfwd();
      } else if (iSLayer >= 5 && iSLayer < 41) {
        const auto& epLayerBwd = endplate.getEndPlateLayer(0);
        const auto& epLayerFwd = endplate.getEndPlateLayer(nEPLayer / 2);
        const auto& senseLayer = geo.getSenseLayer(iSLayer);
        const auto& fieldLayerIn = geo.getFieldLayer(iSLayer - 1);
        const auto& fieldLayerOut = geo.getFieldLayer(iSLayer);

        rMinLeft = epLayerBwd.getRmax();
        rMaxLeft = fieldLayerOut.getR();
        zBackLeft = senseLayer.getZbwd();
        zForLeft = epLayerBwd.getZfwd();

        rMinMiddle = fieldLayerIn.getR();
        rMaxMiddle = fieldLayerOut.getR();
        zBackMiddle = epLayerBwd.getZfwd();
        zForMiddle = epLayerFwd.getZbwd();

        rMinRight = epLayerFwd.getRmax();
        rMaxRight = fieldLayerOut.getR();
        zBackRight = epLayerFwd.getZbwd();
        zForRight = senseLayer.getZfwd();

      } else if (iSLayer == 41) {

        const auto& epLayerBwdIn = endplate.getEndPlateLayer(0);
        const auto& epLayerBwdOut = endplate.getEndPlateLayer((nEPLayer / 2) - 1);
        const auto& epLayerFwdIn = endplate.getEndPlateLayer(nEPLayer / 2);
        const auto& epLayerFwdOut = endplate.getEndPlateLayer(nEPLayer - 1);
        const auto& senseLayer = geo.getSenseLayer(iSLayer);

        int iSLayerMinus1 = iSLayer - 1; //avoid cpp-check warning
        const auto& fieldLayerIn = geo.getFieldLayer(iSLayerMinus1); //avoid cpp-check warning

        rMinLeft = epLayerBwdIn.getRmax();
        rMaxLeft = epLayerBwdOut.getRmax();
        zBackLeft = senseLayer.getZbwd();
        zForLeft = epLayerBwdIn.getZfwd();

        rMinMiddle = fieldLayerIn.getR();
        rMaxMiddle = (geo.getOuterWall(0)).getRmin();
        zBackMiddle = epLayerBwdIn.getZfwd();
        zForMiddle = epLayerFwdIn.getZbwd();

        rMinRight = epLayerFwdIn.getRmax();
        rMaxRight = epLayerFwdOut.getRmax();
        zBackRight = epLayerFwdIn.getZbwd();
        zForRight = senseLayer.getZfwd();

      } else {
        B2ERROR("Undefined sensitive layer : " << iSLayer);
        return false;
      }
      return true;
    }
  }
}
