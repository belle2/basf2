/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/modules/cdcDigitizer/CDCDigiModule.h>

//framework headers
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <generators/dataobjects/MCParticle.h>

//cdc package headers
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/HitCDC.h>
#include <cdc/geometry/CDCGeometryPar.h>

//root
#include <TVector3.h>

//C++ STL
#include <cstdlib>
#include <iomanip>
#include <math.h>
#include <time.h>
#include <utility> //contains pair



using namespace std;
using namespace boost;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CDCDigi)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCDigiModule::CDCDigiModule() : Module()
{
  // Set description
  setDescription("CDCDigiModuletizer");

  // Add parameters
  // I/O
  addParam("InputColName",                m_inColName, "Input collection name", string("CDCSimHits"));
  addParam("OutputColName",               m_outColName, "Output collection name", string("HitCDCArray"));
  addParam("CDCHitOutColName",            m_cdcHitOutColName, "Output collection name", string("CDCHits"));
  //Parameters for Digitization
  addParam("RandomSeed",                  m_randomSeed, "Random seed", 12345);
  addParam("Fraction",                    m_fraction, "The fraction of the first Gaussian used to smear drift length, set in cm", 0.571);
  addParam("Mean1",                       m_mean1, "The mean value of the first Gaussian used to smear drift length, set in cm", 0.0);
  addParam("Resolution1",                 m_resolution1, "Resolution of the first Gaussian used to smear drift length, set in cm", 0.0089);
  addParam("Mean2",                       m_mean2, "The mean value of the second Gaussian used to smear drift length, set in cm", 0.0);
  addParam("Resolution2",                 m_resolution2, "Resolution of the second Gaussian used to smear drift length, set in cm", 0.0188);
  addParam("ElectronicEffects",           m_electronicEffects, "Apply electronic effects?", 0);
  addParam("ElectronicsNoise",            m_elNoise, "Noise added by the electronics, set in ENC", 1000.0);
  addParam("AddTrueDriftTime",            m_addTrueDriftTime, "A switch used to control adding the true drift time into the final drift time or not", true);
  addParam("AddPropagationDelay",         m_addPropagationDelay, "A switch used to control adding propagation delay into the final drift time or not", true);
  addParam("AddTimeOfFlight",             m_addTimeOfFlight, "A switch used to control adding time of flight into the final drift time or not", true);
  addParam("EventTime",                   m_eventTime, "It is a timing of event, which includes a time jitter due to the trigger system, set in ns", 0.0);
  //Relations
  addParam("MCPartToCDCSimHitCollectionName", m_relColNameMCToSim,
           "Name of relation collection - MCParticle to SimCDCHit (if nonzero, created)", string("MCPartToCDCSimHits"));
  addParam("SimHitToCDCHitCollectionName", m_relColNameSimHitToHit,
           "Name of relation collection - Hit CDC to MCParticle (if nonzero, created)", string("SimHitToCDCHits"));
}

CDCDigiModule::~CDCDigiModule()
{
}

void CDCDigiModule::initialize()
{
  // Initialize variables
  m_nRun    = 0 ;
  m_nEvent  = 0 ;

  // Set variables in appropriate physical units
  m_mean1  *= Unit::cm;
  m_mean2  *= Unit::cm;
  m_resolution1  *= Unit::cm;
  m_resolution2  *= Unit::cm;

  // Initialize random generator (engine, mean, sigma)
  m_random = new TRandom3((UInt_t)m_randomSeed);

  // Print set parameters
  printModuleParams();

  // CPU time start
  m_timeCPU = clock() * Unit::us;

  StoreArray<CDCHit> cdcHitArray(m_cdcHitOutColName);
  StoreArray<CDCSimHit> cdcArray(m_inColName);
  StoreArray<MCParticle> mcParticles;

  RelationArray cdcSimHitToHitRel(cdcArray, cdcHitArray);
  RelationArray mcPartToCDCHitRel(mcParticles, cdcHitArray);

}

void CDCDigiModule::beginRun()
{
}

