/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ewan Hill (ehill@mail.ubc.ca)                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 *                                                                        *
 * - Read in a crate time payload and output the same content             *
 *   under a different name for a different tool to compare crate times   *
 *   between different runs                                               *
 *                                                                        *
 *   Usage: eclCrateTimesDump expStart runStart expEnd runEnd             *
 *                                                                        *
 **************************************************************************/

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
  if (argc < 4 || argc > 6) {
    std::cout << "incorrect number of arguments" << std::endl;
    return -1;
  }

  std::cout << "Set experiment and run number ranges\n" ;
  std::cout << "e start = " << argv[1] << "\n" ;
  int experimentStart = std::stoi(argv[1]);
  std::cout << "e start" ;
  int runStart = std::stoi(argv[2]);
  std::cout << "r start" ;
  int experimentEnd = std::stoi(argv[3]);
  std::cout << "e end" ;
  int runEnd  = std::stoi(argv[4]);
  std::cout << "e end" ;
  /*
    std::string gtName = "data_reprocessing_prompt_bucket7";
    if (argc >= 5) {gtName = argv[5];}
  */

  //int experimentStart = 3 ;
  //int runStart = 1162 ;
  //int experimentEnd =  3 ;
  //int runEnd  =  1162 ;
  std::string gtName = "data_reprocessing_prompt_bucket7";


  std::cout << "eclCrateTimesDump called with arguments for all runs between exp = " << experimentStart << ", run = " << runStart
            << " and exp = " << experimentEnd << ", run = " << runEnd << "with GT =  " << gtName << std::endl;




  //------------------------------------------------------------------------
  // Set up the file for output
  TString outputTitle = "ECLCrateTimes";
  outputTitle += "_";
  outputTitle += experimentStart;
  outputTitle += "_";
  outputTitle += runStart;
  outputTitle += "-";
  outputTitle += experimentEnd;
  outputTitle += "_";
  outputTitle += runEnd;

  TString fname = outputTitle;
  fname += ".root";
  TFile tcratefile(fname, "recreate");

  tcratefile.cd();





  std::shared_ptr< ECL::ECLChannelMapper > crystalMapper(new ECL::ECLChannelMapper()) ;
  crystalMapper->initFromDB();


  // Conversion coefficient from ADC ticks to nanoseconds
  const double TICKS_TO_NS = 1.0 / (4.0 * EclConfiguration::m_rf) *
                             1e3;  // 1/(4fRF) = 0.4913 ns/clock tick, where fRF is the accelerator RF frequency, fRF=508.889 MHz. Same for all crystals.  Proper accurate value


  TTree* m_tree_crateTimes ;

  // Per max E crystal entry after all the cuts
  m_tree_crateTimes = new TTree("tree_crateTimes", "crate times") ;

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

  std::cout << "here 1\ns" ;
  //------------------------------------------------------------------------
  /** Specify database. Last one specified is first one used */
  Database::reset();
  bool resetIovs = false;
  DatabaseChain::createInstance(resetIovs);
  ConditionsDatabase::createDefaultInstance(gtName, LogConfig::c_Debug);
  LocalDatabase::createInstance("localdb/database.txt", "", LogConfig::c_Debug);

  /** Create the DBObjPtr for the payloads that we want to read from the DB */
  DBObjPtr<Belle2::ECLCrystalCalib> Existing("ECLCrateTimeOffset");


  std::cout << "here 2\ns" ;
  //------------------------------------------------------------------------
  /** Loop over all the experiments and runs and extract the crate times*/
  for (int experiment = experimentStart; experiment <= experimentEnd; experiment++) {
    for (int run = runStart; run <= runEnd; run++) {

      std::cout << "here 3\ns" ;


      /** Populate database contents */
      setupDatabase(experiment, run);

      //------------------------------------------------------------------------
      /** Get the vectors from the input payload */

      std::vector<float> ExistingCalib;
      std::vector<float> ExistingCalibUnc;
      ExistingCalib = Existing->getCalibVector();
      ExistingCalibUnc = Existing->getCalibUncVector();

      //------------------------------------------------------------------------
      /**  Make a crate time offset vector with an entry per crate (instead of per crystal) and convert from ADC counts to ns. */
      std::vector<float> Crate_time_ns(52, 0.0) ; /**< vector derived from DB object */
      std::vector<float> Crate_time_unc_ns(52, 0.0) ; /**< vector derived from DB object */

      for (int crysID = 1; crysID <= 8736; crysID++) {
        int crateID_temp = crystalMapper->getCrateID(crysID) ;
        Crate_time_ns[crateID_temp - 1] = ExistingCalib[crysID] * TICKS_TO_NS ;
        Crate_time_unc_ns[crateID_temp - 1] = ExistingCalibUnc[crysID] * TICKS_TO_NS ;
      }

      //------------------------------------------------------------------------
      /**  Set the values to the variables that will be used for the tree writing */
      m_expNum = experiment ;
      m_runNum = run ;
      for (int crateIndex = 0; crateIndex <= 51; crateIndex++) {
        m_crateid = crateIndex + 1 ;
        m_tcrate = Crate_time_ns[crateIndex] ;
        m_tcrate_unc = Crate_time_unc_ns[crateIndex] ;

        m_tree_crateTimes->Fill() ;
      }

      //------------------------------------------------------------------------
      /** Write out a few values for quality control purposes */
      for (int ic = 0; ic < 52; ic += 15) {
        std::cout << ic + 1 << " " << Crate_time_ns[ic] << " +- " << Crate_time_unc_ns[ic] << " ns" << std::endl;
      }
    }
  }

  tcratefile.Write();
  tcratefile.Close();
  std::cout << std::endl << "Values written to " << fname << std::endl;

}


