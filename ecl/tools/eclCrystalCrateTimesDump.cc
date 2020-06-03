/*******************************************************************************************
 * BASF2 (Belle Analysis Framework 2)
 * Copyright(C) 2019 - Belle II Collaboration
 *
 * Author: The Belle II Collaboration
 * Contributors: Ewan Hill (ehill@mail.ubc.ca)
 *
 * This software is provided "as is" without any warranty.
 *
 * - Read in a crate time payload and output the same content
 *   under a different name for a different tool to compare crate times
 *   between different runs
 *
 *   Usage: eclCrystalCrateTimesDump experiment run outputFileName saveCrystalsBool gt
 *
 *   e.g
 *      eclCrystalCrateTimesDump 8 3123   test_falseSaveCrystal.root  0   online
 *   or
 *      eclCrystalCrateTimesDump 8 3123   test_trueSaveCrystal.root   1   online
 *
 * - existing root files for output will be UPDATED, NOT REPLACED
 * - the localdb directory will be searched before the global tag
 *
 *
 ******************************************************************************************/

#include <framework/database/DBImportObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/database/Database.h>
#include <framework/database/LocalDatabase.h>
#include <framework/database/DatabaseChain.h>
#include <framework/database/ConditionsDatabase.h>
#include <framework/logging/LogSystem.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <ecl/digitization/EclConfiguration.h>
#include <iostream>
#include <fstream>
#include <TFile.h>
#include <TH1F.h>
#include <TTree.h>
#include <ecl/utility/ECLChannelMapper.h>
#include <string>

using namespace Belle2;
using namespace ECL ;



/** Set experiment, run, and event numbers before reading a payload from the DB */
namespace {
  void setupDatabase(int exp, int run, int eventNr = 1)
  {
    StoreObjPtr<EventMetaData> evtPtr;
    // simulate the initialize() phase where we can register objects in the DataStore
    DataStore::Instance().setInitializeActive(true);
    evtPtr.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);
    // now construct the event metadata
    evtPtr.construct(eventNr, run, exp);
    // and update the database contents
    DBStore& dbstore = DBStore::Instance();
    dbstore.update();
    // this is only needed it the payload might be intra-run dependent,
    // that is if it might change during one run as well
    dbstore.updateEvent();
  }
}