void CDCDigiModule::event()
{
  //------------------------------------------
  // Get CDC hits collection from data store.
  //------------------------------------------
  StoreArray<CDCSimHit> cdcArray(m_inColName);
  if (!cdcArray) {
    B2ERROR("Can not find " << m_inColName << ".");
  }
  StoreArray<MCParticle> mcParticles;                         //needed to use the relations with MCParticles
  RelationArray cdcSimHitRel(mcParticles, cdcArray);  //RelationsArray created by CDC SensitiveDetector

  //---------------------------------------------------------------------
  // Merge the hits in the same cell and save them into CDC signal map.
  //---------------------------------------------------------------------

  // Get number of hits in this event
  int nHits = cdcArray->GetEntriesFast();

  // Define signal map
  CDCSignalMap cdcSignalMap;
  cdcSignalMap.clear();
  CDCSignalMap::const_iterator iterCDCMap;

  // Get instance of cdc geometry parameters
  CDCGeometryPar* cdcp = CDCGeometryPar::Instance();

  // Loop over all hits
  for (int iHits = 0; iHits < nHits; iHits++) {
    // Get a hit
    CDCSimHit* aCDCSimHit = cdcArray[iHits];

    // Hit geom. info
    int hitLayerId       =   aCDCSimHit->getLayerId();
    int hitWireId        =   aCDCSimHit->getWireId();
    TVector3 hitPosWire  =   aCDCSimHit->getPosWire();
    TVector3 backWirePos =   cdcp->wireBackwardPosition(hitLayerId, hitWireId);

    // Hit phys. info
    double hitdEdx        = aCDCSimHit->getEnergyDep() * Unit::GeV;
    double hitDriftLength = aCDCSimHit->getDriftLength() * Unit::cm;
    double hitTOF         = aCDCSimHit->getFlightTime() * Unit::ns;

    // calculate signal propagation length in the wire
    double propLength     = (hitPosWire - backWirePos).Mag();

    // smear drift length
    hitDriftLength        = smearDriftLength(hitDriftLength, m_fraction, m_mean1, m_resolution1, m_mean2, m_resolution2);

    // calculate drift time
    double hitDriftTime   = getDriftTime(hitDriftLength, hitTOF, propLength);

    bool ifNewDigi = true;
    // The first SimHit is always a new digit, but the looping will anyhow end immediately.
    for (iterCDCMap = cdcSignalMap.begin(); iterCDCMap != cdcSignalMap.end(); iterCDCMap++) {

      // Check if new SimHit is in cell of existing Signal.
      if ((iterCDCMap->second->getLayerId() == hitLayerId) && (iterCDCMap->second->getWireId() == hitWireId)) {

        // If true, the SimHit doesn't create a new digit, ...
        ifNewDigi = false;

        // ... smallest drift time has to be checked, ...
        if (hitDriftTime < iterCDCMap->second->getDriftTime()) {
          iterCDCMap->second->setHitNumber(iHits);
          iterCDCMap->second->setDriftTime(hitDriftTime);
          iterCDCMap->second->setDriftLength(hitDriftLength);
          B2DEBUG(250, "hitDriftTime: " << hitDriftTime);
          B2DEBUG(250, "hitDriftLength: " << hitDriftLength);
        }

        // ... total charge has to be updated.
        iterCDCMap->second->updateCharge(hitdEdx);

        //A SimHit will not be in more than one cell.
        break;
      }
    } // End loop over previously stored Signal.

    // If it is a new hit, save it to signal map.
    if (ifNewDigi == true) {
      CDCSignal* signal = new CDCSignal(iHits, hitLayerId, hitWireId, hitdEdx, hitDriftLength, hitDriftTime);
      cdcSignalMap.insert(vpair(iHits, signal));
    }
  } // end loop over SimHits.


  //-----------------------------------------------------
  // Smear drift length and save digits into data store.
  //-----------------------------------------------------
  int iDigits = 0;
  // Arrays for CDCHits and Relations between SimHit and CDCHit.
  StoreArray<CDCHit> cdcHitArray(m_cdcHitOutColName);
  //StoreArray<Relation> cdcSimRelation(m_relColNameSimHitToHit);

  RelationArray cdcSimHitToHitRel(cdcArray, cdcHitArray); //SimHit<->CDCHit
  RelationArray mcPartToCDCHitRel(mcParticles, cdcHitArray); //MCParticle<->CDCHit

  for (iterCDCMap = cdcSignalMap.begin(); iterCDCMap != cdcSignalMap.end(); iterCDCMap++) {

    // Smear drift length using double  Gaussion, based on spatial resolution.
    //double hitNewDriftLength =  smearDriftLength(iterCDCMap->second->getDriftLength(),
    //                                             m_fraction, m_mean1, m_resolution1, m_mean2, m_resolution2);
    //iterCDCMap->second->setDriftLength(hitNewDriftLength);
    //B2DEBUG(250, "NewHitDriftLength " << iterCDCMap->second->getDriftLength());

    // Save digits into data store
    // Next lines are the storage for HitCDC connected to trasan, which will eventually become obsolete.
    StoreArray<HitCDC> NNcdcArray(m_outColName);
    new(NNcdcArray->AddrAt(iDigits)) HitCDC();
    NNcdcArray[iDigits]->setLayerId(iterCDCMap->second->getLayerId());
    NNcdcArray[iDigits]->setWireId(iterCDCMap->second->getWireId());
    NNcdcArray[iDigits]->setLeftDriftLength(iterCDCMap->second->getDriftLength());
    NNcdcArray[iDigits]->setRightDriftLength(iterCDCMap->second->getDriftLength());
    NNcdcArray[iDigits]->setLeftDriftTime(iterCDCMap->second->getDriftTime());
    NNcdcArray[iDigits]->setRightDriftTime(iterCDCMap->second->getDriftTime());
    NNcdcArray[iDigits]->setCharge(iterCDCMap->second->getCharge());
    // End Lines referring to HitCDC.

    // Save digits into the DataStore as CDCHit TClonesArray.

    // The next two lines are geometry dependent. It seems very reasonable to save layers dependent on their
    // belonging to a SuperLayer. So perhaps this can be somehow incorporated earlier...
    int iSuperLayer = (iterCDCMap->second->getLayerId() - 2) / 6;
    int iLayer = (iterCDCMap->second->getLayerId() - 2) % 6;
    if (iSuperLayer == 0) {iLayer += 2;}

    new(cdcHitArray->AddrAt(iDigits)) CDCHit(iterCDCMap->second->getDriftLength(), iterCDCMap->second->getCharge(),
                                             iSuperLayer, iLayer, iterCDCMap->second->getWireId());

    cdcSimHitToHitRel.add(iterCDCMap->second->getHitNumber(), iDigits);     //add entry

    for (int index = 0; index < cdcSimHitRel.getEntries(); index++) {
      for (int hit = 0; hit < (int)cdcSimHitRel[index].getToIndices().size(); hit++) {
        if ((int)cdcSimHitRel[index].getToIndex(hit) == iterCDCMap->second->getHitNumber()) {
          mcPartToCDCHitRel.add(cdcSimHitRel[index].getFromIndex(), iDigits);      //add entry
        }
      }
    }


    // Creation of Relation between SimHit, that has smalles drift length in each cell and the CDCHit.
    B2DEBUG(150, "First: " << (iterCDCMap->first) << "iDigits" << iDigits);
    //new(cdcSimRelation->AddrAt(iDigits)) Relation(cdcArray, cdcHitArray, iterCDCMap->second->getHitNumber(), iDigits);
    B2DEBUG(150, "START");
    B2DEBUG(150, "SimHitDriftLength: " << cdcArray[iterCDCMap->first]->getDriftLength());
    B2DEBUG(150, "CDCHitDriftLength: " << cdcHitArray[iDigits]->getDriftTime());
    B2DEBUG(159, "END");

    // Count number of digits
    iDigits++;

    //delete pointer
    delete iterCDCMap->second;
  }

  m_nEvent++;
}

