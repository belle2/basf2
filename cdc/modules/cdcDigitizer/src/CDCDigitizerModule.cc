/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/modules/cdcDigitizer/CDCDigitizerModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <mdst/dataobjects/MCParticle.h>

#include <cdc/dataobjects/CDCHit.h>
//#include <cdc/geometry/CDCGeometryPar.h>

#include <TRandom.h>

#include <utility>

using namespace std;
using namespace Belle2;
using namespace CDC;

// register module
REG_MODULE(CDCDigitizer)
CDCDigitizerModule::CDCDigitizerModule() : Module(),
  m_tdcOffset(0.0), m_cdcp(), m_aCDCSimHit(), m_tdcBinWidth(1.0),
  m_tdcBinWidthInv(1.0), m_tdcResol(0.144), m_driftV(4.0e-3),
  m_driftVInv(250.0), m_propSpeedInv(27.25)
{
  // Set description
  setDescription("Creates CDCHits from CDCSimHits.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Add parameters
  // I/O
  addParam("InputCDCSimHitsName",         m_inputCDCSimHitsName, "Name of input array. Should consist of CDCSimHits.", string(""));
  addParam("OutputCDCHitsName",           m_outputCDCHitsName,   "Name of output array. Will consist of CDCHits.",     string(""));

  //Relations
  addParam("MCParticlesToCDCSimHitsName",  m_MCParticlesToSimHitsName,
           "Name of relation between MCParticles and CDCSimHits used",     string(""));
  addParam("CDCSimHistToCDCHitsName",      m_SimHitsTOCDCHitsName,
           "Name of relation between the CDCSimHits and the CDCHits used", string(""));


  //Parameters for Digitization
  addParam("UseSimpleDigitization",       m_useSimpleDigitization,
           "If true, a simple x-t with a constant velocity is used for the drift-length to -time conversion", true);

  //float Gauss Parameters
  addParam("Fraction",                    m_fraction,    "Fraction of first Gaussian used to smear drift length in cm",    1.0);
  addParam("Mean1",                       m_mean1,       "Mean value of first Gaussian used to smear drift length in cm",  0.0000);
  addParam("Resolution1",                 m_resolution1, "Resolution of first Gaussian used to smear drift length in cm",  0.0130);
  addParam("Mean2",                       m_mean2,       "Mean value of second Gaussian used to smear drift length in cm", 0.0000);
  addParam("Resolution2",                 m_resolution2, "Resolution of second Gaussian used to smear drift length in cm", 0.0000);

  //Switches to control time information handling
  addParam("AddInWirePropagationDelay",   m_addInWirePropagationDelay,
           "A switch used to control adding propagation delay in the wire into the final drift time or not; this is for signal hits.", false);
  addParam("AddInWirePropagationDelay4Bg",  m_addInWirePropagationDelay4Bg,
           "The same switch but for beam bg. hits.", true);
  addParam("AddTimeOfFlight",  m_addTimeOfFlight,
           "A switch used to control adding time of flight into the final drift time or not; this is for signal hits.", false);
  addParam("AddTimeOfFlight4Bg",   m_addTimeOfFlight4Bg,
           "The same switch but for beam bg. hits.", true);
  addParam("OutputNegativeDriftTime", m_outputNegativeDriftTime, "Output negative drift time", false);

  //TDC Threshold
  addParam("Threshold", m_tdcThreshold,
           "dEdx value for TDC Threshold in eV", 40.0);
  addParam("tMin", m_tMin, "Lower edge of time window in ns", -100.);
  addParam("tMaxOuter", m_tMaxOuter, "Upper edge of time window in ns for the outer layers", 500.);
  addParam("tMaxInner", m_tMaxInner, "Upper edge of time window in ns for the inner layers", 300.);
  // The following doesn't make any sense. The only reasonable steerable would be a switch to decide if the jitter shall be
  // activated. Then there has to be event by event jitter.
  /*  addParam("EventTime",                   m_eventTime,
             "It is a timing of event, which includes a time jitter due to the trigger system, set in ns",     float(0.0));*/

#if defined(CDC_DEBUG)
  cout << " " << endl;
  cout << "CDCDigitizer constructor" << endl;
#endif
}

void CDCDigitizerModule::initialize()
{
  StoreArray<CDCSimHit>::required(m_inputCDCSimHitsName);

  // Register the arrays in the DataStore, that are to be added in this module.
  StoreArray<CDCHit>::registerPersistent(m_outputCDCHitsName);
  RelationArray::registerPersistent<CDCSimHit, CDCHit>(m_inputCDCSimHitsName, m_outputCDCHitsName);
  RelationArray::registerPersistent<MCParticle, CDCHit>("", m_outputCDCHitsName);

  m_cdcp = &(CDCGeometryPar::Instance());
  CDCGeometryPar& cdcp = *m_cdcp;
  m_tdcOffset    = cdcp.getTdcOffset();
  m_tdcBinWidth  = cdcp.getTdcBinWidth();
  m_tdcBinWidthInv = 1. / m_tdcBinWidth;
  m_tdcBinHwidth = m_tdcBinWidth / 2.;
  m_tdcResol     = m_tdcBinWidth / sqrt(12.);
  m_driftV       = cdcp.getNominalDriftV();
  m_driftVInv    = 1. / m_driftV;
  m_propSpeedInv = 1. / cdcp.getNominalPropSpeed();
  /*
      m_fraction = 1.0;
      m_resolution1 = cdcp.getNominalSpaceResol();
      m_resolution2 = 0.;
      m_mean1 = 0.;
      m_mean2 = 0.;
  */

#if defined(CDC_DEBUG)
  cout << " " << endl;
  cout << "CDCDigitizer initialize" << endl;
  cout << "m_tdcOffset= " <<  m_tdcOffset << endl;
  cout << "m_tdcBinWidth= " <<  m_tdcBinWidth << endl;
  cout << "m_tdcBinHwidth= " <<  m_tdcBinHwidth << endl;
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

}

void CDCDigitizerModule::event()
{
  // Get SimHit array, MCParticle array, and relation between the two.
  StoreArray<CDCSimHit> simHits(m_inputCDCSimHitsName);

  StoreArray<MCParticle> mcParticles;                //needed to use the relations with MCParticles
  RelationArray mcParticlesToCDCSimHits(mcParticles, simHits);  //RelationArray created by CDC SensitiveDetector


  //--- Start Primitive Digitization --------------------------------------------------------------------------------------------
  // Merge the hits in the same cell and save them into CDC signal map.

  // Define signal map
  map<WireID, SignalInfo> signalMap;
  map<WireID, SignalInfo>::iterator iterSignalMap;

  // Loop over all hits
  int nHits = simHits.getEntries();
  B2DEBUG(250, "Number of CDCSimHits in the current event: " << nHits);
  for (int iHits = 0; iHits < nHits; ++iHits) {
    // Get a hit
    m_aCDCSimHit = simHits[iHits];

    // Hit geom. info
    m_wireID        =   m_aCDCSimHit->getWireID();
    B2DEBUG(250, "Encoded wire number of current CDCSimHit: " << m_wireID);

    // Hit phys. info
    float hitdEdx        = m_aCDCSimHit->getEnergyDep()   * Unit::GeV;
    float hitDriftLength = m_aCDCSimHit->getDriftLength() * Unit::cm;
    float dedxThreshold = m_tdcThreshold * Unit::eV;

    // If hitdEdx < dedxThreshold (default 40 eV), the hit is ignored
    // M. Uchida 2012.08.31
    //
    if (hitdEdx < dedxThreshold) {
      B2DEBUG(250, "Below Ethreshold: " << hitdEdx << " " << dedxThreshold);
      continue;
    }

    B2DEBUG(250, "Energy deposition: " << hitdEdx << ", DriftLength: " << hitDriftLength);

    // calculate measurement time.

    // smear drift length
    float dDdt = getdDdt(hitDriftLength);
    hitDriftLength = smearDriftLength(hitDriftLength, dDdt);

    //set flags
    bool addTof   = m_addTimeOfFlight4Bg;
    bool addDelay = m_addInWirePropagationDelay4Bg;
    if (m_aCDCSimHit->getBackgroundTag() == 0) {
      addTof   = m_addTimeOfFlight;
      addDelay = m_addInWirePropagationDelay;
    }
    float hitDriftTime = getDriftTime(hitDriftLength, addTof, addDelay);

    //add randamized event time for a beam bg. hit
    if (m_aCDCSimHit->getBackgroundTag() != 0) {
      hitDriftTime +=
        m_aCDCSimHit->getGlobalTime() - m_aCDCSimHit->getFlightTime();
    }

    //apply time window cut
    double tMax = m_tMaxOuter;
    if (m_wireID.getISuperLayer() == 0) tMax = m_tMaxInner;
    if (hitDriftTime < m_tMin || hitDriftTime > tMax) continue;

    iterSignalMap = signalMap.find(m_wireID);

    if (iterSignalMap == signalMap.end()) {
      // new entry
      signalMap.insert(make_pair(m_wireID, SignalInfo(iHits, hitDriftTime, hitdEdx)));
      B2DEBUG(150, "Creating new Signal with encoded wire number: " << m_wireID);

    } else {
      // ... smallest drift time has to be checked, ...

      if (hitDriftTime < iterSignalMap->second.m_driftTime) {
        iterSignalMap->second.m_driftTime   = hitDriftTime;
        iterSignalMap->second.m_simHitIndex = iHits;
        B2DEBUG(250, "hitDriftTime of current Signal: " << hitDriftTime << ",  hitDriftLength: " << hitDriftLength);
      }
      // ... total charge has to be updated.
      iterSignalMap->second.m_charge += hitdEdx;
    }
  } // end loop over SimHits.

  //--- Now Store the results into CDCHits and
  // create corresponding relations between SimHits and CDCHits.

  unsigned int iCDCHits = 0;

  StoreArray<CDCHit> cdcHits(m_outputCDCHitsName);
  cdcHits.create();

  RelationArray cdcSimHitsToCDCHits(simHits, cdcHits); //SimHit<->CDCHit
  RelationArray mcParticlesToCDCHits(mcParticles, cdcHits); //MCParticle<->CDCHit

  for (iterSignalMap = signalMap.begin(); iterSignalMap != signalMap.end(); ++iterSignalMap) {

    //remove negative drift time (TDC) upon request

    if (!m_outputNegativeDriftTime &&
        iterSignalMap->second.m_driftTime < -m_tdcBinHwidth) {
      continue;
    }

    cdcHits.appendNew(static_cast<unsigned short>(iterSignalMap->second.m_driftTime * m_tdcBinWidthInv + 0.5) + m_tdcOffset, getADCCount(iterSignalMap->second.m_charge),
                      iterSignalMap->first);

    //add entry : CDCSimHit <-> CDCHit
    cdcSimHitsToCDCHits.add(iterSignalMap->second.m_simHitIndex, iCDCHits);

    const CDCHit* cdcHit = cdcHits[cdcHits.getEntries() - 1];
    RelationVector<MCParticle> rels = simHits[iterSignalMap->second.m_simHitIndex]->getRelationsFrom<MCParticle>();

    if (rels.size() != 0) {
      //assumption: only one MCParticle
      const MCParticle* mcparticle = rels[0];
      double weight = rels.weight(0);

      mcparticle->addRelationTo(cdcHit, weight);
    }

    ++iCDCHits;
  }

}

float CDCDigitizerModule::smearDriftLength(const float driftLength, const float dDdt)
{
  float mean = 0.;
  float resolution;

  if (m_useSimpleDigitization) {
    if (gRandom->Uniform() <= m_fraction) {
      mean = m_mean1;
      resolution = m_resolution1;
    } else {
      mean = m_mean2;
      resolution = m_resolution2;
    }
  } else {
    resolution = m_cdcp->getSigma(driftLength, m_wireID.getICLayer());
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
  float newDL = gRandom->Gaus(driftLength + mean , resolution);
  while (newDL <= 0.) newDL = gRandom->Gaus(driftLength + mean, resolution);
  return newDL;
}


float CDCDigitizerModule::getdDdt(const float driftL)
{
  //---------------------------------------------------------------------------------
  // Calculates the 1'st derivative: dD/dt, where D: drift length before smearing; t: drift time
  //---------------------------------------------------------------------------------

  float dDdt = m_driftV;

  if (!m_useSimpleDigitization) {
    const unsigned short layer = m_wireID.getICLayer();
    const unsigned short leftRight = m_aCDCSimHit->getPosFlag();
    double alpha = m_cdcp->getAlpha(m_aCDCSimHit->getPosWire(),
                                    m_aCDCSimHit->getMomentum());
    double theta = m_cdcp->getTheta(m_aCDCSimHit->getMomentum());
    double t = m_cdcp->getDriftTime(driftL, layer, leftRight, alpha, theta);
    dDdt = m_cdcp->getDriftV(t, layer, leftRight, alpha, theta);

#if defined(CDC_DEBUG)
    cout << " " << endl;
    cout << "CDCDigitizerModule::getdDdt" << endl;
    cout << "**layer= " << layer << endl;
    if (layer == 55) {
      int lr = 0;
      for (int i = 0; i < 100; ++i) {
        t = 5 * i;
        double d = m_cdcp->getDriftLength(t, layer, lr, alpha, theta);
        cout << t << " " << d << endl;
      }

      cout << " " << endl;

      lr = 1;
      for (int i = 0; i < 100; ++i) {
        t = 5 * i;
        double d = m_cdcp->getDriftLength(t, layer, lr, alpha, theta);
        cout << t << " " << d << endl;
      }
      //      exit(-1);
    }
#endif
  }

  return dDdt;
}


float CDCDigitizerModule::getDriftTime(const float driftLength, const bool addTof, const bool addDelay)
{
  //---------------------------------------------------------------------------------
  // Method returning electron drift time (parameters: position in cm)
  // T(drift) = TOF + T(true drift time) + T(propagation delay in wire) - T(event),
  // T(event) is a timing of event, which includes a time jitter due to
  // the trigger system.
  //---------------------------------------------------------------------------------

  float driftT = 0.;

  if (m_useSimpleDigitization) {
    driftT = (driftLength / Unit::cm) * m_driftVInv;

#if defined(CDC_DEBUG)
    cout << " " << endl;
    cout << "CDCDigitizerModule::getDriftTime" << endl;
    cout << "driftvinv= " << m_driftVInv << endl;
#endif
  } else {
    const unsigned short layer = m_wireID.getICLayer();
    const unsigned short leftRight = m_aCDCSimHit->getPosFlag();
    double alpha = m_cdcp->getAlpha(m_aCDCSimHit->getPosWire(),
                                    m_aCDCSimHit->getMomentum());
    double theta = m_cdcp->getTheta(m_aCDCSimHit->getMomentum());
    driftT = m_cdcp->getDriftTime(driftLength, layer, leftRight, alpha, theta);
    //    std::cout <<"alpha,theta,driftT= " << alpha <<" "<< theta <<" "<< driftT << std::endl;
  }

  if (addTof) {
    driftT += m_aCDCSimHit->getFlightTime(); // in ns
  }

  if (addDelay) {
    //calculate signal propagation length in the wire
    TVector3 backWirePos = m_cdcp->wireBackwardPosition(m_wireID);
    TVector3 hitPosWire  = m_aCDCSimHit->getPosWire();
    double propLength = (hitPosWire - backWirePos).Mag();
    B2DEBUG(250, "Propagation in wire length: " << propLength);

    if (m_useSimpleDigitization) {
      driftT += (propLength / Unit::cm) * m_propSpeedInv;

#if defined(CDC_DEBUG)
      cout << "pseedinv= " << m_propSpeedInv << endl;
#endif
    } else {
      const unsigned short layer = m_wireID.getICLayer();
      driftT += (propLength / Unit::cm) * m_cdcp->getPropSpeedInv(layer);
#if defined(CDC_DEBUG)
      cout << "layer,pseedinv= " << layer << " " << m_cdcp->getPropSpeedInv(layer) << endl;
#endif
    }
  }

  return driftT;
}

unsigned short CDCDigitizerModule::getADCCount(const float charge)
{
  // The current value is taken from E. Nakano from some test beam results. This should be a somewhat realistic value, but doesn't need to be exact.
  // Similar as geometry parameters are for the ideal geometry, not the real one.
  float conversionChargeToADC = (100.0 / 3.2) * 1e6;
  return static_cast<unsigned short>(conversionChargeToADC * charge);
}
