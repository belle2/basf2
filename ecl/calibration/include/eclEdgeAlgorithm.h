/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Generates payloads ECLCrystalThetaEdge and ECLCrystalPhiEdge using     *
 * histograms filled by eclEdgeCollectorModule                            *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christopher Hearty (hearty@physics.ubc.ca)               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <framework/database/DBObjPtr.h>
#include <calibration/CalibrationAlgorithm.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>

namespace Belle2 {
  namespace ECL {

    /**..Obtain payloads specifying the location of the edges of each ECL crystal */
    class eclEdgeAlgorithm : public CalibrationAlgorithm {
    public:

      /**..Constructor */
      eclEdgeAlgorithm();

      /**..Destructor */
      virtual ~eclEdgeAlgorithm() {}

    protected:

      /**..Run algorithm */
      virtual EResult calibrate() override;

    private:
      const short m_crystalsPerRing[69] = {
        48, 48, 64, 64, 64, 96, 96, 96, 96, 96, 96, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 96, 96, 96, 96, 96, 64, 64, 64
      }; /**< crystals per thetaID */
    };
  }
} // namespace Belle2


