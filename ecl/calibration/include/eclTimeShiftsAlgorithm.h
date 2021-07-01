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

      double crysCrateShift_min;   /**< Plotting time min for crystal+crate shift plots */
      double crysCrateShift_max;   /**< Plotting time max for crystal+crate shift plots */

      /** Whether or not to have the algorithm code to loop
          over all the runs and read the payloads itself.  */
      bool algorithmReadPayloads;

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
      Int_t m_run_perCrystal;         /**< Run number */
      Int_t m_exp_perCrystal;         /**< Experiment number  */
      Int_t m_crystalID;              /**< Crystal ID number*/
      Double_t m_crateTimeConst;      /**< Crate time calibration constant */
      Double_t m_crystalTimeConst;    /**< Crystal time calibration constant */
      Double_t m_crateTimeUnc;        /**< Uncertainty on the crate time calibration constant */
      Double_t m_crystalTimeUnc;      /**< Uncertainty on the crystal time calibration constant */
      Int_t m_crateID;                /**< Crate ID number */

      // Variables to be read in from the per-crate tree
      //Int_t m_run_perCrate;
      //Int_t m_exp_perCrate;
      Int_t m_refCrystalID;    /**< Crystal ID number for the reference crystal */

      // Cuts for runs to plot
      double m_tcrate_min_cut = -150;           /**< Minimum value cut for the crate time calibration constant for plotting */
      double m_tcrate_max_cut = 150;            /**< Maximum value cut for the crate time calibration constant for plotting  */
      double m_tcrate_unc_min_cut = 0.0001 ;    /**< Minimum value cut for the crate time calibration constant uncertainty for plotting */
      double m_tcrate_unc_max_cut = 0.4 ;       /**< Maximum value cut for the crate time calibration constant uncertainty for plotting */


    };
  }
} // namespace Belle2


