/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <cdc/geometry/GeoCDCCreator.h>

namespace Belle2 {

  class CDCGeometry;

  namespace CDC {

    //!  The GeoCDCCreatorReducedCDCNoSL0SL1 class.
    /*!
       The creator for the CDC geometry of the Belle II detector
       where the SuperLayers 0 and 1 are removed.
    */
    class GeoCDCCreatorReducedCDCNoSL0SL1 : public GeoCDCCreator {

    public:

      //! Constructor of the GeoCDCCreatorReducedCDCNoSL0SL1 class.
      GeoCDCCreatorReducedCDCNoSL0SL1() : GeoCDCCreator()
      {
        B2WARNING("Using CDC without SL0 and SL1!");
      }

      //! The destructor of the GeoCDCCreatorReducedCDCNoSL0SL1 class.
      ~GeoCDCCreatorReducedCDCNoSL0SL1() {};

    private:
      /**
       *  Get endplate information
       *
       * @param[in] geo: the CDCGeometry
       * @param[in] iSLayer: number of the sense layer
       *
       * @param[inout] rMinLeft: minimum radius of sensitive layer left
       * @param[inout] rMaxLeft: maximum radius of sensitive layer left
       * @param[inout] zBackLeft: backward z value of sensitive layer left
       * @param[inout] zForLeft: forward z value of sensitive layer left
       * @param[inout] rMinMiddle: minimum radius of sensitive layer middle
       * @param[inout] rMaxMiddle: maximum radius of sensitive layer middle
       * @param[inout] zBackMiddle: backward z value of sensitive layer middle
       * @param[inout] zForMiddle: forward z value of sensitive layer middle
       * @param[inout] rMinRight: minimum radius of sensitive layer right
       * @param[inout] rMaxRight: maximum radius of sensitive layer right
       * @param[inout] zBackRight: backward z value of sensitive layer right
       * @param[inout] zForRight: forward z value of sensitive layer right
       *
       * @returns: true on success, false otherwise (only for unknown value of iSLayer)
       */
      virtual bool getEndplateInformation(const CDCGeometry& geo, const uint iSLayer,
                                          double& rMinLeft, double& rMaxLeft, double& zBackLeft, double& zForLeft,
                                          double& rMinMiddle, double& rMaxMiddle, double& zBackMiddle, double& zForMiddle,
                                          double& rMinRight, double& rMaxRight, double& zBackRight, double& zForRight) const override;

    };

  } // end of cdc namespace
} // end of Belle2 namespace
