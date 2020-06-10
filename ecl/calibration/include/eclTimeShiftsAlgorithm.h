/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Plot the ecl times (crystal and crate)                                 *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *   Ewan Hill   <ehill@mail.ubc.ca>                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <framework/database/DBObjPtr.h>
#include <calibration/CalibrationAlgorithm.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <ecl/dbobjects/ECLReferenceCrystalPerCrateCalib.h>
#include <string>

namespace Belle2 {
  namespace ECL {

    /** Calibrate ecl crystals using previously created payloads */
    class eclTimeShiftsAlgorithm : public CalibrationAlgorithm {
    public:

      /**..Constructor */
      eclTimeShiftsAlgorithm();

      /**..Destructor */
      virtual ~eclTimeShiftsAlgorithm() {}


      /*** Parameters ***/

      /** Name of file with debug output, eclTimeShiftsAlgorithm.root by default */
      std::string debugFilenameBase;

      /**< List of time offsets, one per crate, used just to centre the time
           constants around zero */
      double timeShiftForPlotStyle[52] ;

      /** Plotting time min/max for crystal+crate+shift plots */
      double crysCrateShift_min;
      double crysCrateShift_max;

      /** Whether or not to veto runs where an older payload is used.
          Force the payload revision number to increase as the
          run number increases. */
      bool forcePayloadIOVnotOpenEnded;

    protected:

      /**..Run algorithm */
      virtual EResult calibrate() override;

    private:

      /** ECLCrystalTimeOffset payload that we want to read from the DB */
      DBObjPtr<ECLCrystalCalib> m_ECLCrystalTimeOffset;

      /** ECLCrateTimeOffset payload that we want to read from the DB */
      DBObjPtr<ECLCrystalCalib> m_ECLCrateTimeOffset;

      /**  payload that we want to read from the DB */
      DBObjPtr<ECLReferenceCrystalPerCrateCalib> m_refCrysIDzeroingCrate;

      /** Number of Crystals expected */
      const int m_numCrystals = 8736;

      /** Number of Crates expected */
      const int m_numCrates = 52;
    };
  }
} // namespace Belle2


