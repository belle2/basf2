/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/**************************************************************************
 * Description:                                                           *
 * Tool to perform validations using bhabha events.                       *
 * The selections here are very similar to the selections used to         *
 * perform the bhabha based timing calibrations; however,                 *
 * there are some small differences so that the cuts are slightly         *
 * different.  Further differences could appear if the two codes          *
 * are not updated simultaneously if/when there are changes.              *
 * The intention of this code is just to make sure the calibrations       *
 * results are self consistent and to validate the calibrations           *
 * as a whole for a collection of bhabha events rather than to            *
 * validate the cluster times of each individual cluster from the         *
 * original calibration code.  Hopefully the selection is looser          *
 * so that fewer events can be used to validate than are required         *
 * to perform the calibration itself.                                     *
 **************************************************************************/

/* Own header. */
#include <ecl/modules/eclBhabhaTimeCalibrationValidationCollector/eclBhabhaTimeCalibrationValidationCollectorModule.h>

/* ECL headers. */
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLElementNumbers.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <ecl/digitization/EclConfiguration.h>

/* Basf2 headers. */
#include <analysis/utility/PCmsLabTransform.h>
#include <framework/gearbox/Const.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/Track.h>

/* ROOT headers. */
#include <TH2F.h>
#include <TTree.h>

using namespace Belle2;
using namespace ECL;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(eclBhabhaTimeCalibrationValidationCollector);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

eclBhabhaTimeCalibrationValidationCollectorModule::eclBhabhaTimeCalibrationValidationCollectorModule() :
  CalibrationCollectorModule(),
  m_dbg_tree_electronClusters(0),
  m_dbg_tree_event(0),
  m_dbg_tree_run(0),
  m_CrateTimeDB("ECLCrateTimeOffset"),
  m_channelMapDB("ECLChannelMap")//,
{
  setDescription("This module validates the ECL cluster times");

  addParam("timeAbsMax", m_timeAbsMax, // (Time in ns)
           "Events with fabs(getTimeFit) > m_timeAbsMax "
           "are excluded", (short)80);

  addParam("saveTree", m_saveTree,
           "If true, TTree 'tree' with more detailed event info is saved in "
           "the output file specified by HistoManager",
           false);

  addParam("looseTrkZ0", m_looseTrkZ0, "max Z0 for loose tracks (cm)", 10.);
  addParam("tightTrkZ0", m_tightTrkZ0, "max Z0 for tight tracks (cm)", 2.);
  addParam("looseTrkD0", m_looseTrkD0, "max D0 for loose tracks (cm)", 2.);
  addParam("tightTrkD0", m_tightTrkD0, "max D0 for tight tracks (cm)", 0.5);  // beam pipe radius = 1cm in 2019
  addParam("skipTrgSel", skipTrgSel, "boolean to skip the trigger skim selection", false);


  // specify this flag if you need parallel processing
  setPropertyFlags(c_ParallelProcessingCertified);
}

eclBhabhaTimeCalibrationValidationCollectorModule::~eclBhabhaTimeCalibrationValidationCollectorModule()
{
}

void eclBhabhaTimeCalibrationValidationCollectorModule::inDefineHisto()
{

}

