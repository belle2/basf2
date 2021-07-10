/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <ecl/modules/eclHadronTimeCalibrationValidationCollector/eclHadronTimeCalibrationValidationCollectorModule.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/gearbox/Const.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLTrig.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <ecl/digitization/EclConfiguration.h>

#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/ClusterUtility/ClusterUtils.h>
#include <boost/optional.hpp>

#include <TH2F.h>
#include <TTree.h>
#include <TFile.h>

using namespace Belle2;
using namespace ECL;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(eclHadronTimeCalibrationValidationCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

eclHadronTimeCalibrationValidationCollectorModule::eclHadronTimeCalibrationValidationCollectorModule() :
  CalibrationCollectorModule(),
  m_dbg_tree_photonClusters(0),
  m_dbg_tree_event(0),
  m_tree_evt_num(0)//,
  //m_GammaGammaECalib("ECLCrystalEnergyGammaGamma")
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


  // specify this flag if you need parallel processing
  setPropertyFlags(c_ParallelProcessingCertified);
}

eclHadronTimeCalibrationValidationCollectorModule::~eclHadronTimeCalibrationValidationCollectorModule()
{
}

void eclHadronTimeCalibrationValidationCollectorModule::inDefineHisto()
{

  //=== Prepare TTree for debug output
  if (m_saveTree) {
    // Per photon cluster
    m_dbg_tree_photonClusters = new TTree("tree_photonClusters",
                                          "Validating crystal and crate time calibrations using photon clusters in events with lots of tracks and clusters") ;
    m_dbg_tree_photonClusters->Branch("EventNum"  , &m_tree_evt_num)   ->SetTitle("Event number") ;
    m_dbg_tree_photonClusters->Branch("cluster_time"    , &m_tree_time)   ->SetTitle("Cluster time t (calibrated), ns") ;
    m_dbg_tree_photonClusters->Branch("clust_E"    , &m_E_photon_clust)   ->SetTitle("Photon type cluster energy, GeV") ;
    m_dbg_tree_photonClusters->Branch("Ntracks"  , &m_NtightTracks)     ->SetTitle("Number of tracks") ;
    m_dbg_tree_photonClusters->Branch("NphotonClusters"  , &m_NphotonClusters)     ->SetTitle("Number of photons") ;
    m_dbg_tree_photonClusters->Branch("NGoodClusters"  , &m_NGoodClusters)     ->SetTitle("Number of good ECL clusters") ;
    m_dbg_tree_photonClusters->Branch("t0"      , &m_tree_t0)     ->SetTitle("T0, ns") ;
    m_dbg_tree_photonClusters->Branch("t0_unc"  , &m_tree_t0_unc)     ->SetTitle("T0 uncertainty, ns") ;
    m_dbg_tree_photonClusters->Branch("runNum"  , &m_tree_run)     ->SetTitle("Run number") ;
    m_dbg_tree_photonClusters->Branch("CrystalCellID"     , &m_tree_cid)    ->SetTitle("Cell ID, 1..8736") ;
    m_dbg_tree_photonClusters->Branch("dt99"    , &m_tree_dt99)   ->SetTitle("Cluster dt99, ns") ;
    m_dbg_tree_photonClusters->SetAutoSave(10) ;

    // Per event
    m_dbg_tree_event = new TTree("tree_event",
                                 "Validating crystal and crate time calibrations using photon clusters in events with lots of tracks and clusters") ;
    m_dbg_tree_event->Branch("EventNum"  , &m_tree_evt_num)   ->SetTitle("Event number") ;
    m_dbg_tree_event->Branch("t0"      , &m_tree_t0)     ->SetTitle("T0, ns") ;
    m_dbg_tree_event->Branch("t0_unc"  , &m_tree_t0_unc)     ->SetTitle("T0 uncertainty, ns") ;
    m_dbg_tree_event->Branch("runNum"  , &m_tree_run)     ->SetTitle("Run number") ;
    m_dbg_tree_event->Branch("Ntracks"  , &m_NtightTracks)     ->SetTitle("Number of tracks") ;
    m_dbg_tree_event->Branch("NphotonClusters"  , &m_NphotonClusters)     ->SetTitle("Number of photons") ;
    m_dbg_tree_event->Branch("NGoodClusters"  , &m_NGoodClusters)     ->SetTitle("Number of good ECL clusters") ;
    m_dbg_tree_event->Branch("E0"  , &m_tree_E0)   ->SetTitle("Highest E cluster E") ;
    m_dbg_tree_event->Branch("time_E0"  , &m_tree_time_fromE0)   ->SetTitle("Cluster time of highest E cluster") ;
    m_dbg_tree_event->SetAutoSave(10) ;
  }
}

