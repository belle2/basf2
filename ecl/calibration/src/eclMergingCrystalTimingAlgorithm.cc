#include <ecl/calibration/eclMergingCrystalTimingAlgorithm.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <ecl/dbobjects/ECLReferenceCrystalPerCrateCalib.h>
#include <ecl/utility/ECLChannelMapper.h>
#include "TH1F.h"
#include "TString.h"
#include "TFile.h"
#include "TDirectory.h"

using namespace std;
using namespace Belle2;
using namespace ECL;
using namespace Calibration;

/**-----------------------------------------------------------------------------------------------*/
eclMergingCrystalTimingAlgorithm::eclMergingCrystalTimingAlgorithm(): CalibrationAlgorithm("DummyCollector"),
  m_ECLCrystalTimeOffsetBhabha("ECLCrystalTimeOffsetBhabha"), m_ECLCrystalTimeOffsetBhabhaGamma("ECLCrystalTimeOffsetBhabhaGamma"),
  m_ECLCrystalTimeOffsetCosmic("ECLCrystalTimeOffsetCosmic"), m_ECLReferenceCrystalPerCrateCalib("ECLReferenceCrystalPerCrateCalib"),
  m_ECLCrateTimeOffset("ECLCrateTimeOffset")
{
  setDescription(
    "Perform time calibration of ecl crystals by combining previous values from the DB for different calibrations."
  );
}