void eclBhabhaTimeCalibrationValidationCollectorModule::prepare()
{
  //=== Prepare TTree for debug output
  if (m_saveTree) {
    // Per electron cluster
    m_dbg_tree_electronClusters = new TTree("tree_electronClusters",
                                            "Validating crystal and crate time calibrations using electron clusters in events with lots of tracks and clusters") ;
    m_dbg_tree_electronClusters->Branch("EventNum", &m_tree_evt_num)   ->SetTitle("Event number") ;
    m_dbg_tree_electronClusters->Branch("cluster_time", &m_tree_time)   ->SetTitle("Cluster time t (calibrated), ns") ;
    m_dbg_tree_electronClusters->Branch("clust_E", &m_E_electron_clust)   ->SetTitle("Electron type cluster energy, GeV") ;
    m_dbg_tree_electronClusters->Branch("t0", &m_tree_t0)     ->SetTitle("T0, ns") ;
    m_dbg_tree_electronClusters->Branch("t0_unc", &m_tree_t0_unc)     ->SetTitle("T0 uncertainty, ns") ;
    m_dbg_tree_electronClusters->Branch("runNum", &m_tree_run)     ->SetTitle("Run number") ;
    m_dbg_tree_electronClusters->Branch("CrystalCellID", &m_tree_cid)    ->SetTitle("Cell ID, 1..8736") ;
    m_dbg_tree_electronClusters->Branch("dt99", &m_tree_dt99)   ->SetTitle("Cluster dt99, ns") ;
    m_dbg_tree_electronClusters->SetAutoSave(10) ;

    // Per event
    m_dbg_tree_event = new TTree("tree_event",
                                 "Validating crystal and crate time calibrations using electron clusters in events with lots of tracks and clusters") ;
    m_dbg_tree_event->Branch("EventNum", &m_tree_evt_num)   ->SetTitle("Event number") ;
    m_dbg_tree_event->Branch("t0", &m_tree_t0)     ->SetTitle("T0, ns") ;
    m_dbg_tree_event->Branch("t0_unc", &m_tree_t0_unc)     ->SetTitle("T0 uncertainty, ns") ;
    m_dbg_tree_event->Branch("runNum", &m_tree_run)     ->SetTitle("Run number") ;
    m_dbg_tree_event->Branch("E0", &m_tree_E0)   ->SetTitle("Highest E cluster E") ;
    m_dbg_tree_event->Branch("E1", &m_tree_E1)   ->SetTitle("2nd highest E cluster E") ;
    m_dbg_tree_event->Branch("time_E0", &m_tree_time_fromE0)   ->SetTitle("Cluster time of highest E cluster") ;
    m_dbg_tree_event->Branch("time_E1", &m_tree_time_fromE1)   ->SetTitle("Cluster time of 2nd highest E cluster") ;
    m_dbg_tree_event->SetAutoSave(10) ;


    // Per run
    m_dbg_tree_run = new TTree("tree_run", "Storing crate time constants") ;
    m_dbg_tree_run->Branch("runNum", &m_tree_run)               ->SetTitle("Run number") ;
    m_dbg_tree_run->Branch("crateid", &m_tree_crateid)          ->SetTitle("Crate ID") ;
    m_dbg_tree_run->Branch("tcrate", &m_tree_tcrate)            ->SetTitle("Crate time") ;
    m_dbg_tree_run->Branch("tcrate_unc", &m_tree_tcrate_unc)    ->SetTitle("Crate time uncertainty") ;
    m_dbg_tree_run->SetAutoSave(10) ;

  }


  //=== MetaData
  B2INFO("eclBhabhaTimeCalibrationValidationCollector: Experiment = " << m_EventMetaData->getExperiment() <<
         "  run = " << m_EventMetaData->getRun()) ;

  //=== Create histograms and register them in the data store

  // Define the bin size, which is equivalent to the
  double binSize = 2000.0 / pow(2, 12);
  double halfBinSize = binSize / 2.0;

  /* Determine the number of bins required to go from the edge of the bin centred
     on zero to a value just larger than the negative cut off */
  double nBinsNeg = floor((m_timeAbsMax - halfBinSize) / binSize);
  double min_t = -nBinsNeg * binSize - halfBinSize; // lower edge value of left most bin
  int nbins = nBinsNeg * 2 + 1; // number of negative time bins + t=0 bin + number of positive time bins
  double max_t = min_t + nbins * binSize; // upper edge value of right most bin

  /* Variable bin width information for the time information vs energy since
     the time width should vary as a function of 1/E */
  const Int_t N_E_BIN_EDGES = 64;
  const Int_t N_E_BINS = N_E_BIN_EDGES - 1;
  Double_t energyBinEdges[N_E_BIN_EDGES] = {0, 0.05, 0.051, 0.052, 0.053, 0.054, 0.055, 0.056, 0.057, 0.058, 0.059, 0.06, 0.062, 0.064, 0.066, 0.068, 0.07, 0.075, 0.08, 0.085, 0.09, 0.095, 0.1, 0.11, 0.12, 0.13, 0.14, 0.15, 0.16, 0.17, 0.18, 0.19, 0.2, 0.25, 0.3, 0.35, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 1.2, 1.4, 1.6, 1.8, 2, 2.25, 2.5, 2.8, 3.2, 3.6, 4, 4.4, 4.8, 5.2, 5.6, 6, 6.4, 6.8, 7.2, 7.6, 8};


  auto cutflow = new TH1F("cutflow", " ;Cut label number ;Number of events passing cut", 10, 0, 10) ;
  registerObject<TH1F>("cutflow", cutflow) ;

  auto clusterTime = new TH1F("clusterTime", ";Electron ECL cluster time [ns]; number of ECL clusters", nbins, min_t, max_t) ;
  registerObject<TH1F>("clusterTime", clusterTime) ;

  auto clusterTime_cid = new TH2F("clusterTime_cid",
                                  ";crystal Cell ID ;Electron ECL cluster time [ns]", ECLElementNumbers::c_NCrystals, 1, ECLElementNumbers::c_NCrystals + 1, nbins,
                                  min_t, max_t) ;
  registerObject<TH2F>("clusterTime_cid", clusterTime_cid) ;

  auto clusterTime_run = new TH2F("clusterTime_run",
                                  ";Run number ;Electron ECL cluster time [ns]", 7000, 0, 7000, nbins, min_t, max_t) ;
  registerObject<TH2F>("clusterTime_run", clusterTime_run) ;


  auto clusterTimeClusterE = new TH2F("clusterTimeClusterE",
                                      ";Electron cluster energy [GeV];Electron cluster time [ns]", N_E_BINS, energyBinEdges, nbins, min_t, max_t) ;
  registerObject<TH2F>("clusterTimeClusterE", clusterTimeClusterE) ;

  auto dt99_clusterE = new TH2F("dt99_clusterE",
                                ";Electron cluster energy [GeV];dt99 [ns]", N_E_BINS, energyBinEdges, nbins, 0, max_t) ;
  registerObject<TH2F>("dt99_clusterE", dt99_clusterE) ;


  auto eventT0 = new TH1F("eventT0", ";event t0 [ns]; number of events", nbins, min_t, max_t) ;
  registerObject<TH1F>("eventT0", eventT0) ;

  auto clusterTimeE0E1diff = new TH1F("clusterTimeE0E1diff",
                                      ";ECL cluster time of max E electron - ECL cluster time of 2nd max E electron [ns]; number of electron ECL cluster time differences",
                                      nbins, min_t, max_t) ;
  registerObject<TH1F>("clusterTimeE0E1diff", clusterTimeE0E1diff) ;


  //=== Required data objects
  tracks.isRequired() ;
  m_eclClusterArray.isRequired() ;
  m_eclCalDigitArray.isRequired() ;


  B2INFO("skipTrgSel = " << skipTrgSel);

}