int main(int argc, char** argv)
{
  //------------------------------------------------------------------------
  //..Check the arguments
  if (argc < 5 || argc > 6) {
    std::cout << "incorrect number of arguments" << std::endl;
    std::cout << "Number arguments provided = " << argc << "\n" ;
    std::cout << "Program name counts as one argument" ;
    return -1;
  }

  std::cout << "Set experiment and run number ranges\n" ;
  int experiment = std::stoi(argv[1]);
  std::cout << "experiment = " << experiment << "\n" ;
  int run = std::stoi(argv[2]);
  std::cout << "run = " << run << "\n" ;


  std::cout << "outfile name argument = " << argv[3] << "\n" ;
  std::string outputFileName = "crystalCrateTimes.root" ;
  std:: cout << "initial output file name = " << outputFileName << "\n" ;
  outputFileName = std::string(argv[3]);
  std:: cout << "output file name = " << outputFileName << "\n" ;

  int storeCrystalInfo = std::stoi(argv[4]) ;
  bool userGT = false ;

  std::string gtName = "data_reprocessing_prompt";
  if (argc == 6) { // 5 passed arguments by user + 1 filled by function name
    std::string(gtName = argv[5]);
    userGT = true ;
  }

  std::cout << "eclCrystalCrateTimesDump called with arguments for exp = " << experiment << ", run = " << run
            << " with GT = " << gtName  << " to be saved to " << outputFileName << std::endl;
  std::cout << "Crystal time information will be saved to tree (true/false) = " << storeCrystalInfo << "\n" ;



  //------------------------------------------------------------------------
  // Set up the file for output

  TString fname = outputFileName ;
  TFile tCrystalCratefile(fname, "UPDATE");

  tCrystalCratefile.cd();


  // Set up the crystal id to crate id mapper
  std::shared_ptr< ECL::ECLChannelMapper > crystalMapper(new ECL::ECLChannelMapper()) ;
  crystalMapper->initFromDB();


  // Conversion coefficient from ADC ticks to nanoseconds
  const double TICKS_TO_NS = 1.0 / (4.0 * EclConfiguration::m_rf) *
                             1e3;  // 1/(4fRF) = 0.4913 ns/clock tick, where fRF is the accelerator RF frequency, fRF=508.889 MHz. Same for all crystals.  Proper accurate value


  //------------------------------------------------------------------------
  // Set up two trees in the output file

  //==============
  // Set up a tree just for the crate times - only 52 crates so only 52 entries
  TTree* m_tree_crateTimes ;

  m_tree_crateTimes = new TTree("tree_crate", "crate times") ;

  int m_crateid;     /**< crate id */
  int m_expNum;     /**< experiment number */
  int m_runNum;     /**< run number */
  double m_tcrate;     /**< crate time */
  double m_tcrate_unc;     /**< crate time uncertainty */

  m_tree_crateTimes->Branch("crateid" , &m_crateid)  ->SetTitle("Crate id") ;
  m_tree_crateTimes->Branch("expNum" , &m_expNum)  ->SetTitle("Experiment number") ;
  m_tree_crateTimes->Branch("runNum" , &m_runNum)  ->SetTitle("Run number") ;
  m_tree_crateTimes->Branch("tcrate" , &m_tcrate)  ->SetTitle("crate time") ;
  m_tree_crateTimes->Branch("tcrate_unc" , &m_tcrate_unc)  ->SetTitle("Crate crate time uncertainty") ;

  m_tree_crateTimes->SetAutoSave(10) ;


  //==============
  // Set up a tree for the crystal and crate times - 8736 entries
  TTree* m_tree_crystalCrateTimes ;

  m_tree_crystalCrateTimes = new TTree("tree_crysCrateTimes", "crystal and crate times") ;

  // Additional properties
  int m_crystalid;     /**< crystal id */
  double m_tcrystal;     /**< crystal time */
  double m_tcrystal_unc;     /**< crystal time uncertainty */

  m_tree_crystalCrateTimes->Branch("crystal_id" , &m_crystalid)  ->SetTitle("Crystal id") ;
  m_tree_crystalCrateTimes->Branch("crate_id" , &m_crateid)  ->SetTitle("Crate id") ;
  m_tree_crystalCrateTimes->Branch("expNum" , &m_expNum)  ->SetTitle("Experiment number") ;
  m_tree_crystalCrateTimes->Branch("runNum" , &m_runNum)  ->SetTitle("Run number") ;
  m_tree_crystalCrateTimes->Branch("tcrate" , &m_tcrate)  ->SetTitle("crate time") ;
  m_tree_crystalCrateTimes->Branch("tcrate_unc" , &m_tcrate_unc)  ->SetTitle("Crate crate time uncertainty") ;
  m_tree_crystalCrateTimes->Branch("tcrystal" , &m_tcrystal)  ->SetTitle("crystal time") ;
  m_tree_crystalCrateTimes->Branch("tcrystal_unc" , &m_tcrystal_unc)  ->SetTitle("Crystal time uncertainty") ;

  m_tree_crystalCrateTimes->SetAutoSave(10) ;



  //------------------------------------------------------------------------
  /** Specify database. Last one specified is first one used */
  Database::reset();
  bool resetIovs = false;
  DatabaseChain::createInstance(resetIovs);

  if (userGT) {
    ConditionsDatabase::createDefaultInstance(gtName, LogConfig::c_Debug);
  }

  LocalDatabase::createInstance("localdb/database.txt", "", LogConfig::c_Debug);

  /** Create the DBObjPtr for the payloads that we want to read from the DB */
  DBObjPtr<Belle2::ECLCrystalCalib> crystalTimeOffsets("ECLCrystalTimeOffset");
  DBObjPtr<Belle2::ECLCrystalCalib> crateTimeOffsets("ECLCrateTimeOffset");


  /** Populate database contents */
  setupDatabase(experiment, run);

  //------------------------------------------------------------------------
  /** Get the vectors from the input payload */

  std::vector<float> crystalTimeOffsetsCalib;
  std::vector<float> crystalTimeOffsetsCalibUnc;
  crystalTimeOffsetsCalib = crystalTimeOffsets->getCalibVector();
  crystalTimeOffsetsCalibUnc = crystalTimeOffsets->getCalibUncVector();

  std::vector<float> crateTimeOffsetsCalib;
  std::vector<float> crateTimeOffsetsCalibUnc;
  crateTimeOffsetsCalib = crateTimeOffsets->getCalibVector();
  crateTimeOffsetsCalibUnc = crateTimeOffsets->getCalibUncVector();

  //------------------------------------------------------------------------
  /**  Make a crate time offset vector with an entry per crate (instead of per crystal) and convert from ADC counts to ns. */
  std::vector<float> Crate_time_ns(52, 0.0) ; /**< vector derived from DB object */
  std::vector<float> Crate_time_unc_ns(52, 0.0) ; /**< vector derived from DB object */

  for (int crysID = 1; crysID <= 8736; crysID++) {
    int crateID_temp = crystalMapper->getCrateID(crysID) ;
    Crate_time_ns[crateID_temp - 1] = crateTimeOffsetsCalib[crysID - 1] * TICKS_TO_NS ;
    Crate_time_unc_ns[crateID_temp - 1] = crateTimeOffsetsCalibUnc[crysID - 1] * TICKS_TO_NS ;
    //std::cout << "Looking up crate times: Crate_time_ns[crateID_temp - 1] = " << Crate_time_ns[crateID_temp - 1] << ", crateID_temp = " << crateID_temp << ", crysID = " << crysID << "\n" ;
  }

  //------------------------------------------------------------------------
  /**  Fill the crate times tree */
  m_expNum = experiment ;
  m_runNum = run ;
  for (int crateIndex = 0; crateIndex <= 51; crateIndex++) {
    m_crateid = crateIndex + 1 ;
    m_tcrate = Crate_time_ns[crateIndex] ;
    m_tcrate_unc = Crate_time_unc_ns[crateIndex] ;

    m_tree_crateTimes->Fill() ;
  }

  //------------------------------------------------------------------------
  /**  Fill the crystal and crate times tree */
  if (storeCrystalInfo) {
    m_expNum = experiment ;
    m_runNum = run ;
    for (int crysID = 1; crysID <= 8736; crysID++) {
      m_crystalid = crysID ;
      m_tcrystal = crystalTimeOffsetsCalib[crysID - 1] * TICKS_TO_NS ;
      m_tcrystal_unc = crystalTimeOffsetsCalibUnc[crysID - 1] * TICKS_TO_NS ;
      m_crateid = crystalMapper->getCrateID(crysID) ;
      m_tcrate = crateTimeOffsetsCalib[crysID - 1] * TICKS_TO_NS ;
      m_tcrate_unc = crateTimeOffsetsCalibUnc[crysID - 1] * TICKS_TO_NS ;

      m_tree_crystalCrateTimes->Fill() ;
    }
  }

  //------------------------------------------------------------------------
  /** Write out a few values for quality control purposes */
  for (int ic = 0; ic <= 51; ic++) {
    std::cout << ic + 1 << " " << Crate_time_ns[ic] << " +- " << Crate_time_unc_ns[ic] << " ns" << std::endl;
  }

  tCrystalCratefile.Write();
  tCrystalCratefile.Close();
  std::cout << std::endl << "Values written to " << fname << std::endl;

}


