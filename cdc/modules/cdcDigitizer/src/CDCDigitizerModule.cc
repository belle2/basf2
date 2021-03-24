/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck, CDC group                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/modules/cdcDigitizer/CDCDigitizerModule.h>
#include <cdc/utilities/ClosestApproach.h>

#include <framework/datastore/RelationArray.h>
//#include <framework/datastore/RelationIndex.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <TRandom.h>
#include <map>
#include <utility>

using namespace std;
using namespace Belle2;
using namespace CDC;


// register module
REG_MODULE(CDCDigitizer)
CDCDigitizerModule::CDCDigitizerModule() : Module(),
  m_cdcgp(), m_gcp(), m_aCDCSimHit(), m_posFlag(0),
  m_driftLength(0.0), m_flightTime(0.0), m_globalTime(0.0),
  m_tdcBinWidth(1.0), m_tdcBinWidthInv(1.0),
  m_tdcResol(0.9825), m_driftV(4.0e-3),
  m_driftVInv(250.0), m_propSpeedInv(27.25), m_align(true)
{
  // Set description
  setDescription("Creates CDCHits from CDCSimHits.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Add parameters
  // I/O
  addParam("InputCDCSimHitsName",         m_inputCDCSimHitsName, "Name of input array. Should consist of CDCSimHits.", string(""));
  addParam("OutputCDCHitsName",           m_outputCDCHitsName,   "Name of output array. Will consist of CDCHits.",     string(""));
  addParam("OutputCDCHitsName4Trg",       m_outputCDCHitsName4Trg,
           "Name of output array for trigger. Can contain several hits per wire, "
           "if they correspond to different time windows of 32ns.",
           string("CDCHits4Trg"));

  //Relations
  addParam("MCParticlesToCDCSimHitsName",  m_MCParticlesToSimHitsName,
           "Name of relation between MCParticles and CDCSimHits used",     string(""));
  addParam("CDCSimHistToCDCHitsName",      m_SimHitsTOCDCHitsName,
           "Name of relation between the CDCSimHits and the CDCHits used", string(""));


  //Parameters for Digitization
  addParam("UseSimpleDigitization",       m_useSimpleDigitization,
           "If true, a simple x-t with a constant velocity is used for the drift-length to -time conversion", false);

  //float Gauss Parameters
  addParam("Fraction",                    m_fraction,    "Fraction of first Gaussian used to smear drift length in cm",    1.0);
  addParam("Mean1",                       m_mean1,       "Mean value of first Gaussian used to smear drift length in cm",  0.0000);
  addParam("Resolution1",                 m_resolution1, "Resolution of first Gaussian used to smear drift length in cm",  0.0130);
  addParam("Mean2",                       m_mean2,       "Mean value of second Gaussian used to smear drift length in cm", 0.0000);
  addParam("Resolution2",                 m_resolution2, "Resolution of second Gaussian used to smear drift length in cm", 0.0000);

  //Switch to control smearing
  addParam("DoSmearing", m_doSmearing,
           "If false, drift length will not be smeared.", true);

  addParam("TrigTimeJitter", m_trigTimeJitter,
           "Magnitude (w) of trigger timing jitter (ns). The trigger timing is randuminzed uniformly in a time window of [-w/2, +w/2].",
           0.);
  //Switches to control time information handling
  addParam("AddTimeWalk", m_addTimeWalk, "A switch for time-walk (pulse-heght dep. delay); true: on; false: off", true);
  addParam("AddInWirePropagationDelay",   m_addInWirePropagationDelay,
           "A switch used to control adding propagation delay in the wire into the final drift time or not; this is for signal hits.", true);
  addParam("AddInWirePropagationDelay4Bg",  m_addInWirePropagationDelay4Bg,
           "The same switch but for beam bg. hits.", true);
  addParam("AddTimeOfFlight",  m_addTimeOfFlight,
           "A switch used to control adding time of flight into the final drift time or not; this is for signal hits.", true);
  addParam("AddTimeOfFlight4Bg",   m_addTimeOfFlight4Bg,
           "The same switch but for beam bg. hits.", true);
  addParam("OutputNegativeDriftTime", m_outputNegativeDriftTime, "Output hits with negative drift time", true);
  addParam("Output2ndHit", m_output2ndHit,
           "Output the 2nd hit if exists in the time window. Note that it is not well-simulated at all, partly because no cross-talk betw. channels is simulated.",
           false);
  //Switch to control sense wire sag
  addParam("CorrectForWireSag",   m_correctForWireSag,
           "A switch for sense wire sag effect; true: drift-time is calculated with the sag taken into account; false: not. Here, sag means the perturbative part which corresponds to alignment in case of wire-position. The main part (corresponding to design+displacement in wire-position) is taken into account in FullSim; you can control it via CDCJobCntlParModifier.",
           true);

  //Threshold
  addParam("TDCThreshold4Outer", m_tdcThreshold4Outer,
           "TDC threshold (dE in eV) for Layers#8-56. The value corresponds to He-C2H6 gas; for the gas+wire (MaterialDefinitionMode=0) case, (this value)*f will be used, where f is specified by GasToGasWire",
           25.0);
  addParam("TDCThreshold4Inner", m_tdcThreshold4Inner,
           "Same as TDCThreshold4Outer but for Layers#0-7,", 25.0);
  addParam("GasToGasWire", m_gasToGasWire,
           "(Approximate) ratio of dE in He/C2H6-gas to dE in gas+wire, where dE is energy deposit.", 1. / 1.478);

  //ADC Threshold
  addParam("ADCThreshold", m_adcThreshold,
           "Threshold for ADC-count (in unit of count). ADC-count < threshold is treated as count=0.", 2);
  addParam("tMin", m_tMin, "Lower edge of time window in ns; valid only for UseDB4FEE=false", -100.);
  addParam("tMaxOuter", m_tMaxOuter, "Upper edge of time window in ns for the normal-cell layers; valid only for UseDB4FEE=false",
           500.);
  addParam("tMaxInner", m_tMaxInner, "Upper edge of time window in ns for the small-cell layers; valid only for UseDB4FEE=false",
           300.);
  // The following doesn't make any sense. The only reasonable steerable would be a switch to decide if the jitter shall be
  // activated. Then there has to be event by event jitter.
  /*  addParam("EventTime",                   m_eventTime,
             "It is a timing of event, which includes a time jitter due to the trigger system, set in ns",     float(0.0));*/

  //Switch for database
  addParam("UseDB4FEE", m_useDB4FEE, "Fetch and use FEE params. from database or not", true);
  addParam("UseDB4EDepToADC", m_useDB4EDepToADC, "Uuse edep-to-ADC conversion params. from database or not", true);
  addParam("UseDB4RunGain", m_useDB4RunGain, "Fetch and use run gain from database or not", true);
  addParam("OverallGainFactor", m_overallGainFactor, "Overall gain factor for adjustment", 1.0);

  //Switch for synchronization
  addParam("Synchronization", m_synchronization, "Synchronize timing with other sub-detectors", m_synchronization);
  addParam("Randomization", m_randomization, "Randomize timing with other sub-detectors; valid only for Synchronization=false",
           m_randomization);
  addParam("OffsetForGetTriggerBin", m_offsetForTriggerBin, "Input to getCDCTriggerBin(offset), either of 0,1,2 or 3",
           m_offsetForTriggerBin);
  addParam("TrgTimingOffsetInCount", m_trgTimingOffsetInCount, "Trigger timing offset in count, [0,7]", m_trgTimingOffsetInCount);
  addParam("ShiftOfTimeWindowIn32Count", m_shiftOfTimeWindowIn32Count,
           "Shift of time window in 32count for synchronization (L1 timing=0)", m_shiftOfTimeWindowIn32Count);

  //Some FEE params.
  addParam("TDCThresholdOffset", m_tdcThresholdOffset, "Offset for TDC (digital) threshold (mV)", 3820.);
  addParam("AnalogGain",   m_analogGain, "Analog  gain (V/pC)", 1.1);
  addParam("DigitalGain", m_digitalGain, "Digital gain (V/pC)", 15.);
  addParam("ADCBinWidth", m_adcBinWidth, "ADC bin width  (mV)",  2.);

  addParam("AddFudgeFactorForSigma", m_addFudgeFactorForSigma,
           "Additional fudge factor for space resol. (common to all cells)",  1.);
  addParam("SpaceChargeEffect", m_spaceChargeEffect, "Switch for space charge effect", true);

  addParam("AddXTalk", m_addXTalk, "A switch for crosstalk; true: on; false: off", true);
  addParam("Issue2ndHitWarning", m_issue2ndHitWarning, "=true: issue a warning when a 2nd TDC hit is found.", true);
  addParam("IncludeEarlyXTalks", m_includeEarlyXTalks, "=true: include earlier x-talks as well than the signal hit in question.",
           true);
  addParam("DebugLevel", m_debugLevel, "Debug level; 20-29 are usable.", 20);
  addParam("DebugLevel4XTalk", m_debugLevel4XTalk, "Debug level for crosstalk; 20-29 are usable.", 21);

#if defined(CDC_DEBUG)
  cout << " " << endl;
  cout << "CDCDigitizer constructor" << endl;
#endif
}

void CDCDigitizerModule::initialize()
{
  m_simHits.isRequired(m_inputCDCSimHitsName);
  m_simClockState.isOptional();

  // Register the arrays in the DataStore, that are to be added in this module.
  m_cdcHits.registerInDataStore(m_outputCDCHitsName);
  m_simHits.registerRelationTo(m_cdcHits);
  m_mcParticles.registerRelationTo(m_cdcHits);
  // Arrays for trigger.
  m_cdcHits4Trg.registerInDataStore(m_outputCDCHitsName4Trg);
  m_simHits.registerRelationTo(m_cdcHits4Trg);
  m_mcParticles.registerRelationTo(m_cdcHits4Trg);

  m_cdcgp = &(CDCGeometryPar::Instance());
  CDCGeometryPar& cdcgp = *m_cdcgp;
  m_tdcBinWidth  = cdcgp.getTdcBinWidth();
  m_tdcBinWidthInv = 1. / m_tdcBinWidth;
  m_tdcResol     = m_tdcBinWidth / sqrt(12.);
  m_driftV       = cdcgp.getNominalDriftV();
  m_driftVInv    = 1. / m_driftV;
  m_propSpeedInv = 1. / cdcgp.getNominalPropSpeed();
  m_scaleFac = 1.;
  if (m_cdcgp->getMaterialDefinitionMode() == 0) { //gas+wire mode
    m_scaleFac = m_gasToGasWire;
  }
  m_scaleFac *= Unit::GeV;
  m_gcp = &(CDCGeoControlPar::getInstance());
  m_totalFudgeFactor  = m_cdcgp->getFudgeFactorForSigma(2);
  m_totalFudgeFactor *= m_addFudgeFactorForSigma;
  B2DEBUG(m_debugLevel, "totalFugeF in Digi= " << m_totalFudgeFactor);
  /*
      m_fraction = 1.0;
      m_resolution1 = cdcgp.getNominalSpaceResol();
      m_resolution2 = 0.;
      m_mean1 = 0.;
      m_mean2 = 0.;
  */

  if (m_useDB4FEE) {
    m_fEElectronicsFromDB = new DBArray<CDCFEElectronics>;
    if ((*m_fEElectronicsFromDB).isValid()) {
      (*m_fEElectronicsFromDB).addCallback(this, &CDCDigitizerModule::setFEElectronics);
      setFEElectronics();
    } else {
      B2FATAL("CDCDigitizer:: CDCFEElectronics not valid !");
    }
  }

  /*
  if (m_useDB4EDepToADC) {
    m_eDepToADCConversionsFromDB = new DBObjPtr<CDCEDepToADCConversions>;
    if ((*m_eDepToADCConversionsFromDB).isValid()) {
      (*m_eDepToADCConversionsFromDB).addCallback(this, &CDCDigitizerModule::setEDepToADCConversions);
      setEDepToADCConversions();
    } else {
      B2FATAL("CDCDigitizer:: CDCEDepToADCConversions not valid !");
    }
  }
  */

  if (m_useDB4RunGain) {
    m_runGainFromDB = new DBObjPtr<CDCDedxRunGain>;
    if ((*m_runGainFromDB).isValid()) {
      (*m_runGainFromDB).addCallback(this, &CDCDigitizerModule::setRunGain);
      setRunGain();
    } else {
      B2FATAL("CDCDedxRunGain invalid!");
    }
  }
  B2DEBUG(m_debugLevel, "run-gain = " << m_runGain);

  if (m_addXTalk) {
    m_xTalkFromDB = new DBObjPtr<CDCCrossTalkLibrary>;
    if ((*m_xTalkFromDB).isValid()) {
    } else {
      B2FATAL("CDCCrossTalkLibrary invalid!");
    }
  }

#if defined(CDC_DEBUG)
  cout << " " << endl;
  cout << "CDCDigitizer initialize" << endl;
  //  cout << "m_tdcOffset= " <<  m_tdcOffset << endl;
  cout << "m_tdcBinWidth= " <<  m_tdcBinWidth << endl;
  cout << "m_tdcResol= " <<  m_tdcResol << endl;
  cout << "m_driftV= " <<  m_driftV << endl;
  cout << "m_driftVInv= " <<  m_driftVInv << endl;
  cout << "m_propSpeedInv= " <<  m_propSpeedInv << endl;
  /*
    cout << "m_fraction= " <<  m_fraction << endl;
    cout << "m_resolution1= " <<  m_resolution1 << endl;
    cout << "m_resolution2= " <<  m_resolution2 << endl;
    cout << "m_mean1= " <<  m_mean1 << endl;
    cout << "m_mean2= " <<  m_mean2 << endl;
  */
#endif

  if (m_useDB4EDepToADC) {
    if (m_cdcgp->getEDepToADCMainFactor(0, 0) == 0.) {
      B2FATAL("CDCEDepToADCConversion payloads are unavailable!");
    }
  }

  // Set timing sim. mode
  if (m_useDB4FEE) {
    if (m_synchronization) { // syncronization
      m_tSimMode = 0;
    } else {
      if (m_randomization) { // radomization
        m_tSimMode = 1;
      } else {
        m_tSimMode = 2; // old sim.
      }
    }
  } else {
    m_tSimMode = 3; // old sim. w/o relying on fee db
  }
  B2DEBUG(m_debugLevel, "timing sim. mode= " << m_tSimMode);
  if (m_tSimMode < 0 || m_tSimMode > 3) B2FATAL("invalid timing sim. mode= " << m_tSimMode);
}

void CDCDigitizerModule::event()
{
  // Get SimHit array, MCParticle array, and relation between the two.
  RelationArray mcParticlesToCDCSimHits(m_mcParticles, m_simHits);  //RelationArray created by CDC SensitiveDetector


  //--- Start Digitization --------------------------------------------------------------------------------------------
  // Merge the hits in the same cell and save them into CDC signal map.

  // Define signal map
  map<WireID, SignalInfo> signalMap;
  map<WireID, SignalInfo>::iterator iterSignalMap;
  // Define adc map
  map<WireID, unsigned short> adcMap;
  map<WireID, unsigned short>::iterator iterADCMap;
  //  map<WireID, double> adcMap;
  //  map<WireID, double>::iterator iterADCMap;

  // signal map for trigger
  map<pair<WireID, unsigned>, SignalInfo> signalMapTrg;
  map<pair<WireID, unsigned>, SignalInfo>::iterator iterSignalMapTrg;

  // Set time window per event
  if (m_tSimMode == 0 || m_tSimMode == 1) {
    int trigBin = 0;
    if (m_simClockState.isValid()) {
      trigBin = m_simClockState->getCDCTriggerBin(m_offsetForTriggerBin);
    } else {
      if (m_tSimMode == 0) {
        B2DEBUG(m_debugLevel, "SimClockState unavailable so switched the mode from synchro to random.");
        m_tSimMode = 1;
      }
      trigBin = gRandom->Integer(4);
    }
    if (trigBin < 0 || trigBin > 3) B2ERROR("Invalid trigger bin; must be an integer [0,3]!");
    unsigned short offs = 8 * trigBin + m_trgTimingOffsetInCount;
    B2DEBUG(m_debugLevel, "tSimMode,trigBin,offs= " << m_tSimMode << " " << trigBin << " " << offs);

    //TODO: simplify the following 7 lines by modifying setFEElectronics()
    for (unsigned short bd = 1; bd < nBoards; ++bd) {
      const short tMaxInCount = 32 * (m_shiftOfTimeWindowIn32Count - m_trgDelayInCount[bd]) - offs;
      const short tMinInCount = tMaxInCount - 32 * m_widthOfTimeWindowInCount[bd];
      B2DEBUG(m_debugLevel, bd << " " << tMinInCount << " " << tMaxInCount);
      m_uprEdgeOfTimeWindow[bd] = m_tdcBinWidth * tMaxInCount;
      m_lowEdgeOfTimeWindow[bd] = m_tdcBinWidth * tMinInCount;
    }
  }

  // Set trigger timing jitter for this event
  double trigTiming = m_trigTimeJitter == 0. ? 0. : m_trigTimeJitter * (gRandom->Uniform() - 0.5);
  //  std::cout << "trigTiming= " << trigTiming << std::endl;
  // Loop over all hits
  int nHits = m_simHits.getEntries();
  B2DEBUG(m_debugLevel, "Number of CDCSimHits in the current event: " << nHits);
  for (int iHits = 0; iHits < nHits; ++iHits) {
    // Get a hit
    m_aCDCSimHit = m_simHits[iHits];

    // Hit geom. info
    m_wireID = m_aCDCSimHit->getWireID();
    //    B2DEBUG(29, "Encoded wire number of current CDCSimHit: " << m_wireID);

    m_posFlag    = m_aCDCSimHit->getLeftRightPassageRaw();
    m_boardID    = m_cdcgp->getBoardID(m_wireID);
    //    B2DEBUG(29, "m_boardID= " << m_boardID);
    m_posWire    = m_aCDCSimHit->getPosWire();
    m_posTrack   = m_aCDCSimHit->getPosTrack();
    m_momentum   = m_aCDCSimHit->getMomentum();
    m_flightTime = m_aCDCSimHit->getFlightTime();
    m_globalTime = m_aCDCSimHit->getGlobalTime();
    m_driftLength = m_aCDCSimHit->getDriftLength() * Unit::cm;

    //include alignment effects
    //basically align flag should be always on since on/off is controlled by the input alignment.xml file itself.
    m_align = true;

    TVector3 bwpAlign = m_cdcgp->wireBackwardPosition(m_wireID, CDCGeometryPar::c_Aligned);
    TVector3 fwpAlign = m_cdcgp->wireForwardPosition(m_wireID, CDCGeometryPar::c_Aligned);

    TVector3 bwp = m_cdcgp->wireBackwardPosition(m_wireID);
    TVector3 fwp = m_cdcgp->wireForwardPosition(m_wireID);

    //skip correction for wire-position alignment if unnecessary
    if ((bwpAlign - bwp).Mag() == 0. && (fwpAlign - fwp).Mag() == 0.) m_align = false;
    //    std::cout << "a m_align= " << m_align << std::endl;

    if (m_align || m_correctForWireSag) {

      bwp = bwpAlign;
      fwp = fwpAlign;

      if (m_correctForWireSag) {
        double zpos = m_posWire.z();
        double bckYSag = bwp.y();
        double forYSag = fwp.y();

        //        CDCGeometryPar::EWirePosition set = m_align ?
        //                                            CDCGeometryPar::c_Aligned : CDCGeometryPar::c_Base;
        CDCGeometryPar::EWirePosition set = CDCGeometryPar::c_Aligned;
        const int layerID = m_wireID.getICLayer();
        const int  wireID = m_wireID.getIWire();
        m_cdcgp->getWireSagEffect(set, layerID, wireID, zpos, bckYSag, forYSag);
        bwp.SetY(bckYSag);
        fwp.SetY(forYSag);
      }

      const TVector3 L = 5. * m_momentum.Unit(); //(cm) tentative
      TVector3 posIn  = m_posTrack - L;
      TVector3 posOut = m_posTrack + L;
      TVector3 posTrack = m_posTrack;
      TVector3 posWire = m_posWire;

      //      m_driftLength = m_cdcgp->ClosestApproach(bwp, fwp, posIn, posOut, posTrack, posWire);
      m_driftLength = ClosestApproach(bwp, fwp, posIn, posOut, posTrack, posWire);
      //      std::cout << "base-dl, sag-dl, diff= " << m_aCDCSimHit->getDriftLength() <<" "<< m_driftLength <<" "<< m_driftLength - m_aCDCSimHit->getDriftLength() << std::endl;
      m_posTrack = posTrack;
      m_posWire  = posWire;

      double deltaTime = 0.; //tentative (probably ok...)
      //      double deltaTime = (posTrack - m_posTrack).Mag() / speed;
      m_flightTime += deltaTime;
      m_globalTime += deltaTime;
      m_posFlag = m_cdcgp->getNewLeftRightRaw(m_posWire, m_posTrack, m_momentum);
    }

    // Calculate measurement time.
    // Smear drift length
    double hitDriftLength = m_driftLength;
    double dDdt = getdDdt(hitDriftLength);
    if (m_doSmearing) {
      hitDriftLength = smearDriftLength(hitDriftLength, dDdt);
    }

    //set flags
    bool addTof   = m_addTimeOfFlight4Bg;
    bool addDelay = m_addInWirePropagationDelay4Bg;
    if (m_aCDCSimHit->getBackgroundTag() == 0) {
      addTof   = m_addTimeOfFlight;
      addDelay = m_addInWirePropagationDelay;
    }
    double hitDriftTime = getDriftTime(hitDriftLength, addTof, addDelay);

    //add randamized event time for a beam bg. hit
    if (m_aCDCSimHit->getBackgroundTag() != 0) {
      hitDriftTime += m_globalTime - m_flightTime;
    }

    //add trigger timing jitter
    hitDriftTime += trigTiming;

    //apply time window cut
    double tMin = m_tMin;
    double tMax = m_tMaxOuter;
    if (m_wireID.getISuperLayer() == 0) tMax = m_tMaxInner;
    if (m_tSimMode <= 2) {
      tMin = m_lowEdgeOfTimeWindow[m_boardID];
      tMax = m_uprEdgeOfTimeWindow[m_boardID];
    }
    if (hitDriftTime < tMin || hitDriftTime > tMax) continue;

    //Sum ADC count
    const double stepLength  = m_aCDCSimHit->getStepLength() * Unit::cm;
    const double costh = m_momentum.z() / m_momentum.Mag();
    const double hitdE = m_scaleFac * m_aCDCSimHit->getEnergyDep();
    //    B2DEBUG(29, "m_scaleFac,UnitGeV= " << m_scaleFac <<" "<< Unit::GeV);
    unsigned short layerID = m_wireID.getICLayer();
    unsigned short cellID  = m_wireID.getIWire();
    //    unsigned short adcCount = getADCCount(layerID, cellID, hitdE, stepLength, costh);
    unsigned short adcCount = getADCCount(m_wireID, hitdE, stepLength, costh);
    const unsigned short adcTh = m_useDB4FEE ? m_adcThresh[m_boardID] : m_adcThreshold;
    //    B2DEBUG(29, "adcTh,adcCount= " << adcTh <<" "<< adcCount);
    if (adcCount < adcTh) adcCount = 0;
    iterADCMap = adcMap.find(m_wireID);
    if (iterADCMap == adcMap.end()) {
      adcMap.insert(make_pair(m_wireID, adcCount));
      //      adcMap.insert(make_pair(m_wireID, hitdE));
    } else {
      iterADCMap->second += adcCount;
      //      iterADCMap->second += hitdE;
    }

    //Apply energy threshold
    // If hitdE < dEThreshold, the hit is ignored
    // M. Uchida 2012.08.31
    double dEThreshold = 0.;
    if (m_useDB4FEE && m_useDB4EDepToADC) {
      dEThreshold = (m_tdcThresh[m_boardID] / m_cdcgp->getEDepToADCMainFactor(layerID, cellID)) * Unit::keV;
    } else {
      dEThreshold = (m_wireID.getISuperLayer() == 0) ? m_tdcThreshold4Inner : m_tdcThreshold4Outer;
      dEThreshold *= Unit::eV;
    }
    B2DEBUG(m_debugLevel, "hitdE,dEThreshold,driftLength " << hitdE << " " << dEThreshold << " " << hitDriftLength);

    if (hitdE < dEThreshold) {
      B2DEBUG(m_debugLevel, "Below Ethreshold: " << hitdE << " " << dEThreshold);
      continue;
    }

    // add one hit per trigger time window to the trigger signal map
    unsigned short trigWindow = floor((hitDriftTime - tMin) * m_tdcBinWidthInv / 32);
    iterSignalMapTrg = signalMapTrg.find(make_pair(m_wireID, trigWindow));
    if (iterSignalMapTrg == signalMapTrg.end()) {
      //      signalMapTrg.insert(make_pair(make_pair(m_wireID, trigWindow),
      //                                    SignalInfo(iHits, hitDriftTime, hitdE)));
      signalMapTrg.insert(make_pair(make_pair(m_wireID, trigWindow),
                                    SignalInfo(iHits, hitDriftTime, adcCount)));
    } else {
      if (hitDriftTime < iterSignalMapTrg->second.m_driftTime) {
        iterSignalMapTrg->second.m_driftTime = hitDriftTime;
        iterSignalMapTrg->second.m_simHitIndex = iHits;
      }
      //      iterSignalMapTrg->second.m_charge += hitdE;
      iterSignalMapTrg->second.m_charge += adcCount;
    }

    // Reject totally-dead wire; to be replaced by isDeadWire() in future
    if (m_cdcgp->isBadWire(m_wireID)) {
      //      std::cout<<"badwire= " << m_wireID.getICLayer() <<" "<< m_wireID.getIWire() << std::endl;
      continue;
    }
    // Reject partly-dead wire as well
    double eff = 1.;
    if (m_cdcgp->isDeadWire(m_wireID, eff)) {
      //      std::cout << "wid,eff= " << m_wireID << " " << eff << std::endl;
      if (eff < gRandom->Uniform()) continue;
    }

    // For TOT simulation, calculate drift length from In to the wire, and Out to the wire. The calculation is apprximate ignoring wire sag (this would be ok because TOT simulation is not required to be so accurate).
    const double a = bwpAlign.X();
    const double b = bwpAlign.Y();
    const double c = bwpAlign.Z();
    const TVector3 fmbAlign = fwpAlign - bwpAlign;
    const double lmn = 1. / fmbAlign.Mag();
    const double l = fmbAlign.X() * lmn;
    const double m = fmbAlign.Y() * lmn;
    const double n = fmbAlign.Z() * lmn;

    double dx = m_aCDCSimHit->getPosIn().X() - a;
    double dy = m_aCDCSimHit->getPosIn().Y() - b;
    double dz = m_aCDCSimHit->getPosIn().Z() - c;
    double sub = l * dx + m * dy + n * dz;
    const double driftLFromIn = sqrt(dx * dx + dy * dy + dz * dz - sub * sub);

    dx = m_aCDCSimHit->getPosOut().X() - a;
    dy = m_aCDCSimHit->getPosOut().Y() - b;
    dz = m_aCDCSimHit->getPosOut().Z() - c;
    sub = l * dx + m * dy + n * dz;
    const double driftLFromOut = sqrt(dx * dx + dy * dy + dz * dz - sub * sub);

    const double maxDriftL = std::max(driftLFromIn, driftLFromOut);
    const double minDriftL = m_driftLength;
    B2DEBUG(m_debugLevel, "driftLFromIn= " << driftLFromIn << " driftLFromOut= " << driftLFromOut << " minDriftL= " << minDriftL <<
            " maxDriftL= "
            <<
            maxDriftL << "m_driftLength= " << m_driftLength);

    iterSignalMap = signalMap.find(m_wireID);

    if (iterSignalMap == signalMap.end()) {
      // new entry
      //      signalMap.insert(make_pair(m_wireID, SignalInfo(iHits, hitDriftTime, hitdE)));
      signalMap.insert(make_pair(m_wireID, SignalInfo(iHits, hitDriftTime, adcCount, maxDriftL, minDriftL)));
      B2DEBUG(m_debugLevel, "Creating new Signal with encoded wire number: " << m_wireID);
    } else {
      // ... smallest drift time has to be checked, ...
      if (hitDriftTime < iterSignalMap->second.m_driftTime) {
        iterSignalMap->second.m_driftTime3 = iterSignalMap->second.m_driftTime2;
        iterSignalMap->second.m_simHitIndex3 = iterSignalMap->second.m_simHitIndex2;
        iterSignalMap->second.m_driftTime2 = iterSignalMap->second.m_driftTime;
        iterSignalMap->second.m_simHitIndex2 = iterSignalMap->second.m_simHitIndex;
        iterSignalMap->second.m_driftTime   = hitDriftTime;
        iterSignalMap->second.m_simHitIndex = iHits;
        B2DEBUG(m_debugLevel, "hitDriftTime of current Signal: " << hitDriftTime << ",  hitDriftLength: " << hitDriftLength);
      } else if (hitDriftTime < iterSignalMap->second.m_driftTime2) {
        iterSignalMap->second.m_driftTime3 = iterSignalMap->second.m_driftTime2;
        iterSignalMap->second.m_simHitIndex3 = iterSignalMap->second.m_simHitIndex2;
        iterSignalMap->second.m_driftTime2   = hitDriftTime;
        iterSignalMap->second.m_simHitIndex2 = iHits;
      } else if (hitDriftTime < iterSignalMap->second.m_driftTime3) {
        iterSignalMap->second.m_driftTime3   = hitDriftTime;
        iterSignalMap->second.m_simHitIndex3 = iHits;
      }
      // ... total charge has to be updated.
      //      iterSignalMap->second.m_charge += hitdE;
      iterSignalMap->second.m_charge += adcCount;

      // set max and min driftLs
      if (iterSignalMap->second.m_maxDriftL < maxDriftL) iterSignalMap->second.m_maxDriftL = maxDriftL;
      if (iterSignalMap->second.m_minDriftL > minDriftL) iterSignalMap->second.m_minDriftL = minDriftL;
      B2DEBUG(m_debugLevel, "maxDriftL in struct= " << iterSignalMap->second.m_maxDriftL << "minDriftL in struct= " <<
              iterSignalMap->second.m_minDriftL);
    }

  } // end loop over SimHits.

  //--- Now Store the results into CDCHits and
  // create corresponding relations between SimHits and CDCHits.

  unsigned int iCDCHits = 0;
  RelationArray cdcSimHitsToCDCHits(m_simHits, m_cdcHits); //SimHit<->CDCHit
  RelationArray mcParticlesToCDCHits(m_mcParticles, m_cdcHits); //MCParticle<->CDCHit

  for (iterSignalMap = signalMap.begin(); iterSignalMap != signalMap.end(); ++iterSignalMap) {

    //add time-walk (here for simplicity)
    //    unsigned short adcCount = getADCCount(iterSignalMap->second.m_charge);
    //    unsigned short adcCount = iterSignalMap->second.m_charge;
    iterADCMap = adcMap.find(iterSignalMap->first);
    unsigned short adcCount = iterADCMap != adcMap.end() ? iterADCMap->second : 0;
    /*
    unsigned short adcCount = 0;
    if (iterADCMap != adcMap.end()) {
      adcCount = getADCCount(iterSignalMap->first, iterADCMap->second, 1., 0.);
      unsigned short boardID = m_cdcgp->getBoardID(iterSignalMap->first);
      //      B2DEBUG(29, "boardID= " << boardID);
      const unsigned short adcTh = m_useDB4FEE ? m_adcThresh[boardID] : m_adcThreshold;
      if (adcCount < adcTh) adcCount = 0;
    }
    */

    if (m_addTimeWalk) {
      B2DEBUG(m_debugLevel, "timewalk= " << m_cdcgp->getTimeWalk(iterSignalMap->first, adcCount));
      iterSignalMap->second.m_driftTime += m_cdcgp->getTimeWalk(iterSignalMap->first, adcCount);
    }

    //remove negative drift time (TDC) upon request
    if (!m_outputNegativeDriftTime &&
        iterSignalMap->second.m_driftTime < 0.) {
      continue;
    }

    //N.B. No bias (+ or -0.5 count) is introduced on average in digitization by the real TDC (info. from KEK electronics division). So round off (t0 - drifttime) below.
    unsigned short tdcCount = static_cast<unsigned short>((m_cdcgp->getT0(iterSignalMap->first) - iterSignalMap->second.m_driftTime) *
                                                          m_tdcBinWidthInv + 0.5);

    //calculate tot; hard-coded currently
    double deltaDL = iterSignalMap->second.m_maxDriftL - iterSignalMap->second.m_minDriftL;
    if (deltaDL < 0.) {
      B2DEBUG(m_debugLevel, "negative deltaDL= " << deltaDL);
      deltaDL = 0.;
    }
    const unsigned short boardID = m_cdcgp->getBoardID(iterSignalMap->first);
    unsigned short tot = std::min(std::round(5.92749 * deltaDL + 2.59706), static_cast<double>(m_widthOfTimeWindowInCount[boardID]));
    if (m_adcThresh[boardID] > 0) {
      tot = std::min(static_cast<int>(tot), static_cast<int>(adcCount / m_adcThresh[boardID]));
    }

    CDCHit* firstHit = m_cdcHits.appendNew(tdcCount, adcCount, iterSignalMap->first, 0, tot);
    //    std::cout <<"firsthit?= " << firstHit->is2ndHit() << std::endl;
    //set a relation: CDCSimHit -> CDCHit
    cdcSimHitsToCDCHits.add(iterSignalMap->second.m_simHitIndex, iCDCHits);

    //set a relation: MCParticle -> CDCHit
    RelationVector<MCParticle> rels = m_simHits[iterSignalMap->second.m_simHitIndex]->getRelationsFrom<MCParticle>();
    if (rels.size() != 0) {
      //assumption: only one MCParticle
      const MCParticle* mcparticle = rels[0];
      double weight = rels.weight(0);
      mcparticle->addRelationTo(firstHit, weight);
    }

    //Set 2nd-hit related things if it exists
    if (m_output2ndHit && iterSignalMap->second.m_simHitIndex2 >= 0) {
      unsigned short tdcCount2 = static_cast<unsigned short>((m_cdcgp->getT0(iterSignalMap->first) - iterSignalMap->second.m_driftTime2) *
                                                             m_tdcBinWidthInv + 0.5);
      if (tdcCount2 != tdcCount) {
        CDCHit* secondHit = m_cdcHits.appendNew(tdcCount2, adcCount, iterSignalMap->first, 0, tot);
        secondHit->set2ndHitFlag();
        secondHit->setOtherHitIndices(firstHit);
        //  std::cout <<"2ndhit?= " << secondHit->is2ndHit() << std::endl;
        //  std::cout <<"1st-otherhitindex= " << firstHit->getOtherHitIndex() << std::endl;
        //  std::cout <<"2nd-otherhitindex= " << secondHit->getOtherHitIndex() << std::endl;
        //  secondHit->setOtherHitIndex(firstHit->getArrayIndex());
        //  firstHit->setOtherHitIndex(secondHit->getArrayIndex());
        //  std::cout <<"1st-otherhitindex= " << firstHit->getOtherHitIndex() << std::endl;
        //  std::cout <<"2nd-otherhitindex= " << secondHit->getOtherHitIndex() << std::endl;

        //set a relation: CDCSimHit -> CDCHit
        ++iCDCHits;
        cdcSimHitsToCDCHits.add(iterSignalMap->second.m_simHitIndex2, iCDCHits);
        //        std::cout << "settdc2 " << firstHit->getTDCCount() << " " << secondHit->getTDCCount() << std::endl;

        //set a relation: MCParticle -> CDCHit
        rels = m_simHits[iterSignalMap->second.m_simHitIndex2]->getRelationsFrom<MCParticle>();
        if (rels.size() != 0) {
          //assumption: only one MCParticle
          const MCParticle* mcparticle = rels[0];
          double weight = rels.weight(0);
          mcparticle->addRelationTo(secondHit, weight);
        }
      } else { //Check the 3rd hit when tdcCount = tdcCount2
        //        std::cout << "tdcCount1=2" << std::endl;
        if (iterSignalMap->second.m_simHitIndex3 >= 0) {
          unsigned short tdcCount3 = static_cast<unsigned short>((m_cdcgp->getT0(iterSignalMap->first) - iterSignalMap->second.m_driftTime3) *
                                                                 m_tdcBinWidthInv + 0.5);
          //          std::cout << "tdcCount3= " << tdcCount3 << " " << tdcCount << std::endl;
          if (tdcCount3 != tdcCount) {
            CDCHit* secondHit = m_cdcHits.appendNew(tdcCount3, adcCount, iterSignalMap->first, 0, tot);
            secondHit->set2ndHitFlag();
            secondHit->setOtherHitIndices(firstHit);
            //      secondHit->setOtherHitIndex(firstHit->getArrayIndex());
            //      firstHit->setOtherHitIndex(secondHit->getArrayIndex());
            //      std::cout <<"2ndhit?= " << secondHit->is2ndHit() << std::endl;

            //set a relation: CDCSimHit -> CDCHit
            ++iCDCHits;
            cdcSimHitsToCDCHits.add(iterSignalMap->second.m_simHitIndex3, iCDCHits);
            //            std::cout << "settdc3 " << firstHit->getTDCCount() << " " << secondHit->getTDCCount() << std::endl;

            //set a relation: MCParticle -> CDCHit
            rels = m_simHits[iterSignalMap->second.m_simHitIndex3]->getRelationsFrom<MCParticle>();
            if (rels.size() != 0) {
              //assumption: only one MCParticle
              const MCParticle* mcparticle = rels[0];
              double weight = rels.weight(0);
              mcparticle->addRelationTo(secondHit, weight);
            }
          }
        }
      } //end of checking tdcCount 1=2 ?
    } //end of 2nd hit setting

    //    std::cout <<"t0= " << m_cdcgp->getT0(iterSignalMap->first) << std::endl;
    /*    unsigned short tdcInCommonStop = static_cast<unsigned short>((m_tdcOffset - iterSignalMap->second.m_driftTime) * m_tdcBinWidthInv);
    float driftTimeFromTDC = static_cast<float>(m_tdcOffset - (tdcInCommonStop + 0.5)) * m_tdcBinWidth;
    std::cout <<"driftT bf digitization, TDC in common stop, digitized driftT = " << iterSignalMap->second.m_driftTime <<" "<< tdcInCommonStop <<" "<< driftTimeFromTDC << std::endl;
    */
    ++iCDCHits;
  }

  //Add crosstalk
  if (m_addXTalk) addXTalk();

  // Store the results with trigger time window in a separate array
  // with corresponding relations.
  for (iterSignalMapTrg = signalMapTrg.begin(); iterSignalMapTrg != signalMapTrg.end(); ++iterSignalMapTrg) {
    /*
    unsigned short adcCount = getADCCount(iterSignalMapTrg->first.first, iterSignalMapTrg->second.m_charge, 1., 0.);
    unsigned short boardID = m_cdcgp->getBoardID(iterSignalMapTrg->first.first);
    //    B2DEBUG(29, "boardID= " << boardID);
    const unsigned short adcTh = m_useDB4FEE ? m_adcThresh[boardID] : m_adcThreshold;
    if (adcCount < adcTh) adcCount = 0;
    */
    //    unsigned short adcCount = getADCCount(iterSignalMapTrg->second.m_charge);
    unsigned short adcCount = iterSignalMapTrg->second.m_charge;
    unsigned short tdcCount =
      static_cast<unsigned short>((m_cdcgp->getT0(iterSignalMapTrg->first.first) -
                                   iterSignalMapTrg->second.m_driftTime) * m_tdcBinWidthInv + 0.5);
    const CDCHit* cdcHit = m_cdcHits4Trg.appendNew(tdcCount, adcCount, iterSignalMapTrg->first.first);

    // relations
    m_simHits[iterSignalMapTrg->second.m_simHitIndex]->addRelationTo(cdcHit);
    RelationVector<MCParticle> rels = m_simHits[iterSignalMapTrg->second.m_simHitIndex]->getRelationsFrom<MCParticle>();
    if (rels.size() != 0) {
      //assumption: only one MCParticle
      const MCParticle* mcparticle = rels[0];
      double weight = rels.weight(0);
      mcparticle->addRelationTo(cdcHit, weight);
    }
  }

  /*
  std::cout << " " << std::endl;
  RelationIndex<MCParticle, CDCHit> mcp_to_hit(mcParticles, cdcHits);
  if (!mcp_to_hit) B2FATAL("No MCParticle -> CDCHit relation founf!");
  typedef RelationIndex<MCParticle, CDCHit>::Element RelationElement;
  int ncdcHits = cdcHits.getEntries();
  for (int j = 0; j < ncdcHits; ++j) {
    for (const RelationElement& rel : mcp_to_hit.getElementsTo(cdcHits[j])) {
      std::cout << j << " " << cdcHits[j]->is2ndHit() <<" "<< rel.from->getIndex() << " " << rel.weight << std::endl;
    }
  }
  */
}

double CDCDigitizerModule::smearDriftLength(const double driftLength, const double dDdt)
{
  double mean = 0.;
  double resolution;

  if (m_useSimpleDigitization) {
    if (gRandom->Uniform() <= m_fraction) {
      mean = m_mean1;
      resolution = m_resolution1;
    } else {
      mean = m_mean2;
      resolution = m_resolution2;
    }
  } else {
    const unsigned short leftRight = m_posFlag;
    double alpha = m_cdcgp->getAlpha(m_posWire, m_momentum);
    double theta = m_cdcgp->getTheta(m_momentum);
    resolution = m_cdcgp->getSigma(driftLength, m_wireID.getICLayer(), leftRight, alpha, theta);
    resolution *= m_totalFudgeFactor;
  }

  //subtract resol. due to digitization, which'll be added later in the digitization

  double diff = resolution - dDdt * m_tdcResol;
  if (diff > 0.) {
    resolution = sqrt(diff * (resolution + dDdt * m_tdcResol));
  } else {
    resolution = 0.;
  }

#if defined(CDC_DEBUG)
  cout << " " << endl;
  cout << "CDCDigitizerModule::smearDriftLength" << endl;
  cout << "tdcResol= " << m_tdcResol << endl;
  cout << "dDdt,resolution= " << dDdt << " " << resolution << endl;
#endif

  // Smear drift length
  double newDL = gRandom->Gaus(driftLength + mean , resolution);
  while (newDL <= 0.) newDL = gRandom->Gaus(driftLength + mean, resolution);
  //  cout << "totalFugeF in Digi= " << m_totalFudgeFactor << endl;
  return newDL;
}


double CDCDigitizerModule::getdDdt(const double driftL)
{
  //---------------------------------------------------------------------------------
  // Calculates the 1'st derivative: dD/dt, where D: drift length before smearing; t: drift time
  //---------------------------------------------------------------------------------

  double dDdt = m_driftV;

  if (!m_useSimpleDigitization) {
    const unsigned short layer = m_wireID.getICLayer();
    const unsigned short leftRight = m_posFlag;
    double alpha = m_cdcgp->getAlpha(m_posWire, m_momentum);
    double theta = m_cdcgp->getTheta(m_momentum);
    double t = m_cdcgp->getDriftTime(driftL, layer, leftRight, alpha, theta);
    dDdt = m_cdcgp->getDriftV(t, layer, leftRight, alpha, theta);

#if defined(CDC_DEBUG)
    cout << " " << endl;
    cout << "CDCDigitizerModule::getdDdt" << endl;
    cout << "**layer= " << layer << endl;
    cout << "alpha= " << 180.*alpha / M_PI << std::endl;
    if (layer == 55) {
      int lr = 0;
      for (int i = 0; i < 1000; ++i) {
        t = 1.0 * i;
        double d = m_cdcgp->getDriftLength(t, layer, lr, alpha, theta);
        cout << t << " " << d << endl;
      }

      cout << " " << endl;

      lr = 1;
      for (int i = 0; i < 100; ++i) {
        t = 5 * i;
        double d = m_cdcgp->getDriftLength(t, layer, lr, alpha, theta);
        cout << t << " " << d << endl;
      }
      exit(-1);
    }
#endif
  }

  return dDdt;
}


double CDCDigitizerModule::getDriftTime(const double driftLength, const bool addTof, const bool addDelay)
{
  //---------------------------------------------------------------------------------
  // Method returning electron drift time (parameters: position in cm)
  // T(drift) = TOF + T(true drift time) + T(propagation delay in wire) - T(event),
  // T(event) is a timing of event, which includes a time jitter due to
  // the trigger system.
  //---------------------------------------------------------------------------------

  double driftT = 0.;

  if (m_useSimpleDigitization) {
    driftT = (driftLength / Unit::cm) * m_driftVInv;

#if defined(CDC_DEBUG)
    cout << " " << endl;
    cout << "CDCDigitizerModule::getDriftTime" << endl;
    cout << "driftvinv= " << m_driftVInv << endl;
#endif
  } else {
    const unsigned short layer = m_wireID.getICLayer();
    const unsigned short leftRight = m_posFlag;
    double alpha = m_cdcgp->getAlpha(m_posWire, m_momentum);
    double theta = m_cdcgp->getTheta(m_momentum);
    driftT = m_cdcgp->getDriftTime(driftLength, layer, leftRight, alpha, theta);
    //    std::cout <<"alpha,theta,driftT= " << alpha <<" "<< theta <<" "<< driftT << std::endl;
  }

  if (addTof) {
    driftT += m_flightTime; // in ns
  }

  if (addDelay) {
    //calculate signal propagation length in the wire
    CDCGeometryPar::EWirePosition set = m_align ? CDCGeometryPar::c_Aligned : CDCGeometryPar::c_Base;
    TVector3 backWirePos = m_cdcgp->wireBackwardPosition(m_wireID, set);

    double propLength = (m_posWire - backWirePos).Mag();
    //    if (m_cdcgp->getSenseWireZposMode() == 1) {
    //TODO: replace the following with cached reference
    //    std::cout << m_gcp->getInstance().getSenseWireZposMode() << std::endl;
    if (m_gcp->getSenseWireZposMode() == 1) {
      const unsigned short layer = m_wireID.getICLayer();
      propLength += m_cdcgp->getBwdDeltaZ(layer);
    }
    //    B2DEBUG(29, "Propagation in wire length: " << propLength);

    if (m_useSimpleDigitization) {
      driftT += (propLength / Unit::cm) * m_propSpeedInv;

#if defined(CDC_DEBUG)
      cout << "pseedinv= " << m_propSpeedInv << endl;
#endif
    } else {
      const unsigned short layer = m_wireID.getICLayer();
      driftT += (propLength / Unit::cm) * m_cdcgp->getPropSpeedInv(layer);
#if defined(CDC_DEBUG)
      cout << "layer,pseedinv= " << layer << " " << m_cdcgp->getPropSpeedInv(layer) << endl;
#endif
    }
  }

  return driftT;
}


//unsigned short CDCDigitizerModule::getADCCount(unsigned short layer, unsigned short cell, double edep, double dx, double costh)
unsigned short CDCDigitizerModule::getADCCount(const WireID& wid, double dEinGeV, double dx, double costh)
{
  unsigned short adcCount = 0;
  if (dEinGeV <= 0. || dx <= 0.) return adcCount;

  // The current value is taken from E. Nakano from some test beam results. This should be a somewhat realistic value, but doesn't need to be exact.
  // Similar as geometry parameters are for the ideal geometry, not the real one.
  //  const double conversionEDepToADC = (100.0 / 3.2) * 1e6;
  double conv  = (100.0 / 3.2); //keV -> count
  const unsigned short layer = wid.getICLayer();
  const unsigned short cell  = wid.getIWire();
  double dEInkeV = dEinGeV / Unit::keV;
  if (m_spaceChargeEffect) {
    if (m_useDB4EDepToADC) conv = m_cdcgp->getEDepToADCConvFactor(layer, cell, dEInkeV, dx, costh);
  } else {
    if (m_useDB4EDepToADC) conv = m_cdcgp->getEDepToADCMainFactor(layer, cell);
  }
  conv *= m_runGain;

  //The ADCcount is obtained by rounding-up (measured voltage)/bin in real ADC. This is true both for pedestal and signal voltages, so the pedestal-subtracted ADCcount (simulated here) is rounded.
  adcCount = static_cast<unsigned short>(std::round(conv * dEInkeV));
  return adcCount;
}


// Set FEE parameters (from DB)
void CDCDigitizerModule::setFEElectronics()
{
  const double& off   = m_tdcThresholdOffset;
  const double& gA    = m_analogGain;
  const double& gD    = m_digitalGain;
  const double& adcBW = m_adcBinWidth;
  const double convF  = gA / gD / adcBW;
  const double el1TrgLatency = m_cdcgp->getMeanT0(); // ns
  B2DEBUG(m_debugLevel, "L1TRGLatency= " << el1TrgLatency);
  const double c = 32. * m_tdcBinWidth;

  if (!m_fEElectronicsFromDB) B2FATAL("No FEEElectronics dbobject!");
  const CDCFEElectronics& fp = *((*m_fEElectronicsFromDB)[0]);
  int mode = (fp.getBoardID() == -1) ? 1 : 0;
  int iNBoards = static_cast<int>(nBoards);

  //set typical values for all channels first if mode=1
  if (mode == 1) {
    for (int bdi = 1; bdi < iNBoards; ++bdi) {
      m_uprEdgeOfTimeWindow[bdi] = el1TrgLatency - c * (fp.getTrgDelay() + 1);
      if (m_uprEdgeOfTimeWindow[bdi] < 0.) B2FATAL("CDCDigitizer: Upper edge of time window < 0!");
      m_lowEdgeOfTimeWindow[bdi] = m_uprEdgeOfTimeWindow[bdi] - c * (fp.getWidthOfTimeWindow() + 1);
      if (m_lowEdgeOfTimeWindow[bdi] > 0.) B2FATAL("CDCDigitizer: Lower edge of time window > 0!");
      m_adcThresh[bdi] = fp.getADCThresh();
      m_tdcThresh[bdi] = convF * (off - fp.getTDCThreshInMV());
      m_widthOfTimeWindowInCount[bdi] = fp.getWidthOfTimeWindow() + 1;
      m_trgDelayInCount  [bdi] = fp.getTrgDelay();
    }
  }

  //ovewrite    values for specific channels if mode=1
  //set typical values for all channels if mode=0
  for (const auto& fpp : (*m_fEElectronicsFromDB)) {
    int bdi = fpp.getBoardID();
    if (mode == 0 && bdi ==  0) continue; //bdi=0 is dummy (not used)
    if (mode == 1 && bdi == -1) continue; //skip typical case
    if (bdi < 0 || bdi >= iNBoards) B2FATAL("CDCDigitizer:: Invalid no. of FEE board!");
    m_uprEdgeOfTimeWindow[bdi] = el1TrgLatency - c * (fpp.getTrgDelay() + 1);
    if (m_uprEdgeOfTimeWindow[bdi] < 0.) B2FATAL("CDCDigitizer: Upper edge of time window < 0!");
    m_lowEdgeOfTimeWindow[bdi] = m_uprEdgeOfTimeWindow[bdi] - c * (fpp.getWidthOfTimeWindow() + 1);
    if (m_lowEdgeOfTimeWindow[bdi] > 0.) B2FATAL("CDCDigitizer: Lower edge of time window > 0!");
    m_adcThresh[bdi] = fpp.getADCThresh();
    m_tdcThresh[bdi] = convF * (off - fpp.getTDCThreshInMV());
    m_widthOfTimeWindowInCount[bdi] = fpp.getWidthOfTimeWindow() + 1;
    m_trgDelayInCount  [bdi] = fpp.getTrgDelay();
  }

  //debug
  B2DEBUG(m_debugLevel, "mode= " << mode);
  for (int bdi = 1; bdi < iNBoards; ++bdi) {
    B2DEBUG(m_debugLevel, bdi << " " << m_lowEdgeOfTimeWindow[bdi] << " " << m_uprEdgeOfTimeWindow[bdi] << " " << m_adcThresh[bdi] <<
            " " <<
            m_tdcThresh[bdi]);
  }
}

// Set Run-gain (from DB)
void CDCDigitizerModule::setRunGain()
{
  m_runGain = (*m_runGainFromDB)->getRunGain();
  m_runGain *= m_overallGainFactor;
}

void CDCDigitizerModule::addXTalk()
{
  map<WireID, XTalkInfo> xTalkMap;
  map<WireID, XTalkInfo> xTalkMap1;
  map<WireID, XTalkInfo>::iterator iterXTalkMap1;

  // Loop over all cdc hits to create a xtalk map
  int OriginalNoOfHits = m_cdcHits.getEntries();
  B2DEBUG(m_debugLevel4XTalk, "\n \n" << "#CDCHits " << OriginalNoOfHits);
  for (const auto& aHit : m_cdcHits) {
    if (m_issue2ndHitWarning && aHit.is2ndHit()) {
      B2WARNING("2nd TDC hit found, but not ready for it!");
    }
    WireID wid(aHit.getID());
    //    B2DEBUG(m_debugLevel4XTalk, "Encoded wireid of current CDCHit: " << wid);
    short tdcCount = aHit.getTDCCount();
    short adcCount = aHit.getADCCount();
    short tot      = aHit.getTOT();
    short board    = m_cdcgp->getBoardID(wid);
    short channel  = m_cdcgp->getChannelID(wid);
    const vector<pair<short, asicChannel>> xTalks = (*m_xTalkFromDB)->getLibraryCrossTalk(channel, tdcCount, adcCount, tot);

    int nXTalks = xTalks.size();
    for (int i = 0; i < nXTalks; ++i) {
      const unsigned short tdcCount4XTalk = xTalks[i].second.TDC;
      if (i == 0) {
        B2DEBUG(m_debugLevel4XTalk, "\n" << "          signal: " << channel << " " << tdcCount << " " << adcCount << " " << tot);
      }
      B2DEBUG(m_debugLevel4XTalk, "xtalk: " << xTalks[i].first << " " << tdcCount4XTalk << " " << xTalks[i].second.ADC << " " <<
              xTalks[i].second.TOT);
      WireID widx = m_cdcgp->getWireID(board, xTalks[i].first);
      if (!m_cdcgp->isBadWire(widx)) { // for non-bad wire
        if (m_includeEarlyXTalks || (xTalks[i].second.TDC <= tdcCount)) {
          const double t0 = m_cdcgp->getT0(widx);
          const double ULOfTDC = (t0 - m_lowEdgeOfTimeWindow[board]) * m_tdcBinWidthInv;
          const double LLOfTDC = (t0 - m_uprEdgeOfTimeWindow[board]) * m_tdcBinWidthInv;
          if (LLOfTDC <= tdcCount4XTalk && tdcCount4XTalk <= ULOfTDC) {
            const unsigned short status = 0;
            xTalkMap.insert(make_pair(widx, XTalkInfo(tdcCount4XTalk, xTalks[i].second.ADC, xTalks[i].second.TOT, status)));
          }
        }
        //  } else {
        //    cout<<"badwire= " << widx.getICLayer() <<" "<< widx.getIWire() << endl;
      }
    } //end of xtalk loop
  } //end of cdc hit loop

  //Loop over all xtalk hits to creat a new xtalk map with only the fastest hits kept (approx.)
  B2DEBUG(m_debugLevel4XTalk, "#xtalk  hits: " << xTalkMap.size());
  for (const auto& aHit : xTalkMap) {
    WireID wid = aHit.first;

    iterXTalkMap1 = xTalkMap1.find(wid);
    unsigned short tdcCount = aHit.second.m_tdc;
    unsigned short adcCount = aHit.second.m_adc;
    unsigned short tot      = aHit.second.m_tot;
    unsigned short status   = aHit.second.m_status;

    if (iterXTalkMap1 == xTalkMap1.end()) { // new entry
      xTalkMap1.insert(make_pair(wid, XTalkInfo(tdcCount, adcCount, tot, status)));
      //      B2DEBUG(m_debugLevel4XTalk, "Creating a new xtalk hit with encoded wire no.: " << wid);
    } else { // not new; check if fastest
      if (tdcCount < iterXTalkMap1->second.m_tdc) {
        iterXTalkMap1->second.m_tdc = tdcCount;
        B2DEBUG(m_debugLevel4XTalk, "TDC-count of current xtalk: " << tdcCount);
      }
      iterXTalkMap1->second.m_adc += adcCount;
      iterXTalkMap1->second.m_tot += tot; // approx.
    }
  } // end of xtalk loop

  //add xtalk in the same way as the beam bg. overlay
  B2DEBUG(m_debugLevel4XTalk, "#xtalk1 hits: " << xTalkMap1.size());
  for (const auto& aX : xTalkMap1) {
    bool append = true;
    const unsigned short tdc4Bg = aX.second.m_tdc;
    const unsigned short adc4Bg = aX.second.m_adc;
    const unsigned short tot4Bg = aX.second.m_tot;
    const unsigned short status4Bg = aX.second.m_status;

    for (int iHit = 0; iHit < OriginalNoOfHits; ++iHit) {
      CDCHit& aH = *(m_cdcHits[iHit]);
      if (aH.getID() != aX.first.getEWire()) { //wire id unmatched
        continue;
      } else { //wire id matched
        append = false;
        const unsigned short tdc4Sg = aH.getTDCCount();
        const unsigned short adc4Sg = aH.getADCCount();
        const unsigned short tot4Sg = aH.getTOT();
        //  B2DEBUG(m_debuglevel4XTalk, "Sg tdc,adc,tot= " << tdc4Sg << " " << adc4Sg << " " << tot4Sg);
        //  B2DEBUG(m_debugLevel4XTalk, "Bg tdc,adc,tot= " << tdc4Bg << " " << adc4Bg << " " << tot4Bg);

        // If the BG hit is faster than the true hit, the TDC count is replaced, and
        // the relations are removed. ADC counts are summed up.
        if (tdc4Sg < tdc4Bg) {
          aH.setTDCCount(tdc4Bg);
          aH.setStatus(status4Bg);
          auto relSimHits = aH.getRelationsFrom<CDCSimHit>();
          for (int i = relSimHits.size() - 1; i >= 0; --i) {
            relSimHits.remove(i);
          }
          auto relMCParticles = aH.getRelationsFrom<MCParticle>();
          for (int i = relMCParticles.size() - 1; i >= 0; --i) {
            relMCParticles.remove(i);
          }
        }

        aH.setADCCount(adc4Sg + adc4Bg);

        //Set TOT for signal+background case. It is assumed that the start timing
        //of a pulse (input to ADC) is given by the TDC-count. This is an
        //approximation becasue analog (for ADC) and digital (for TDC) parts are
        //different in the front-end electronics.
        unsigned short s1 = tdc4Sg; //start time of 1st pulse
        unsigned short s2 = tdc4Bg; //start time of 2nd pulse
        unsigned short w1 = tot4Sg; //its width
        unsigned short w2 = tot4Bg; //its width
        if (tdc4Sg < tdc4Bg) {
          s1 = tdc4Bg;
          w1 = tot4Bg;
          s2 = tdc4Sg;
          w2 = tot4Sg;
        }
        w1 *= 32;
        w2 *= 32;
        const unsigned short e1 = s1 - w1; //end time of 1st pulse
        const unsigned short e2 = s2 - w2; //end time of 2nd pulse
        //  B2DEBUG(m_debuglevel4Xtalk, "s1,e1,w1,s2,e2,w2= " << s1 << " " << e1 << " " << w1 << " " << s2 << " " << e2 << " " << w2);

        double pulseW = w1 + w2;
        if (e1 <= e2) {
          pulseW = w1;
        } else if (e1 <= s2) {
          pulseW = s1 - e2;
        }

        unsigned short board = m_cdcgp->getBoardID(aX.first);
        aH.setTOT(std::min(std::round(pulseW / 32.), static_cast<double>(m_widthOfTimeWindowInCount[board])));
        B2DEBUG(m_debugLevel4XTalk, "replaced tdc,adc,tot,wid,status= " << aH.getTDCCount() << " " << aH.getADCCount() << " " << aH.getTOT()
                <<
                " " << aH.getID() << " " << aH.getStatus());
        break;
      }
    } //end of cdc hit loop

    if (append) {
      m_cdcHits.appendNew(tdc4Bg, adc4Bg, aX.first, status4Bg, tot4Bg);
      B2DEBUG(m_debugLevel4XTalk, "appended tdc,adc,tot,wid,status= " << tdc4Bg << " " << adc4Bg << " " << tot4Bg << " " << aX.first <<
              " " <<
              status4Bg);
    }
  } //end of x-talk loop
  B2DEBUG(m_debugLevel4XTalk, "original #hits, #hits= " << OriginalNoOfHits << " " << m_cdcHits.getEntries());
}