void eclHadronTimeCalibrationValidationCollectorModule::prepare()
{
  //=== MetaData
  StoreObjPtr<EventMetaData> evtMetaData ;
  B2INFO("eclHadronTimeCalibrationValidationCollector: Experiment = " << evtMetaData->getExperiment() <<
         "  run = " << evtMetaData->getRun()) ;

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

  auto clusterTime = new TH1F("clusterTime", ";Photon ECL cluster time [ns]; number of photon ECL clusters", nbins, min_t, max_t) ;
  registerObject<TH1F>("clusterTime", clusterTime) ;

  auto clusterTime_cid = new TH2F("clusterTime_cid",
                                  ";crystal Cell ID ;Photon ECL cluster time [ns]", 8736, 1, 8736 + 1, nbins, min_t, max_t) ;
  registerObject<TH2F>("clusterTime_cid", clusterTime_cid) ;

  auto clusterTime_run = new TH2F("clusterTime_run",
                                  ";Run number ;Photon ECL cluster time [ns]", 7000, 0, 7000, nbins, min_t, max_t) ;
  registerObject<TH2F>("clusterTime_run", clusterTime_run) ;


  auto clusterTimeClusterE = new TH2F("clusterTimeClusterE",
                                      ";Photon cluster energy [GeV];Photon cluster time [ns]", N_E_BINS, energyBinEdges, nbins, min_t, max_t) ;
  registerObject<TH2F>("clusterTimeClusterE", clusterTimeClusterE) ;


  auto dt99_clusterE = new TH2F("dt99_clusterE",
                                ";Photon cluster energy [GeV];dt99 [ns]", N_E_BINS, energyBinEdges, nbins, 0, max_t) ;
  registerObject<TH2F>("dt99_clusterE", dt99_clusterE) ;


  auto eventT0 = new TH1F("eventT0", ";event t0 [ns]; number of events", nbins, min_t, max_t) ;
  registerObject<TH1F>("eventT0", eventT0) ;


  auto clusterTimeE0E1diff = new TH1F("clusterTimeE0E1diff",
                                      ";ECL cluster time of max E photon - ECL cluster time of 2nd max E photon [ns]; number of photon ECL cluster time differences",
                                      nbins, min_t, max_t) ;
  registerObject<TH1F>("clusterTimeE0E1diff", clusterTimeE0E1diff) ;


  //=== Required data objects
  tracks.isRequired() ;
  m_eclClusterArray.isRequired() ;
  m_eclCalDigitArray.isRequired() ;

}