double CDCDigiModule::smearDriftLength(double driftLength, double fraction, double mean1, double resolution1, double mean2, double resolution2)
{
  // Smear drift length using double Gaussian function
  double mean, resolution;
  if (m_random->Uniform() <= fraction) {
    mean = mean1;
    resolution = resolution1;
  } else {
    mean = mean2;
    resolution = resolution2;
  }

  // Smear drift length
  double newDL = m_random->Gaus(driftLength / Unit::cm + mean / Unit::cm, resolution / Unit::cm);
  while (newDL <= 0.) newDL = m_random->Gaus(driftLength / Unit::cm + mean / Unit::cm, resolution / Unit::cm);
  return newDL * Unit::cm;
}

void CDCDigiModule::endRun()
{
}

void CDCDigiModule::terminate()
{
  // CPU time end
  m_timeCPU = clock() * Unit::us - m_timeCPU;

  if (m_random)  delete m_random;
  // Print message
  //B2INFO(   " "
  //     << "Time per event: "
  //     << std::setiosflags(std::ios::fixed | std::ios::internal )
  //     << std::setprecision(3)
  //     << m_timeCPU/m_nEvent/ms
  //     << " ms"
  //     << "\n"
  //     << std::setprecision(3)
  //     << "\n"
  //     << DGREEN
  //     << " "
  //     << "Processor succesfully finished!"
  //     << ENDCOLOR);
}

