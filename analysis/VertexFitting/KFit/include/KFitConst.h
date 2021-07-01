/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#pragma once


namespace Belle2 {

  namespace analysis {

    /**
     * KFitConst is a container of constant names.
     */
    struct KFitConst {
      /** Constant 3 to check matrix size (internal use) */
      static const int    kNumber3 = 3;
      /** Constant 6 to check matrix size (internal use) */
      static const int    kNumber6 = 6;
      /** Constant 7 to check matrix size (internal use) */
      static const int    kNumber7 = 7;

      /** Input parameter to specify before-fit when setting/getting a track attribute */
      static const int    kBeforeFit = 0;
      /** Input parameter to specify after-fit when setting/getting a track attribute */
      static const int    kAfterFit  = 1;

      /** Maximum track size */
      static const int    kMaxTrackCount  = 10;
      /** Maximum track size (internal use) */
      static const int    kMaxTrackCount2 = 30;

      /** Maximum iteration step (internal use) */
      static const int    kMaxIterationCount = 15;

      /** Initial chi-square value (internal use) */
      static constexpr double kInitialCHIsq = 1.0e+30;

      /** Default magnetic field */
      static constexpr double kDefaultMagneticField = 1.45638;

      /** Speed of light */
      static constexpr double kLightSpeed = 0.00299792458;
    };

  } // namespace analysis

} // namespace Belle2