void eclHadronTimeCalibrationValidationCollectorModule::collect()
{
  int cutIndexPassed = 0;
  getObjectPtr<TH1F>("cutflow")->Fill(cutIndexPassed);
  B2DEBUG(22, "Cutflow: no cuts: index = " << cutIndexPassed);


  /* Use ECLChannelMapper to get other detector indices for the crystals */
  /* For conversion from CellID to crate, shaper, and channel ids. */

  // Use smart pointer to avoid memory leak when the ECLChannelMapper object needs destroying at the end of the event.
  shared_ptr< ECL::ECLChannelMapper > crystalMapper(new ECL::ECLChannelMapper());
  crystalMapper->initFromDB();


  // Storage crystal energies
  m_EperCrys.resize(8736);
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
  //..Track properties. Use pion (211) mass hypothesis,
  //     which is the only particle hypothesis currently available???
  int nTrkAll = tracks.getEntries() ;

  int nTrkLoose = 0 ; /**< number of loose tracks */
  int nTrkTight = 0 ; /**< number of tight tracks */


  /* Loop over all the tracks to define the tight and loose selection tracks.
     We will select events with only a few tight tracks and no additional loose tracks.
     Tight tracks are a subset of looses tracks. */
  for (int iTrk = 0 ; iTrk < nTrkAll ; iTrk++) {

    // Get track biasing towards the particle being a pion
    const TrackFitResult* tempTrackFit = tracks[iTrk]->getTrackFitResultWithClosestMass(Const::pion) ;
    if (not tempTrackFit) {continue ;}

    // Collect track info to be used for categorizing
    //short charge = tempTrackFit->getChargeSign() ;
    double z0 = tempTrackFit->getZ0() ;
    double d0 = tempTrackFit->getD0() ;
    int nCDChits = tempTrackFit->getHitPatternCDC().getNHits() ;
    //double pt = tempTrackFit->getTransverseMomentum() ;
    //double p = tempTrackFit->getMomentum().Mag() ;

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

  }
  // After that last section the numbers of loose and tight tracks are known
  B2DEBUG(26, "Found loose and tight tracks") ;


  int numGoodTightTracks_minCut = 4 ;
  if (nTrkTight < numGoodTightTracks_minCut) {
    return ;
  }
  // There are at least X tight tracks
  cutIndexPassed++ ;
  getObjectPtr<TH1F>("cutflow")->Fill(cutIndexPassed) ;
  B2DEBUG(22, "Cutflow: At least " << numGoodTightTracks_minCut << " tight tracks: index = " << cutIndexPassed) ;


  int numGoodLooseTracks_minCut = numGoodTightTracks_minCut ;
  if (nTrkLoose < numGoodLooseTracks_minCut) {
    return ;
  }
  // There are more loose tracks than tight tracks then veto the event.  If there are fewer loose tracks than tight tracks then veto the event, although this should be impossible
  cutIndexPassed++ ;
  getObjectPtr<TH1F>("cutflow")->Fill(cutIndexPassed) ;
  B2DEBUG(22, "Cutflow: No additional loose tracks: index = " << cutIndexPassed) ;


  //------------------------------------------------------------------------
  // Find the good ECL clusters
  double clusterE_minCut = 0.1 ;  // GeV
  int nclust = m_eclClusterArray.getEntries();
  int nGoodClusts = 0 ;
  vector<int> goodClusterIdxs ;
  for (int ic = 0; ic < nclust; ic++) {
    double eClust = m_eclClusterArray[ic]->getEnergy(Belle2::ECLCluster::EHypothesisBit::c_nPhotons) ;
    if (eClust > clusterE_minCut) {
      goodClusterIdxs.push_back(ic) ;
      nGoodClusts++ ;
    }
  }


  // Cut on the minimum number of good clusters
  int numGoodEMclusters_minCut = 5 ;
  if (nGoodClusts < numGoodEMclusters_minCut) {
    return ;
  }
  // There are at least 5 good EM clusters (photon = basically all clusters)
  cutIndexPassed++ ;
  getObjectPtr<TH1F>("cutflow")->Fill(cutIndexPassed) ;
  B2DEBUG(22, "Cutflow: At least " << numGoodEMclusters_minCut << " ECL clusters: index = " << cutIndexPassed) ;


  //------------------------------------------------------------------------
  // Find the good photons first before doing anything with them

  double photonE_minCut = 0.05 ;  // GeV
  double zernikeMVA_minCut = 0.2 ;
  int nPhotons = 0 ;

  vector<int> goodPhotonClusterIdxs ;
  for (int ic = 0; ic < nclust; ic++) {
    if (m_eclClusterArray[ic]->hasHypothesis(Belle2::ECLCluster::EHypothesisBit::c_nPhotons)) {
      double eClust = m_eclClusterArray[ic]->getEnergy(Belle2::ECLCluster::EHypothesisBit::c_nPhotons);
      double photonTime = m_eclClusterArray[ic]->getTime();
      double zernikeMVA = m_eclClusterArray[ic]->getZernikeMVA();
      bool badPhotonTime = m_eclClusterArray[ic]->hasFailedFitTime();
      bool badPhotonTimeResolution = m_eclClusterArray[ic]->hasFailedTimeResolution();
      bool hasTrack = m_eclClusterArray[ic]->isTrack();
      if ((eClust > photonE_minCut)           &&
          (fabs(photonTime) < m_timeAbsMax)   &&
          (!badPhotonTime)                    &&
          (!badPhotonTimeResolution)          &&
          (zernikeMVA > zernikeMVA_minCut)    &&
          (!hasTrack)) {
        goodPhotonClusterIdxs.push_back(ic) ;
        nPhotons++;
      }
    }
  }


  // Cut on the minimum number of good photon clusters
  int numGoodPhotonclusters_minCut = 1 ;
  if (nPhotons < numGoodPhotonclusters_minCut) {
    return ;
  }
  // There is at least one good photon in the event
  cutIndexPassed++ ;
  getObjectPtr<TH1F>("cutflow")->Fill(cutIndexPassed) ;
  B2DEBUG(22, "Cutflow: At least " << numGoodPhotonclusters_minCut << " good photon: index = " << cutIndexPassed) ;



  //------------------------------------------------------------------------
  /* Extract the times of the good clusters and
     save the maximum energy crystal information (cid) */
  vector<double> goodClustTimes ;
  vector<double> goodClust_dt99 ;
  vector<double> goodClusters_crysE ;
  vector<double> goodClustE ;
  vector<int> goodClustMaxEcrys_cid ;
  for (long unsigned int i = 0; i < goodPhotonClusterIdxs.size(); i++) {
    int ic = goodPhotonClusterIdxs[i] ;

    if (m_eclClusterArray[ic]->hasHypothesis(Belle2::ECLCluster::EHypothesisBit::c_nPhotons)) {
      double eClust = m_eclClusterArray[ic]->getEnergy(Belle2::ECLCluster::EHypothesisBit::c_nPhotons);
      short cid = m_eclClusterArray[ic]->getMaxECellId() ;

      goodClustMaxEcrys_cid.push_back(cid) ;
      goodClustTimes.push_back(m_eclClusterArray[ic]->getTime()) ;
      goodClust_dt99.push_back(m_eclClusterArray[ic]->getDeltaTime99()) ;
      goodClusters_crysE.push_back(m_EperCrys[cid - 1]) ;
      goodClustE.push_back(eClust);
    }
  }


  // Define a pair (energy,time) so that we can quickly and easily sort the cluster information
  //    based on the energy of the clusters
  vector< pair<double, double> > pair_energy_time ;
  for (long unsigned int ic = 0; ic < goodClusters_crysE.size(); ic++) {
    pair_energy_time.push_back(make_pair(goodClusters_crysE[ic], goodClustTimes[ic])) ;
  }

  // sorts pairs in decreasing order of their first value (energy)
  // i.e. highest energy first
  sort(pair_energy_time.begin(), pair_energy_time.end(), greater<>()) ;



  B2DEBUG(22, "Event passed all cuts");


  // Fill the histogram for the event level variables
  getObjectPtr<TH1F>("eventT0")->Fill(evt_t0) ;

  bool isCDCt0 = (static_cast<EventT0::EventT0Component>(*m_eventT0->getEventT0Component())).detectorSet.contains(Const::CDC);
  bool isECLt0 = (static_cast<EventT0::EventT0Component>(*m_eventT0->getEventT0Component())).detectorSet.contains(Const::ECL);
  string t0Detector = "UNKNOWN... WHY?";
  if (isCDCt0) {
    t0Detector = "CDC" ;
  } else if (isECLt0) {
    t0Detector = "ECL" ;
  }

  B2DEBUG(26, "t0 = " << evt_t0 << " ns.  t0 is from CDC?=" << isCDCt0 << ", t0 is from ECL?=" << isECLt0 << " t0 from " <<
          t0Detector);



  //=== For each good photon cluster in the processed event and fill histogram.

  StoreObjPtr<EventMetaData> evtMetaData ;
  for (long unsigned int i = 0 ; i < goodPhotonClusterIdxs.size() ; i++) {
    getObjectPtr<TH1F>("clusterTime")->Fill(goodClustTimes[i]) ;
    getObjectPtr<TH2F>("clusterTime_cid")->Fill(goodClustMaxEcrys_cid[i] + 0.001, goodClustTimes[i] , 1) ;
    getObjectPtr<TH2F>("clusterTime_run")->Fill(evtMetaData->getRun() + 0.001, goodClustTimes[i] , 1) ;
    getObjectPtr<TH2F>("clusterTimeClusterE")->Fill(goodClustE[i], goodClustTimes[i], 1) ;
    getObjectPtr<TH2F>("dt99_clusterE")->Fill(goodClustE[i], goodClust_dt99[i], 1) ;

    //== Save debug TTree with detailed information if necessary.
    if (m_saveTree) {

      m_tree_time      = goodClustTimes[i] ;
      m_E_photon_clust = goodClusters_crysE[i] ;
      m_tree_t0        = evt_t0 ;
      m_tree_t0_unc    = evt_t0_unc ;
      m_NtightTracks    = nTrkTight ;
      m_NphotonClusters = nPhotons ;
      m_NGoodClusters   = nGoodClusts ;
      m_tree_evt_num   = evtMetaData->getEvent() ;
      m_tree_run         = evtMetaData->getRun() ;
      m_tree_cid       = goodClustMaxEcrys_cid[i] ;
      m_tree_dt99      = goodClust_dt99[i] ;

      m_dbg_tree_photonClusters->Fill() ;

    }
  }
  B2DEBUG(26, "Filled cluster tree") ;

  //=== Fill histogram for cluster time difference of the two max E photons
  if (pair_energy_time.size() >= 2) {
    getObjectPtr<TH1F>("clusterTimeE0E1diff")->Fill(pair_energy_time[0].second - pair_energy_time[1].second) ;
  }



  if (m_saveTree) {
    m_tree_t0         = evt_t0 ;
    m_tree_t0_unc     = evt_t0_unc ;
    m_tree_evt_num    = evtMetaData->getEvent() ;
    m_tree_run          = evtMetaData->getRun() ;
    m_NtightTracks    = nTrkTight ;
    m_NphotonClusters = nPhotons ;
    m_NGoodClusters   = nGoodClusts ;

    m_tree_E0 = pair_energy_time[0].first ;
    m_tree_time_fromE0 = pair_energy_time[0].second ;
    m_dbg_tree_event->Fill() ;
  }

  B2DEBUG(26, "Filled event tree") ;

}