void CDCDigiModule::genNoise(CDCSignalMap&)  //cdcSignalMap)
{
  //-------------------------------------------------------------------------------
  // Method generating random noise using Gaussian distribution (input parameter:
  // sensor map of strips with total integrated charge)
  //-------------------------------------------------------------------------------
  /*
    SensorStripMap::iterator iterSMap;
    StripChargeMap::iterator iterChMap;

    double elNoise;

    // Add noise, only if set nonzero!
    if (_elNoise != 0.) for (iterSMap=sensorMap.begin(); iterSMap!=sensorMap.end(); iterSMap++) {

       // Strips in R-Phi
      for (iterChMap=iterSMap->second[STRIPRPHI].begin(); iterChMap!=iterSMap->second[STRIPRPHI].end(); iterChMap++) {

        elNoise = _genGauss->fire();

        iterChMap->second->updateCharge(elNoise);

      }

      // Strips in Z
        for (iterChMap=iterSMap->second[STRIPZ].begin(); iterChMap!=iterSMap->second[STRIPZ].end(); iterChMap++) {

           elNoise = _genGauss->fire();

           iterChMap->second->updateCharge(elNoise);

        }


    } // For
  */
}

double CDCDigiModule::getDriftTime(double driftLength, double tof, double propLength)
{
  //---------------------------------------------------------------------------------
  // Method returning electron drift time (parameters: position in cm)
  // T(drift) = TOF + T(true drift time) + T(propagation delay in wire) - T(event),
  // T(event) is a timing of event, which includes a time jitter due to
  // the trigger system.
  //---------------------------------------------------------------------------------
  double driftTimeFactor = 1.0;
  double propagationDelayFactor = 1.0;
  double tofFactor = 1.0;

  if (!m_addTrueDriftTime)     driftTimeFactor = 0;
  if (!m_addPropagationDelay)  propagationDelayFactor = 0;
  if (!m_addTimeOfFlight)      tofFactor = 0;

  //drift speed: 4.0cm/us, propagation speed: 27.25cm/ns, provided by iwasaki-san and hard-coded here.
  return (1000 * driftTimeFactor * (driftLength / Unit::cm) / 4.0 + tofFactor * tof / Unit::ns + propagationDelayFactor * (propLength / Unit::cm) / 27.25 + m_eventTime);
}

