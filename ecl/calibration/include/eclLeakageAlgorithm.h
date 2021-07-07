/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <framework/database/DBObjPtr.h>
#include <calibration/CalibrationAlgorithm.h>
#include <ecl/dbobjects/ECLLeakageCorrections.h>

namespace Belle2 {
  namespace ECL {

    /** Calculate ECL energy leakage corrections */
    class eclLeakageAlgorithm : public CalibrationAlgorithm {
    public:

      /** Constructor */
      eclLeakageAlgorithm();

      /** Destructor */
      virtual ~eclLeakageAlgorithm() {}

    protected:

      /** Run algorithm */
      virtual EResult calibrate() override;

    private:

      /** For TTree */
      int t_cellID = 0; /**< cellID of photon */
      int t_thetaID = 0; /**< thetaID of photon */
      int t_region = 0; /**< region of photon 0=forward 1=barrel 2=backward*/
      int t_thetaBin = -1; /**< binned location in theta relative to crystal edge */
      int t_phiBin = -1; /**< binned location in phi relative to crystal edge */
      int t_phiMech = -1; /**< mechanical structure next to lower phi (0), upper phi (1), or neither (2) */
      int t_energyBin = -1; /**< generated energy point */
      int t_nCrys = -1; /**< number of crystals used to calculate energy */
      float t_energyFrac = 0.; /**< measured energy (without leakage correction) divided by generated */
      float t_origEnergyFrac = 0.; /**< measured energy with leakage correction divided by generated */
      float t_locationError = 999.; /**< reconstructed minus generated position (cm) */


    };
  }
} // namespace Belle2


