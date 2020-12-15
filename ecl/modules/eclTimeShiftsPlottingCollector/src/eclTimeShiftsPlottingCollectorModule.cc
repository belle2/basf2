/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *    Ewan Hill       (ehill@mail.ubc.ca)                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclTimeShiftsPlottingCollector/eclTimeShiftsPlottingCollectorModule.h>
#include <framework/gearbox/Const.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <ecl/dbobjects/ECLReferenceCrystalPerCrateCalib.h>
#include <ecl/digitization/EclConfiguration.h>
#include <ecl/geometry/ECLGeometryPar.h>

#include <TH2F.h>
#include <TTree.h>
#include <TFile.h>

using namespace Belle2;
using namespace ECL;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(eclTimeShiftsPlottingCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

eclTimeShiftsPlottingCollectorModule::eclTimeShiftsPlottingCollectorModule() : CalibrationCollectorModule(),
  m_CrystalTimeDB("ECLCrystalTimeOffset"),
  m_CrateTimeDB("ECLCrateTimeOffset"),
  m_RefCrystalsCalibDB("ECLReferenceCrystalPerCrateCalib")//,
{
  setDescription("This module reads the crystal and crate time offset information from the database");

  // specify this flag if you need parallel processing
  setPropertyFlags(c_ParallelProcessingCertified);
}

eclTimeShiftsPlottingCollectorModule::~eclTimeShiftsPlottingCollectorModule()
{
}

void eclTimeShiftsPlottingCollectorModule::inDefineHisto()
{
}

void eclTimeShiftsPlottingCollectorModule::prepare()
{

  B2INFO("eclTimeShiftsPlottingCollector: Experiment = " << m_evtMetaData->getExperiment() <<
         "  run = " << m_evtMetaData->getRun());

  /* -----------------
     Store the information about the crystal and crate times
  -----------------*/
  string objectName = "tree_perCrystal";
  TTree* tree_crys = new TTree(objectName.c_str(), "");
  tree_crys->Branch<int>("run", &m_run);
  tree_crys->Branch<int>("exp", &m_exp);
  tree_crys->Branch<double>("crateTimeConst", &m_crateTimeConst);
  tree_crys->Branch<double>("crystalTimeConst", &m_crystalTimeConst);
  tree_crys->Branch<double>("crateTimeUnc", &m_crateTimeConstUnc);
  tree_crys->Branch<double>("crystalTimeUnc", &m_crystalTimeConstUnc);
  tree_crys->Branch<int>("crystalID", &m_crystalID);
  tree_crys->Branch<int>("crateID", &m_crateID);
  tree_crys->Branch<int>("refCrystalID", &m_refCrystalID);

  // We register the objects so that our framework knows about them.
  // Don't try and hold onto the pointers or fill these objects directly
  // Use the getObjectPtr functions to access collector objects
  registerObject<TTree>(objectName, tree_crys);

}

void eclTimeShiftsPlottingCollectorModule::collect()
{
  m_run = m_evtMetaData->getRun();
  m_exp = m_evtMetaData->getExperiment();

  /* Check if we have looked at this (exp,run).  If we have not
     then store the information to the trees and update the
     (exp previous, run previous) variables for the next comparison.
  */
  if (m_run != m_previousRun or m_exp != m_previousExp or m_exp < 0 or m_run < 0) {
    /* Use ECLChannelMapper to get other detector indices for the crystals */
    /* For conversion from CellID to crate, shaper, and channel ids. */

    // Use smart pointer to avoid memory leak when the ECLChannelMapper object needs destroying at the end of the event.
    shared_ptr< ECL::ECLChannelMapper > crystalMapper(new ECL::ECLChannelMapper());
    crystalMapper->initFromDB();

    // Get the previous crystal time offset (the same thing that this calibration is meant to calculate).
    // This can be used for testing purposes, and for the crate time offset.
    if (m_CrystalTimeDB.hasChanged()) {
      m_CrystalTime = m_CrystalTimeDB->getCalibVector();
      m_CrystalTimeUnc = m_CrystalTimeDB->getCalibUncVector();
    }
    if (m_CrateTimeDB.hasChanged()) {
      m_CrateTime = m_CrateTimeDB->getCalibVector();
      m_CrateTimeUnc = m_CrateTimeDB->getCalibUncVector();
    }
    B2DEBUG(35, "Finished checking if previous crate time payload has changed");
    B2DEBUG(35, "m_CrateTime size = " << m_CrateTime.size());
    B2DEBUG(29, "Crate time +- uncertainty [0]= " << m_CrateTime[0] << " +- " << m_CrateTimeUnc[0]);
    B2DEBUG(29, "Crate time +- uncertainty [8735]= " << m_CrateTime[8735] << " +- " << m_CrateTimeUnc[8735]);

    B2DEBUG(35, "Finished checking if previous crate time payload has changed");
    if (m_RefCrystalsCalibDB.hasChanged()) {
      m_RefCrystalsCalib = m_RefCrystalsCalibDB->getReferenceCrystals();
    }
    B2DEBUG(35, "Finished checking if reference crystal ids payload has changed");


    B2DEBUG(29, "eclTimeShiftsPlottingCollector:: loaded ECLCrystalTimeOffset from the database"
            << LogVar("IoV", m_CrystalTimeDB.getIoV())
            << LogVar("Revision", m_CrystalTimeDB.getRevision()));
    B2DEBUG(29, "eclTimeShiftsPlottingCollector:: loaded ECLCrateTimeOffset from the database"
            << LogVar("IoV", m_CrateTimeDB.getIoV())
            << LogVar("Revision", m_CrateTimeDB.getRevision()));
    B2DEBUG(29, "eclTimeShiftsPlottingCollector:: loaded ECLReferenceCrystalPerCrateCalib from the database"
            << LogVar("IoV", m_RefCrystalsCalibDB.getIoV())
            << LogVar("Revision", m_RefCrystalsCalibDB.getRevision()));

    string objectName = "tree_perCrystal";
    auto tree_perCrystal = getObjectPtr<TTree>(objectName);

    /* Loop over all the crystals and store the crystal and crate time information
       to the root tree for crytals */
    for (int crysID = 1; crysID <= NUM_CRYSTALS; crysID++) {
      m_crystalTimeConst    = m_CrystalTime[crysID - 1];
      m_crystalTimeConstUnc = m_CrystalTimeUnc[crysID - 1];
      m_crateTimeConst    = m_CrateTime[crysID - 1];
      m_crateTimeConstUnc = m_CrateTimeUnc[crysID - 1];
      m_crystalID = crysID;
      int crateID_temp = crystalMapper->getCrateID(crysID);
      m_crateID = crateID_temp;
      m_refCrystalID = m_RefCrystalsCalib[crateID_temp - 1];
      tree_perCrystal->Fill();
    }

    m_previousExp = m_exp ;
    m_previousRun = m_run ;
  }

}
