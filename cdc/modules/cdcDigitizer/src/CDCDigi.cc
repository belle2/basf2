/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/modules/cdcDigitizer/CDCDigi.h>

//framework headers
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/datastore/RelationArray.h>
#include <framework/logging/Logger.h>

//cdc package headers
#include <cdc/modules/cdcDigitizer/CDCColours.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/hitcdc/HitCDC.h>

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
REG_MODULE(CDCDigi, "CDCDigitizer")

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCDigi::CDCDigi() : Module()
{
  // Set description
  setDescription("CDCDigitizer");

  // Add parameters
  // I/O
  addParam("InputColName",                m_inColName, string("SimHitCDCArray"), "Input collection name");
  addParam("OutputColName",               m_outColName, string("HitCDCArray"), "Output collection name");
  addParam("CDCHitOutColName",            m_cdcHitOutColName, string("CDCHitCollection"), "Output collection name");
  //Parameters for Digitization
  addParam("RandomSeed",                  m_randomSeed, 12345, "Random seed");
  addParam("Fraction",                    m_fraction, 0.571, "The fraction of the first Gaussian used to smear drift length, set in cm");
  addParam("Mean1",                       m_mean1, 0.0, "The mean value of the first Gaussian used to smear drift length, set in cm");
  addParam("Resolution1",                 m_resolution1, 0.0089, "Resolution of the first Gaussian used to smear drift length, set in cm");
  addParam("Mean2",                       m_mean1, 0.0, "The mean value of the second Gaussian used to smear drift length, set in cm");
  addParam("Resolution2",                 m_resolution2, 0.0188, "Resolution of the second Gaussian used to smear drift length, set in cm");
  addParam("ElectronicEffects",           m_electronicEffects, 0, "Apply electronic effects?");
  addParam("ElectronicsNoise",            m_elNoise, 1000.0, "Noise added by the electronics, set in ENC");
  //Relations
  addParam("RelCollectionName_MCPartToSimHit", m_relColNameMCToSim,    string("MCPartToCDCSimHitCollection"),
           "Name of relation collection - MCParticle to SimCDCHit (if nonzero, created)");
  addParam("RelCollectionName_SimHitToCDCHit", m_relColNameSimHitToHit, string("SimHitToCDCHitCollection"),
           "Name of relation collection - Hit CDC to MCParticle (if nonzero, created)");
}

CDCDigi::~CDCDigi()
{
}

void CDCDigi::initialize()
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
}

void CDCDigi::beginRun()
{
}