void CDCDigiModule::printCDCSimHitInfo(const CDCSimHit& aHit) const
{
  //----------------------
  // Printing a hit info.
  //----------------------
  double depE = aHit.getEnergyDep() / Unit::keV;
  B2INFO("    Hit:"
         << std::fixed
         << std::setprecision(1)
         << " DepE [keV]: "     << depE
         << std::setprecision(3)
         << std::setiosflags(std::ios::showpos)
         << " Pos X [mm]: " << (aHit.getPosIn()[0] + aHit.getPosOut()[0]) / 2. / Unit::mm
         << " Pos Y [mm]: " << (aHit.getPosIn()[1] + aHit.getPosOut()[1]) / 2. / Unit::mm
         << " Pos Z [mm]: " << (aHit.getPosIn()[2] + aHit.getPosOut()[2]) / 2. / Unit::mm
         << " DriftLength [mm]: "      << aHit.getDriftLength() / Unit::mm
         << " Time of flight [ns]: "      << aHit.getFlightTime() / Unit::ns
         << " Position flag: "      << aHit.getPosFlag()
         << std::resetiosflags(std::ios::showpos)
         << std::setprecision(0));
}

void CDCDigiModule::printCDCSimHitsInfo(std::string info, const CDCSimHitVec& hitVec) const
{
  //---------------------
  // Printing hits info.
  //---------------------
  CDCSimHitVec::const_iterator iterHits;

  B2INFO("   " << info << ":" << "\n");
  for (iterHits = hitVec.begin(); iterHits != hitVec.end(); iterHits++) {
    printCDCSimHitInfo(**iterHits);
  }
}

void CDCDigiModule::printModuleParams() const
{
  //-----------------------------
  // Printing module parameters.
  //-----------------------------
  B2INFO(" "
         << "CDCDigiModule parameters:"
         << " "
         << "\n");

  B2INFO("  Input collection name:                 " << std::setw(6) << m_inColName << "\n");

  B2INFO("                                         " << "\n");

  B2INFO("  Output collection name:                " << std::setw(6) << m_outColName << "\n");

  B2INFO("                                         " << "\n");

  //B2INFO(   "  Apply electronic effect?               " << std::setw(6) << m_electronicEffects << "\n" );
  //
  //B2INFO(   "                                         " << "\n"
  //     << std::setiosflags(std::ios::fixed | std::ios::internal )
  //     << std::setprecision(2)
  //     << "  Electronics noise - ENC [fC]:          " << std::setw(6) << m_elNoise      << "\n");

  B2INFO("                                         " << "\n"
         << "  Mean1 [um]:                     " << std::setw(6) << m_mean1 / Unit::um  << "\n"
         << "  Resolution1 [um]:               " << std::setw(6) << m_resolution1 / Unit::um  << "\n"
         << "  Mean2 [um]:                     " << std::setw(6) << m_mean2 / Unit::um  << "\n"
         << "  Resolution2 [um]:               " << std::setw(6) << m_resolution2 / Unit::um  << "\n"
         << std::resetiosflags(std::ios::showpos)
         << std::setprecision(0)
         << "\n");
}

void CDCDigiModule::printCDCSignalInfo(std::string info, const CDCSignalMap& cdcSignalMap) const
{
  //---------------------------------------------
  // Printing info about signals in each cell.
  //---------------------------------------------

  //  map of cdc digits with total integrated charge
  CDCSignalMap::const_iterator iterCDCMap;

  B2INFO("  Digi results - " << info
         << ":");

  // Loop over all digits
  for (iterCDCMap = cdcSignalMap.begin(); iterCDCMap != cdcSignalMap.end(); iterCDCMap++) {
    int layerId = iterCDCMap->second->getLayerId();
    int wireId =  iterCDCMap->second->getWireId();

    B2INFO("   Layer: " << layerId
           << "   Wire: "  << wireId);

    // cdc signal (digits)
    B2INFO("                 " << "\n"
           << std::setiosflags(std::ios::fixed | std::ios::internal)
           << std::setprecision(2)
           << " Total charge [keV]: "  << iterCDCMap->second->getCharge() / Unit::keV
           << " Drift length [mm]: " << iterCDCMap->second->getDriftLength() / Unit::mm
           << std::setprecision(0));
  } // end loop
}

