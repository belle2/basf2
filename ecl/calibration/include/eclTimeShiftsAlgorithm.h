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
      ~eclTimeShiftsAlgorithm() {}


      /*** Parameters ***/

      /** Name of file with debug output, eclTimeShiftsAlgorithm.root by default */
      std::string debugFilenameBase;

      /** List of time offsets, one per crate, used just to centre the time
           constants around zero */
      double timeShiftForPlotStyle[52] ;

      /** Plotting time min/max for crystal+crate+shift plots */
      double crysCrateShift_min;
      double crysCrateShift_max;

      /** Whether or not to have the algorithm code to loop
          over all the runs and read the payloads itself.  */
      bool algorithmReadPayloads;

      /** Whether or not to veto runs where an older payload is used.
          Force the payload revision number to increase as the
          run number increases. */
      bool forcePayloadIOVnotOpenEndedAndSequentialRevision;

    protected:

      /**..Run algorithm */
      EResult calibrate() override;

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

      // Variables to be read in from the per-crystal tree
      Int_t m_run_perCrystal;
      Int_t m_exp_perCrystal;
      Int_t m_crystalID;
      Double_t m_crateTimeConst;
      Double_t m_crystalTimeConst;
      Double_t m_crateTimeUnc;
      Double_t m_crystalTimeUnc;
      Int_t m_crateID;

      // Variables to be read in from the per-crate tree
      //Int_t m_run_perCrate;
      //Int_t m_exp_perCrate;
      Int_t m_refCrystalID;

      // Cuts for runs to plot
      double m_tcrate_min_cut = -150;
      double m_tcrate_max_cut = 150;
      double m_tcrate_unc_min_cut = 0.0001 ;
      double m_tcrate_unc_max_cut = 0.4 ;


    };
  }
} // namespace Belle2