void CDCDigi::event()
{
  //------------------------------------------
  // Get CDC hits collection from data store.
  //------------------------------------------
  StoreArray<SimHitCDC> cdcArray(m_inColName);
  if (!cdcArray) {
    B2ERROR("Can not find " << m_inColName << ".");
  }

  //---------------------------------------------------------------------
  // Merge the hits in the same cell and save them into CDC signal map.
  //---------------------------------------------------------------------

  // Get number of hits in this event
  int nHits = cdcArray->GetEntriesFast();

  // Define signal map
  CDCSignalMap cdcSignalMap;
  cdcSignalMap.clear();
  CDCSignalMap::const_iterator iterCDCMap;

  // Loop over all hits
  for (int iHits = 0; iHits < nHits; iHits++) {
    // Get a hit
    SimHitCDC* aSimHitCDC = cdcArray[iHits];

    // Hit geom. info
    int hitLayerId =   aSimHitCDC->getLayerId();
    int hitWireId  =   aSimHitCDC->getWireId();

    // Hit phys. info
    double hitdEdx        = aSimHitCDC->getEnergyDep() * Unit::GeV;
    double hitDriftLength = aSimHitCDC->getDriftLength() * Unit::cm;

    bool ifNewDigi = true;
    // The first SimHit is always a new digit, but the looping will anyhow end immediately.
    for (iterCDCMap = cdcSignalMap.begin(); iterCDCMap != cdcSignalMap.end(); iterCDCMap++) {

      // Check if new SimHit is in cell of existing Signal.
      if ((iterCDCMap->second->getLayerId() == hitLayerId) && (iterCDCMap->second->getWireId() == hitWireId)) {

        // If true, the SimHit doesn't create a new digit, ...
        ifNewDigi = false;

        // ... smallest drift time has to be checked, ...
        if (hitDriftLength < iterCDCMap->second->getDriftLength()) {
          iterCDCMap->second->setDriftLength(hitDriftLength);
          B2DEBUG(150, "hitDriftLength: " << hitDriftLength);
        }

        // ... total charge has to be updated.
        iterCDCMap->second->updateCharge(hitdEdx);

        //A SimHit will not be in more than one cell.
        break;
      }
    } // End loop over previously stored Signal.

    // If it is a new hit, save it to signal map.
    if (ifNewDigi == true) {
      CDCSignal* signal = new CDCSignal(hitLayerId, hitWireId, hitdEdx, hitDriftLength);
      cdcSignalMap.insert(vpair(iHits, signal));
    }
  } // end loop over SimHits.


  //-----------------------------------------------------
  // Smear drift length and save digits into data store.
  //-----------------------------------------------------
  int iDigits = 0;
  // Arrays for CDCHits and Relations between SimHit and CDCHit.
  StoreArray<CDCHit> cdcHitArray(m_cdcHitOutColName);
  StoreArray<Relation> cdcSimRelation(m_relColNameSimHitToHit);

  for (iterCDCMap = cdcSignalMap.begin(); iterCDCMap != cdcSignalMap.end(); iterCDCMap++) {

    // Smear drift length using double  Gaussion, based on spatial resolution.
    double hitNewDriftLength =  smearDriftLength(iterCDCMap->second->getDriftLength(),
                                                 m_fraction, m_mean1, m_resolution1, m_mean2, m_resolution2);
    iterCDCMap->second->setDriftLength(hitNewDriftLength);
    B2DEBUG(150, "NewHitDriftLength " << iterCDCMap->second->getDriftLength());

    // Save digits into data store
    // Next lines are the storage for HitCDC connected to trasan, which will eventually become obsolete.
    StoreArray<HitCDC> NNcdcArray(m_outColName);
    new(NNcdcArray->AddrAt(iDigits)) HitCDC();
    NNcdcArray[iDigits]->setLayerId(iterCDCMap->second->getLayerId());
    NNcdcArray[iDigits]->setWireId(iterCDCMap->second->getWireId());
    NNcdcArray[iDigits]->setLeftDriftLength(iterCDCMap->second->getDriftLength());
    NNcdcArray[iDigits]->setRightDriftLength(iterCDCMap->second->getDriftLength());
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

    // Creation of Relation between SimHit, that has smalles drift length in each cell and the CDCHit.
    B2DEBUG(150, "First: " << (iterCDCMap->first));
    new(cdcSimRelation->AddrAt(iDigits)) Relation(*cdcArray.relateTo(cdcHitArray, iDigits, iterCDCMap->first));

    // Count number of digits
    iDigits++;
  }

  m_nEvent++;
}

double CDCDigi::smearDriftLength(double driftLength, double fraction, double mean1, double resolution1, double mean2, double resolution2)
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
  return newDL*Unit::cm;
}

void CDCDigi::endRun()
{
}

void CDCDigi::terminate()
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

void CDCDigi::genNoise(CDCSignalMap &) //cdcSignalMap)
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

double CDCDigi::getDriftTime(double) //driftLength)
{
  //--------------------------------------------------------------------
  // Method returning electron drift time (parameters: position in cm)
  //--------------------------------------------------------------------
  return 0;
}

void CDCDigi::printSimCDCHitInfo(const SimHitCDC & aHit) const
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

void CDCDigi::printSimCDCHitsInfo(std::string info, const SimHitCDCVec & hitVec) const
{
  //---------------------
  // Printing hits info.
  //---------------------
  SimHitCDCVec::const_iterator iterHits;

  B2INFO("   " << info << ":" << "\n");
  for (iterHits = hitVec.begin(); iterHits != hitVec.end(); iterHits++) {
    printSimCDCHitInfo(**iterHits);
  }
}

void CDCDigi::printModuleParams() const
{
  //-----------------------------
  // Printing module parameters.
  //-----------------------------
  B2INFO(" "
         << DUNDERL
         << DBLUE
         << "CDCDigi parameters:"
         << ENDCOLOR
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

void CDCDigi::printCDCSignalInfo(std::string info, const CDCSignalMap & cdcSignalMap) const
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