void eclBhabhaTimeCalibrationValidationCollectorModule::collect()
{
  int cutIndexPassed = 0;
  getObjectPtr<TH1F>("cutflow")->Fill(cutIndexPassed);
  B2DEBUG(22, "Cutflow: no cuts: index = " << cutIndexPassed);


  // --- Check the trigger skim is the type that has two tracks

  /* If we skip the trigger skim selection then still fill the cutflow histogram
     just so that the positions don't change. */
  if (!skipTrgSel) {
    if (!m_TrgResult.isValid()) {
      B2WARNING("SoftwareTriggerResult required to select bhabha event is not found");
      return;
    }

    /* Release05: bhabha_all is grand skim = bhabha+bhabhaecl+radee.  We only want
       to look at the 2 track bhabha events. */
    const std::map<std::string, int>& fresults = m_TrgResult->getResults();
    if (fresults.find("software_trigger_cut&skim&accept_bhabha") == fresults.end()) {
      B2WARNING("Can't find required bhabha trigger identifier");
      return;
    }

    const bool eBhabha = (m_TrgResult->getResult("software_trigger_cut&skim&accept_bhabha") ==
                          SoftwareTriggerCutResult::c_accept);
    B2DEBUG(22, "eBhabha (trigger passed) = " << eBhabha);

    if (!eBhabha) {
      return;
    }
  }

  /*  Fill the histgram showing that the trigger skim cut passed OR that we
      are skipping this selection. */
  cutIndexPassed++;
  getObjectPtr<TH1F>("cutflow")->Fill(cutIndexPassed);
  B2DEBUG(22, "Cutflow: Trigger cut passed: index = " << cutIndexPassed);





  /* Use ECLChannelMapper to get other detector indices for the crystals
     For conversion from CellID to crate, shaper, and channel ids.
     The initialization function automatically checks to see if the
     object has been initialized and ifthe payload has changed and
     thus needs updating. */
  bool ECLchannelMapHasChanged = m_channelMapDB.hasChanged();
  if (ECLchannelMapHasChanged) {
    B2INFO("eclBhabhaTimeCalibrationValidationCollectorModule::collect() " << LogVar("ECLchannelMapHasChanged",
           ECLchannelMapHasChanged));
    if (!m_crystalMapper->initFromDB()) {
      B2FATAL("eclBhabhaTimeCalibrationValidationCollectorModule::collect() : Can't initialize eclChannelMapper!");
    }
  }





  B2DEBUG(29, "Finished checking if previous crystal time payload has changed");

  if (m_CrateTimeDB.hasChanged()) {
    m_CrateTime = m_CrateTimeDB->getCalibVector();
    m_CrateTimeUnc = m_CrateTimeDB->getCalibUncVector();
  }

  B2DEBUG(25, "eclBhabhaTimeCalibrationValidationCollector:: loaded ECLCrateTimeOffset from the database"
          << LogVar("IoV", m_CrateTimeDB.getIoV())
          << LogVar("Checksum", m_CrateTimeDB.getChecksum()));

  // Conversion coefficient from ADC ticks to nanoseconds
  // TICKS_TO_NS ~ 0.4931 ns/clock tick
  // 1/(4fRF) = 0.4913 ns/clock tick, where fRF is the accelerator RF frequency
  const double TICKS_TO_NS = 1.0 / (4.0 * EclConfiguration::getRF()) * 1e3;


  vector<float> Crate_time_ns(52, 0.0); /**< vector derived from DB object */
  vector<float> Crate_time_unc_ns(52, 0.0); /**< vector derived from DB object */

  // Make a crate time offset vector with an entry per crate (instead of per crystal) and convert from ADC counts to ns.
  for (int crysID = 1; crysID <= ECLElementNumbers::c_NCrystals; crysID++) {
    int crateID_temp = m_crystalMapper->getCrateID(crysID);
    Crate_time_ns[crateID_temp - 1] = m_CrateTime[crysID] * TICKS_TO_NS;
    Crate_time_unc_ns[crateID_temp - 1] = m_CrateTimeUnc[crysID] * TICKS_TO_NS;
  }



  // Storage crystal energies
  m_EperCrys.resize(ECLElementNumbers::c_NCrystals);
  for (auto& eclCalDigit : m_eclCalDigitArray) {
    int tempCrysID = eclCalDigit.getCellId() - 1;
    m_EperCrys[tempCrysID] = eclCalDigit.getEnergy();
  }


  // Getting the event t0 using the full event t0 rather than from the CDC specifically

  double evt_t0 = -1000 ;
  double evt_t0_unc = -1000 ;

  // Determine if there is an event t0 to use and then extract the information about it
  if (m_eventT0.isOptional()) {
    if (!m_eventT0.isValid()) {
      return;
    }
    if (!m_eventT0->hasEventT0()) {
      return;
    } else {
      // Overall event t0 (combination of multiple event t0s from different detectors)
      evt_t0 = m_eventT0->getEventT0() ;
      evt_t0_unc = m_eventT0->getEventT0Uncertainty() ;
    }
    B2DEBUG(26, "Found event t0") ;
  }


  //---------------------------------------------------------------------
  //..Some utilities
  PCmsLabTransform boostrotate;

  //---------------------------------------------------------------------
  //..Track properties. Use pion (211) mass hypothesis,
  //     which is the only particle hypothesis currently available???
  double maxp[2] = {0., 0.};
  int maxiTrk[2] = { -1, -1};
  int nTrkAll = tracks.getEntries() ;

  int nTrkLoose = 0 ; /**< number of loose tracks */
  int nTrkTight = 0 ; /**< number of tight tracks */


  /* Loop over all the tracks to define the tight and loose selection tracks
     We will select events with only 2 tight tracks and no additional loose tracks.
     Tight tracks are a subset of looses tracks. */
  for (int iTrk = 0 ; iTrk < nTrkAll ; iTrk++) {
    // Get track biasing towards the particle being a pion based on what particle types
    // are used for reconstruction at this stage.
    const TrackFitResult* tempTrackFit = tracks[iTrk]->getTrackFitResultWithClosestMass(Const::pion);
    if (not tempTrackFit) {continue ;}

    // Collect track info to be used for categorizing
    short charge = tempTrackFit->getChargeSign() ;
    double z0 = tempTrackFit->getZ0() ;
    double d0 = tempTrackFit->getD0() ;
    int nCDChits = tempTrackFit->getHitPatternCDC().getNHits() ;
    double p = tempTrackFit->getMomentum().R() ;

    /* Test if loose track  */

    // d0 and z0 cuts
    if (fabs(d0) > m_looseTrkD0) {
      continue;
    }
    if (fabs(z0) > m_looseTrkZ0) {
      continue;
    }
    // Number of hits in the CDC
    if (nCDChits < 1) {
      continue;
    }
    nTrkLoose++;



    /* Test if the loose track is also a tight track */

    // Number of hits in the CDC
    if (nCDChits < 20) {
      continue;
    }
    // d0 and z0 cuts
    if (fabs(d0) > m_tightTrkD0) {
      continue;
    }
    if (fabs(z0) > m_tightTrkZ0) {
      continue;
    }
    nTrkTight++;

    // Sorting of tight tracks.  Not really required as we only want two tight tracks (at the moment) but okay.
    //..Find the maximum p negative [0] and positive [1] tracks
    int icharge = 0;
    if (charge > 0) {icharge = 1;}
    if (p > maxp[icharge]) {
      maxp[icharge] = p;
      maxiTrk[icharge] = iTrk;
    }

  }
  /* After that last section the numbers of loose and tight tracks are known as well as the
     index of the loose tracks that have the highest p negatively charged and highest p positively
     charged tracks as measured in the centre of mass frame */


  if (nTrkTight != 2) {
    return;
  }
  // There are exactly two tight tracks
  cutIndexPassed++;
  getObjectPtr<TH1F>("cutflow")->Fill(cutIndexPassed);
  B2DEBUG(22, "Cutflow: Two tight tracks: index = " << cutIndexPassed);


  if (nTrkLoose != 2) {
    return;
  }
  // There are exactly two loose tracks as well, i.e. no additional loose tracks
  cutIndexPassed++ ;
  getObjectPtr<TH1F>("cutflow")->Fill(cutIndexPassed) ;
  B2DEBUG(22, "Cutflow: No additional loose tracks: index = " << cutIndexPassed) ;
  /* Determine if the two tracks have the opposite electric charge.
     We know this because the track indices stores the max pt track in [0] for negatively charged track
     and [1] fo the positively charged track.  If both are filled then both a negatively charged
     and positively charged track were found.   */
  bool oppositelyChargedTracksPassed = maxiTrk[0] != -1  &&  maxiTrk[1] != -1;
  if (!oppositelyChargedTracksPassed) {
    return;
  }
  // The two tracks have the opposite electric charges.
  cutIndexPassed++;
  getObjectPtr<TH1F>("cutflow")->Fill(cutIndexPassed);
  B2DEBUG(22, "Cutflow: Oppositely charged tracks: index = " << cutIndexPassed);



  //---------------------------------------------------------------------
  /* Determine associated energy clusters to each of the two tracks.  Sum the energies of the
     multiple clusters to each track and find the crystal with the maximum energy within all
     the sets of clusters associated to the tracks.  Extract the good cluster times.*/
  double trkEClustLab[2] = {0., 0.};
  double trkEClustCOM[2] = {0., 0.};
  double trkpLab[2];
  double trkpCOM[2];
  ROOT::Math::PxPyPzEVector trkp4Lab[2];
  ROOT::Math::PxPyPzEVector trkp4COM[2];

  // Index of the cluster and the crystal that has the highest energy crystal for the two tracks
  int numClustersPerTrack[2] = { 0, 0 };
  double E_DIV_p[2];

  vector<double> goodClustTimes ;
  vector<double> goodClust_dt99 ;
  vector<double> goodClustE ;
  vector<int> goodClustMaxEcrys_cid ;

  for (int icharge = 0; icharge < 2; icharge++) {
    if (maxiTrk[icharge] > -1) {
      B2DEBUG(22, "looping over the 2 max pt tracks");

      const TrackFitResult* tempTrackFit = tracks[maxiTrk[icharge]]->getTrackFitResultWithClosestMass(Const::pion);
      if (not tempTrackFit) {continue ;}

      trkp4Lab[icharge] = tempTrackFit->get4Momentum();
      trkp4COM[icharge] = boostrotate.rotateLabToCms() * trkp4Lab[icharge];
      trkpLab[icharge] = trkp4Lab[icharge].P();
      trkpCOM[icharge] = trkp4COM[icharge].P();


      /* For each cluster associated to the current track, sum up the energies to get the total
         energy of all clusters associated to the track and find which crystal has the highest
         energy from all those clusters*/
      auto eclClusterRelationsFromTracks = tracks[maxiTrk[icharge]]->getRelationsTo<ECLCluster>();
      for (unsigned int clusterIdx = 0; clusterIdx < eclClusterRelationsFromTracks.size(); clusterIdx++) {

        B2DEBUG(22, "Looking at clusters.  index = " << clusterIdx);
        auto cluster = eclClusterRelationsFromTracks[clusterIdx];

        if (cluster->hasHypothesis(Belle2::ECLCluster::EHypothesisBit::c_nPhotons)) {
          numClustersPerTrack[icharge]++;
          double eClust = cluster->getEnergy(Belle2::ECLCluster::EHypothesisBit::c_nPhotons);
          double electronTime = cluster->getTime();
          bool badElectronTime = cluster->hasFailedFitTime();
          bool badElectronTimeResolution = cluster->hasFailedTimeResolution();
          if ((fabs(electronTime) < m_timeAbsMax)   &&
              (!badElectronTime)                    &&
              (!badElectronTimeResolution)) {
            trkEClustLab[icharge] += eClust ;
            short cid = cluster->getMaxECellId() ;
            goodClustMaxEcrys_cid.push_back(cid) ;
            goodClustTimes.push_back(electronTime) ;
            goodClust_dt99.push_back(cluster->getDeltaTime99()) ;
            goodClustE.push_back(eClust);
          }
        }
      }
      trkEClustCOM[icharge] = trkEClustLab[icharge] * trkpCOM[icharge] / trkpLab[icharge];

      // Check both electrons to see if their cluster energy / track momentum is good.
      // The Belle II physics book shows that this is the main way of separating electrons from other particles
      // Done in the centre of mass reference frame although I believe E/p is invariant under a boost.
      E_DIV_p[icharge] = trkEClustCOM[icharge] / trkpCOM[icharge];

    }
  }
  /* At the end of this section the 3-momenta magnitudes and the cluster energies are known
     for the two saved track indices for both the lab and COM frames.
     The crystal with the maximum energy, one associated to each track, is recorded*/
  B2DEBUG(26, "Extracted time information and E/p for the tracks") ;



  /* Cut on the number of ECL cluster connected to tracks

     THIS IS DIFFERENT FROM THE CODE THAT PERFORMS THE CALIBRATIONS.  THIS VALIDATIONS REQUIRES
     THAT THERE ARE EXACTLY TWO CLUSTERS ASSOCIATED TO THE TRACKS WHILE THE CALIBRATION
     CODE ALLOWS FOR MORE THAN ONE CLUSTER PER TRACK.  THIS VALIDATION ALSO DOES NOT CUT ON THE
     NUMBER OF EXTRA CLUSTERS NOT ASSOCIATED TO THE 2 TRACKS, WHICH IS A LOOSER CUT THAN USED
     TO PERFORM THE CALIBRATION. */
  long unsigned int numGoodElectronClusters_cut = 2 ;
  if (goodClustTimes.size() != numGoodElectronClusters_cut) {
    return ;
  }
  // There is exactly two ECL clusters connected to tracks in the event
  cutIndexPassed++ ;
  getObjectPtr<TH1F>("cutflow")->Fill(cutIndexPassed) ;
  B2DEBUG(22, "Cutflow: Exactly " << numGoodElectronClusters_cut
          << " good clusters connected to tracks: index = " << cutIndexPassed);


  // Check both electrons to see if their cluster energy / track momentum is good.
  // The Belle II physics book shows that this is the main way of separating electrons from other particles
  // Done in the centre of mass reference frame although I believe E/p is invariant under a boost.
  bool E_DIV_p_instance_passed[2] = {false, false};
  double E_DIV_p_CUT = 0.7;
  for (int icharge = 0; icharge < 2; icharge++) {
    E_DIV_p_instance_passed[icharge] = E_DIV_p[icharge] > E_DIV_p_CUT;
  }
  if (!E_DIV_p_instance_passed[0] || !E_DIV_p_instance_passed[1]) {
    return;
  }
  // E/p sufficiently large
  cutIndexPassed++;
  getObjectPtr<TH1F>("cutflow")->Fill(cutIndexPassed);
  B2DEBUG(22, "Cutflow: E_i/p_i > " << E_DIV_p_CUT  << ": index = " << cutIndexPassed);



  // Cut on the invariant mass of the tracks in the event
  double invMassTrk = (trkp4Lab[0] + trkp4Lab[1]).M();
  double invMass_CUT = 0.9;

  bool invMassCutsPassed = invMassTrk > (invMass_CUT * boostrotate.getCMSEnergy());
  if (!invMassCutsPassed) {
    return;
  }
  // Invariable mass of the two tracks are above the minimum
  cutIndexPassed++;
  getObjectPtr<TH1F>("cutflow")->Fill(cutIndexPassed);
  B2DEBUG(22, "Cutflow: m(track 1+2) > " << invMass_CUT << "*E_COM = " << invMass_CUT << " * " << boostrotate.getCMSEnergy() <<
          " : index = " << cutIndexPassed);


  B2DEBUG(22, "Event passed all cuts");


  // Fill the histogram for the event level variables
  getObjectPtr<TH1F>("eventT0")->Fill(evt_t0) ;

  bool isCDCt0 = m_eventT0->isCDCEventT0();
  bool isECLt0 = m_eventT0->isECLEventT0();
  string t0Detector = "UNKNOWN... WHY?";
  if (isCDCt0) {
    t0Detector = "CDC" ;
  } else if (isECLt0) {
    t0Detector = "ECL" ;
  }

  B2DEBUG(26, "t0 = " << evt_t0 << " ns.  t0 is from CDC?=" << isCDCt0 << ", t0 is from ECL?=" << isECLt0 << " t0 from " <<
          t0Detector);


  //=== For each good electron cluster in the processed event and fill histogram.
  for (long unsigned int i = 0 ; i < goodClustTimes.size() ; i++) {
    getObjectPtr<TH1F>("clusterTime")->Fill(goodClustTimes[i]) ;
    getObjectPtr<TH2F>("clusterTime_cid")->Fill(goodClustMaxEcrys_cid[i] + 0.001, goodClustTimes[i], 1) ;
    getObjectPtr<TH2F>("clusterTime_run")->Fill(m_EventMetaData->getRun() + 0.001, goodClustTimes[i], 1) ;
    getObjectPtr<TH2F>("clusterTimeClusterE")->Fill(goodClustE[i], goodClustTimes[i], 1) ;
    getObjectPtr<TH2F>("dt99_clusterE")->Fill(goodClustE[i], goodClust_dt99[i], 1) ;


    //== Save debug TTree with detailed information if necessary.
    if (m_saveTree) {

      m_tree_time      = goodClustTimes[i] ;
      m_tree_t0        = evt_t0 ;
      m_tree_t0_unc    = evt_t0_unc ;
      m_E_electron_clust = goodClustE[i] ;
      m_NtightTracks   = nTrkTight ;
      m_tree_evt_num   = m_EventMetaData->getEvent() ;
      m_tree_run       = m_EventMetaData->getRun() ;
      m_tree_cid       = goodClustMaxEcrys_cid[i] ;
      m_tree_dt99      = goodClust_dt99[i] ;

      m_dbg_tree_electronClusters->Fill() ;

    }
  }
  B2DEBUG(26, "Filled cluster tree") ;

  //=== Fill histogram for cluster time difference of the two electrons
  double tDiff;
  if (goodClustE[0] > goodClustE[1]) {
    tDiff = goodClustTimes[0] - goodClustTimes[1];
  } else {
    tDiff = goodClustTimes[1] - goodClustTimes[0];
  }

  getObjectPtr<TH1F>("clusterTimeE0E1diff")->Fill(tDiff) ;



  if (m_saveTree) {
    m_tree_t0          = evt_t0 ;
    m_tree_t0_unc      = evt_t0_unc ;
    m_tree_evt_num     = m_EventMetaData->getEvent() ;
    m_tree_run         = m_EventMetaData->getRun() ;
    m_NtightTracks     = nTrkTight ;
    m_tree_E0          = goodClustE[0] ;
    m_tree_E1          = goodClustE[1] ;
    m_tree_time_fromE0 = goodClustTimes[0] ;
    m_tree_time_fromE1 = goodClustTimes[1] ;

    m_dbg_tree_event->Fill() ;


    int runNum = m_EventMetaData->getRun();
    if (m_tree_PreviousRun != runNum) {
      for (int icrate = 1; icrate <= 52; icrate++) {
        m_tree_run        = runNum ;
        m_tree_crateid    = icrate ;
        m_tree_tcrate     = Crate_time_ns[icrate] ;
        m_tree_tcrate_unc = Crate_time_unc_ns[icrate] ;

        m_dbg_tree_run->Fill() ;
      }
      m_tree_PreviousRun = m_tree_run ;
    }
  }

  B2DEBUG(26, "Filled event tree") ;

}
