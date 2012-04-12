/**************************************************************************
 * Copyright(C) 1997 - J. Tanaka                                          *
 *                                                                        *
 * Author: J. Tanaka                                                      *
 * Contributor: J. Tanaka and                                             *
 *              conversion to Belle II structure by T. Higuchi            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef KFITCONST_H
#define KFITCONST_H


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
      static const double kInitialCHIsq = 1.0e+30;

      /** Default magnetic field */
      static const double kDefaultMagneticField = 1.45638;

      /** Speed of light */
      static const double kLightSpeed = 0.00299792458;
    };

  } // namespace analysis

} // namespace Belle2

#endif  /* KFITCONST_H */