CalibrationAlgorithm::EResult eclMergingCrystalTimingAlgorithm::calibrate()
{
  //------------------------------------------------------------------------
  // Get the input run list (should be only 1) for us to use to update the DBObjectPtrs
  auto runs = getRunList();
  /* Take the first run.  For the crystal cosmic calibrations, because of the crate
     calibrations, there is not a known correct run to use within the range. */
  ExpRun chosenRun = runs.front();
  B2INFO("merging using the ExpRun (" << chosenRun.second << "," << chosenRun.first << ")");
  // After here your DBObjPtrs are correct
  updateDBObjPtrs(1, chosenRun.second, chosenRun.first);


  //------------------------------------------------------------------------
  /* With the database set up, load the ecl channel mapper to access the
     crystal to crate mapping information */
  unique_ptr<ECLChannelMapper> crystalMapper(new ECL::ECLChannelMapper());
  crystalMapper->initFromDB();

  //------------------------------------------------------------------------
  // Test the DBObjects we want to exist and fail if not all of them do.
  bool allObjectsFound = true;

  /** Test that the DBObjects are valid */
  // Check that the payloads we want to merge are sufficiently loaded
  if (!m_ECLCrystalTimeOffsetBhabha) {
    allObjectsFound = false;
    B2ERROR("No valid DBObject found for 'ECLCrystalTimeOffsetBhabha'");
  }
  if (!m_ECLCrystalTimeOffsetBhabhaGamma) {
    allObjectsFound = false;
    B2ERROR("No valid DBObject found for 'ECLCrystalTimeOffsetBhabhaGamma'");
  }
  if (!m_ECLCrystalTimeOffsetCosmic) {
    allObjectsFound = false;
    B2ERROR("No valid DBObject found for 'ECLCrystalTimeOffsetCosmic'");
  }

  // Check that the reference crystal payload is loaded
  if (!m_ECLReferenceCrystalPerCrateCalib) {
    allObjectsFound = false;
    B2ERROR("No valid DBObject found for 'ECLReferenceCrystalPerCrateCalib'");
  }

  // Check that the crate payload is loaded (used for transforming cosmic payload)
  if (!m_ECLCrateTimeOffset) {
    allObjectsFound = false;
    B2ERROR("No valid DBObject found for 'ECLCrateTimeOffset'");
  }


  if (allObjectsFound) {
    B2INFO("Valid objects found for 'ECLCrystalTimeOffsetBhabha', 'ECLCrystalTimeOffsetBhabhaGamma' and 'ECLCrystalTimeOffsetCosmic')");
    B2INFO("Valid object found for 'ECLReferenceCrystalPerCrateCalib'");
    B2INFO("Valid object found for 'ECLCrateTimeOffset'");
  } else {
    B2INFO("Exiting with failure");
    return c_Failure;
  }

  //------------------------------------------------------------------------
  /** Get the vectors from the input payloads */
  vector<float> bhabhaCalib = m_ECLCrystalTimeOffsetBhabha->getCalibVector();
  vector<float> bhabhaCalibUnc = m_ECLCrystalTimeOffsetBhabha->getCalibUncVector();

  vector<float> bhabhaGammaCalib = m_ECLCrystalTimeOffsetBhabhaGamma->getCalibVector();
  vector<float> bhabhaGammaCalibUnc = m_ECLCrystalTimeOffsetBhabhaGamma->getCalibUncVector();

  vector<float> cosmicCalib = m_ECLCrystalTimeOffsetCosmic->getCalibVector();
  vector<float> cosmicCalibUnc = m_ECLCrystalTimeOffsetCosmic->getCalibUncVector();

  B2INFO("Loaded 'ECLCrystalTimeOffset*' calibrations");

  // Get the list of reference crystals as special case crystals
  vector<short> refCrystalPerCrate = m_ECLReferenceCrystalPerCrateCalib->getReferenceCrystals();

  B2INFO("Loaded 'ECLReferenceCrystalPerCrateCalib' calibration");

  vector<float> crateCalib = m_ECLCrateTimeOffset->getCalibVector();
  vector<float> crateCalibUnc = m_ECLCrateTimeOffset->getCalibUncVector();

  B2INFO("Loaded 'ECLCrateTimeOffset' calibration");


  //------------------------------------------------------------------------
  /** Shift cosmic calibrations.  The cosmic calibrations were made assuming
      all crate times were zero (taken before the crate calibrations were
      even created).  As such, the cosmic crystal calibrations for the merged
      payload should be shifted by the the appropriate amount to accommodate
      the non-zero crate time calibrations.  Also, the cosmic calibrations
      should be shifted to better match the bhabha calibrations.  The
      average crystal calibration per crate is calculated for both the
      cosmic and bhabha calibrations and all the cosmic calibrations are
      shifted so that the averages are the same.

      Some of the conditions that define a bad fit in the bhabha
      calibrations.

           (fit_mean_unc < 3)    <<--- uncertianty on mean
           (fit_sigma < 0.1)     <<--- sigma, not uncertainty on mean
           (numEntries < 40 )

      If the fit is not used then the overall histogram stats information
      is used.  That means that there can still be poor ts values from
      non-fits, including low stats and single event values.
      Events with hits really far apart in time in low stats events can
      be problematic.
  */




  //------------------------------------------------------------------------
  /* Determine the bhabha and radiavtive bhabha calibration quality*/


  vector<bool> bhabhaCalibGoodQuality(m_numCrystals, false);
  vector<bool> bhabhaGammaCalibGoodQuality(m_numCrystals, false);
  for (int ic = 0; ic < m_numCrystals; ic++) {
    /* Define a good bhabha calibration value.  This is the
       uncertainty on the calibration constant (e.g. fit mean),
       not the guassian width of the timing distribution for
       that crystal (i.e. resolution). */
    if (bhabhaCalibUnc[ic] != 0
        && bhabhaCalibUnc[ic] < 1) {    // The limits on the uncertainties are not yet optimized !!!!!!!!!!!!!!!!!!!!!!!!!!
      bhabhaCalibGoodQuality[ic] = true ;
    }
    /* Define a good radiative bhabha calibration value, which
       should be looser than the bhabha requirements since
       the radiative bhabha calibration is the backup choice.
       If the radiative bhabha calibration is not good then
       we resort to using the cosmic calibration so the
       radiative bhabha requirements should be fairly loose*/
    if (bhabhaGammaCalibUnc[ic] != 0
        && bhabhaGammaCalibUnc[ic] < 2) {     // The limits on the uncertainties are not yet optimized !!!!!!!!!!!!!!!!!!!!!!!!!!
      bhabhaGammaCalibGoodQuality[ic] = true ;
    }
  }


  /* Loop over all crystals and shift the cosmic crystal calibration by
     the crate calibration because the cosmic calibration was made
     assuming the crate calibration is 0.*/

  vector<float> crateCalibShort(m_numCrates);
  vector<float> cosmicCalibShifted = cosmicCalib;
  vector<float> cosmicCalibShiftedUnc = cosmicCalibUnc;  // Do not modify uncertainty?
  for (int ic = 0; ic < m_numCrystals; ic++) {
    cosmicCalibShifted[ic] -= crateCalib[ic];
    int crate_id_from_crystal = crystalMapper->getCrateID(ic + 1);
    crateCalibShort[crate_id_from_crystal - 1] = crateCalib[ic];
  }

  for (int icrate = 0; icrate < m_numCrates; icrate++) {
    B2INFO("crate time calibration for crate " <<
           icrate + 1 << " = " << crateCalibShort[icrate]);
  }


  /* Keep track of the mean value of the crystal calibration constants
     for each crate.  Exclude crystals with bad quality or that have
     a value of 0 (except reference crystals). */
  vector<float> meanGoodCrystalCalibPerCrate_bhabha(m_numCrates, 0);
  vector<float> meanGoodCrystalCalibPerCrate_cosmic(m_numCrates, 0);

  /* Start num entries at 1 because we will skip all 0s but include
     the reference crystal (ts=0) by just increasing the counter.
     We only need a counter for the bhabha events since we want to
     get the same sample of crystals for both the bhabha and cosmic
     calibrations.*/
  vector<short> numGoodCrystalCalibPerCrate_bhabha(m_numCrates, 1);


  /* Loop over all crystals and save the crystal calibration information
     for each crate. */
  for (int ic_base1 = 1; ic_base1 <= m_numCrystals; ic_base1++) {
    int crate_id_from_crystal = crystalMapper->getCrateID(ic_base1);

    if (!((bhabhaCalib[ic_base1 - 1] == 0) || (! bhabhaCalibGoodQuality[ic_base1 - 1]))) {
      meanGoodCrystalCalibPerCrate_bhabha[crate_id_from_crystal - 1] += bhabhaCalib[ic_base1 - 1];
      meanGoodCrystalCalibPerCrate_cosmic[crate_id_from_crystal - 1] += cosmicCalibShifted[ic_base1 - 1];
      numGoodCrystalCalibPerCrate_bhabha[crate_id_from_crystal - 1]++ ;
    }
  }

  /* Complete the calculation of the mean by dividing the sum by the number of entries */
  for (int icrate = 0; icrate < m_numCrates; icrate++) {
    meanGoodCrystalCalibPerCrate_bhabha[icrate] /= numGoodCrystalCalibPerCrate_bhabha[icrate] ;
    meanGoodCrystalCalibPerCrate_cosmic[icrate] /= numGoodCrystalCalibPerCrate_bhabha[icrate] ;
    B2INFO("Mean bhabha crystal calibration for crate " << icrate + 1 << " = " << meanGoodCrystalCalibPerCrate_bhabha[icrate]);
    B2INFO("Mean cosmic crystal calibration for crate " << icrate + 1 << " = " << meanGoodCrystalCalibPerCrate_cosmic[icrate]);
    B2INFO("Number of good crystal calibrations for crate" << icrate + 1 << " = " << numGoodCrystalCalibPerCrate_bhabha[icrate]);
  }


  /** Now correct the cosmic calibrations by the bhabha calibration */

  /* Loop over all crystals and correct the cosmic calibration by the cosmic calibration
     for each crate by the average difference to the bhabha calibration. */
  for (int ic_base1 = 1; ic_base1 <= m_numCrystals; ic_base1++) {
    int crate_id_from_crystal = crystalMapper->getCrateID(ic_base1);
    cosmicCalibShifted[ic_base1 - 1] += meanGoodCrystalCalibPerCrate_bhabha[crate_id_from_crystal - 1] -
                                        meanGoodCrystalCalibPerCrate_cosmic[crate_id_from_crystal - 1] ;
  }


  //------------------------------------------------------------------------
  /** Calculate the new ECLCrystalTimeOffset from existing payloads.  Use
      the bhabha calibrations as the default but then resort to the
      radiative bhabha calibrations if the bhabha calibraitons are not
      good enough (most likely to be when the electrons are outside
      the CDC acceptance).  If the radiative bhabha calibrations are good
      or unavailable then use the cosmic calibration values, shifted
      accordingly to align with the bhabha sample.*/


  vector<float> newCalib(m_numCrystals);
  vector<float> newCalibUnc(m_numCrystals);


  // Loop over each crystal we want to check
  for (int ic = 0; ic < m_numCrystals; ic++) {
    B2DEBUG(29, "Merging crystal " << ic + 1);
    int crate_id_from_crystal = crystalMapper->getCrateID(ic + 1);

    // Determine if the current crystal is a reference crystal
    bool isRefCrys = false ;
    for (int icrate = 0; icrate < m_numCrates; icrate++) {
      if (ic + 1 == refCrystalPerCrate[icrate]) {
        isRefCrys = true;
        B2INFO("Crystal " << ic + 1 << " is a reference crystal");
        break;
      }
      B2DEBUG(29, "Checked reference crystal for crate " <<  icrate + 1);
    }
    B2DEBUG(29, "Checked ref crystal");

    string whichCalibUsed = "bhabha";
    // By default set the new calibration to the bhabha calibration
    newCalib[ic] = bhabhaCalib[ic];
    newCalibUnc[ic] = bhabhaCalibUnc[ic];

    B2DEBUG(29, "Set new calib to bhabha for now...");

    /* If the crystal is not a reference crystal and there is no bhabha
       calib for that crystal then use a different calibration.  If the
       crystal is not a reference crystal and the crystal has a bad bhabha
       crystal calibration uncertainty. */
    /*
        if (    (!isRefCrys)                 &&
                (  (bhabhaCalib[ic] == 0)    ||
                   (  (bhabhaCalibUnc[ic]>2) && ( (ic<=288) || (ic>=8545) )  )   )    ) {}
    */
    if ((!isRefCrys)             &&
        ((bhabhaCalib[ic] == 0) || (! bhabhaCalibGoodQuality[ic]))) {

      /* If there is a radiative bhabha calibration for this crystal then
         use it otherwise use the cosmic calibration */
      if (bhabhaGammaCalib[ic] != 0 || bhabhaGammaCalibGoodQuality[ic]) {
        newCalib[ic] = bhabhaGammaCalib[ic] ;
        newCalibUnc[ic] = bhabhaGammaCalibUnc[ic];
        whichCalibUsed = "bhabhaGamma";
      } else {
        newCalib[ic] = cosmicCalibShifted[ic] ;
        newCalibUnc[ic] = cosmicCalibShiftedUnc[ic];
        whichCalibUsed = "cosmic";
      }
    }
    B2INFO("Crystal " << ic + 1 << ", crate " << crate_id_from_crystal <<
           "::: bhabha = " << bhabhaCalib[ic] << "+-" << bhabhaCalibUnc[ic] <<
           " ticks, rad bhabha = " << bhabhaGammaCalib[ic] << "+-" << bhabhaGammaCalibUnc[ic]  <<
           " ticks, cosmic UNshifted = " << cosmicCalib[ic] << "+-" << cosmicCalibUnc[ic]  <<
           " ticks, cosmic shifted = " << cosmicCalibShifted[ic] << "+-" << cosmicCalibShiftedUnc[ic]  <<
           " ticks. ||| New calib = " << newCalib[ic] << " ticks, selected " << whichCalibUsed);
  } // end of setting new calibration for each crystal


  //------------------------------------------------------------------------
  //..Histograms of existing calibration, new calibration, and ratio new/old

  // Just in case, we remember the current TDirectory so we can return to it
  TDirectory* executeDir = gDirectory;

  TString fname = m_payloadName;
  fname += ".root";
  TFile hfile(fname, "recreate");

  TString htitle = m_payloadName;
  htitle += " bhabha values;cellID";
  TH1F* bhabhaPayload = new TH1F("bhabhaPayload", htitle, m_numCrystals, 1, 8737);

  htitle = m_payloadName;
  htitle += " bhabhaGamma values;cellID";
  TH1F* bhabhaGammaPayload = new TH1F("bhabhaGammaPayload", htitle, m_numCrystals, 1, 8737);

  htitle = m_payloadName;
  htitle += " cosmic unshifted values;cellID";
  TH1F* cosmicPayload = new TH1F("cosmicUnshiftedPayload", htitle, m_numCrystals, 1, 8737);

  htitle = m_payloadName;
  htitle += " cosmic shifted values;cellID";
  TH1F* cosmicShiftedPayload = new TH1F("cosmicShiftedPayload", htitle, m_numCrystals, 1, 8737);

  htitle = m_payloadName;
  htitle += " new values;cellID";
  TH1F* newPayload = new TH1F("newPayload", htitle, m_numCrystals, 1, 8737);


  for (int cellID = 1; cellID <= m_numCrystals; cellID++) {
    bhabhaPayload->SetBinContent(cellID, bhabhaCalib[cellID - 1]);
    bhabhaPayload->SetBinError(cellID, bhabhaCalibUnc[cellID - 1]);

    bhabhaGammaPayload->SetBinContent(cellID, bhabhaGammaCalib[cellID - 1]);
    bhabhaGammaPayload->SetBinError(cellID, bhabhaGammaCalibUnc[cellID - 1]);

    cosmicPayload->SetBinContent(cellID, cosmicCalib[cellID - 1]);
    cosmicPayload->SetBinError(cellID, cosmicCalibUnc[cellID - 1]);

    cosmicShiftedPayload->SetBinContent(cellID, cosmicCalibShifted[cellID - 1]);
    cosmicShiftedPayload->SetBinError(cellID, cosmicCalibShiftedUnc[cellID - 1]);

    newPayload->SetBinContent(cellID, newCalib[cellID - 1]);
    newPayload->SetBinError(cellID, newCalibUnc[cellID - 1]);
  }

  hfile.cd();
  hfile.Write();
  hfile.Close();
  B2INFO("Debugging histograms written to " << fname);
  // Go back to original TDirectory
  executeDir->cd();


  /* Re-save the new bhabha calibrations to a payload.  The bhabha calibrations have NOT
     been changed; however, the best way of controling which payloads get get uploaded to
     the GT as part of the prommp calibrations is to save a second copy in this calibration
     directory since we don't want to save all the payloads from the previous directory.
     See the "<calibration>.save_payloads = <True/False>" code in the airflow steering file.*/
  ECLCrystalCalib* bhabhaCalibCopy = new ECLCrystalCalib();
  bhabhaCalibCopy->setCalibVector(bhabhaCalib, bhabhaCalibUnc);
  saveCalibration(bhabhaCalibCopy, "ECLCrystalTimeOffsetBhabha");

  // Save the new merged calibrations to a payload
  ECLCrystalCalib* newCrystalTimes = new ECLCrystalCalib();
  newCrystalTimes->setCalibVector(newCalib, newCalibUnc);
  saveCalibration(newCrystalTimes, m_payloadName);
  return c_OK;
}
