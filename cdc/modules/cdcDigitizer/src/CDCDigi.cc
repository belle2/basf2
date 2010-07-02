/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/modules/cdcDigitizer/CDCDigi.h>
#include <cdc/modules/cdcDigitizer/CDCColours.h>

#include <framework/fwcore/ModuleManager.h>
//#include <boost/format.hpp>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreDefs.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/Units.h>

#include <framework/logging/Logger.h>
#include <cdc/hitcdc/HitCDC.h>

#include <cstdlib>
#include <iomanip>
#include <math.h>
#include <time.h>

using namespace std;
using namespace boost;
using namespace Belle2;

REG_MODULE(CDCDigi)

CDCDigi::CDCDigi() : Module("CDCDigitizer")
{
  // Set description
  setDescription("CDCDigitizer");

  // Add parameters
  addParam("RandomSeed", m_randomSeed, 12345, "Random seed");
  addParam("InputColName", m_inColName, string("SimHitCDCArray"), "Input collection name");
  addParam("OutputColName", m_outColName, string("HitCDCArray"), "Output collection name");
  addParam("SpatialResolution", m_spatialResolution, 0.013, "Spatial resolution of sense wire, set in cm");
  addParam("ElectronicEffects", m_electronicEffects, 0, "Apply electronic effects?");
  addParam("ElectronicsNoise", m_elNoise, 1000.0, "Noise added by the electronics, set in ENC");
  addParam("RelCollectionNameMCToSimHit", m_relColNameMCToSim, string("MCToSimHitCDCRel"), "Name of relation collection - MCParticle to SimCDCHit (if nonzero, created)");
  addParam("RelCollectionNameHitCDCToMC", m_relColNamePlsToMC, string("HitCDCToMCRel"), "Name of relation collection - Hit CDC to MCParticle (if nonzero, created)");
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
  m_spatialResolution  *= cm;

  // Initialize random generator (engine, mean, sigma)
  m_random = new TRandom((UInt_t)m_randomSeed);

  // Print set parameters
  printModuleParams();

  // CPU time start
  m_timeCPU = clock() * us;
}

void CDCDigi::beginRun()
{
  // Print run number
  INFO(" Processing run: "
       << ENDCOLOR
       << m_nRun);

  m_nRun++;
}

void CDCDigi::event()
{
  //------------------------------------------
  // Get CDC hits collection from data store.
  //------------------------------------------
  StoreArray<SimHitCDC> cdcArray(m_inColName);
  if (!cdcArray) {
    ERROR("Can not find " << m_inColName << ", exit.");
    exit(-1);
  }

  //---------------------------------------------------------------------
  // Merge the hits in the same cell and save them into CDC signal map.
  //---------------------------------------------------------------------

  // Get number of hits in this event
  int nHits = cdcArray->GetEntries();

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
    double hitdEdx        = aSimHitCDC->getEnergyDep() * GeV;
    double hitDriftLength = aSimHitCDC->getDriftLength() * cm;

    if (iHits == 0) {
      // Save the first hit into signal map
      CDCSignal* signal = new CDCSignal(hitLayerId, hitWireId, hitdEdx, hitDriftLength);
      cdcSignalMap.insert(vpair(iHits, signal));
    } else {
      bool ifNewDigi = true;
      for (iterCDCMap = cdcSignalMap.begin(); iterCDCMap != cdcSignalMap.end(); iterCDCMap++) {

        // Check if in the same cell
        if ((iterCDCMap->second->getLayerId() == hitLayerId) && (iterCDCMap->second->getWireId() == hitWireId)) {
          ifNewDigi = false;

          // If true, update drift length
          if (hitDriftLength < iterCDCMap->second->getTime()) {
            iterCDCMap->second->setTime(hitDriftLength);
          }

          // Update signal magnitude (add current charge to the total)
          iterCDCMap->second->updateCharge(hitdEdx);

        }
      }

      // If it is a new hit, save it to signal map
      if (ifNewDigi == true) {
        CDCSignal* signal = new CDCSignal(hitLayerId, hitWireId, hitdEdx, hitDriftLength);
        cdcSignalMap.insert(vpair(iHits, signal));
      }
    }
  } // end loop


  //-----------------------------------------------------
  // Smear drift length and save digits into data store.
  //-----------------------------------------------------
  int iDigits = 0;
  for (iterCDCMap = cdcSignalMap.begin(); iterCDCMap != cdcSignalMap.end(); iterCDCMap++) {
    // Smear drift length using single Gaussion, based on spatial resolution.
    double hitNewDriftLength =  smearDriftLength(iterCDCMap->second->getTime());
    iterCDCMap->second->setTime(hitNewDriftLength);

    // Save digits into data store
    StoreArray<HitCDC> cdcArray(m_outColName);
    new(cdcArray->AddrAt(iDigits)) HitCDC();
    cdcArray[iDigits]->setLayerId(iterCDCMap->second->getLayerId());
    cdcArray[iDigits]->setWireId(iterCDCMap->second->getWireId());
    cdcArray[iDigits]->setTime(iterCDCMap->second->getTime());
    cdcArray[iDigits]->setCharge(iterCDCMap->second->getCharge());

    // Count number of digits
    iDigits++;
  }

  m_nEvent++;
}

double CDCDigi::smearDriftLength(double driftLength)
{
  // Smear drift length
  double newDL = m_random->Gaus(driftLength / cm, m_spatialResolution / cm);
  while (newDL <= 0.) newDL = m_random->Gaus(driftLength / cm, m_spatialResolution / cm);
  return newDL*cm;
}

void CDCDigi::endRun()
{
}

void CDCDigi::terminate()
{
  // CPU time end
  m_timeCPU = clock() * us - m_timeCPU;

  if (m_random)  delete m_random;
  // Print message
  //INFO(   " "
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
  double depE = aHit.getEnergyDep() / keV;
  INFO("    Hit:"
       << std::fixed
       << std::setprecision(1)
       << " DepE [keV]: "     << depE
       << std::setprecision(3)
       << std::setiosflags(std::ios::showpos)
       << " Pos X [mm]: " << (aHit.getPosIn()[0] + aHit.getPosOut()[0]) / 2. / mm
       << " Pos Y [mm]: " << (aHit.getPosIn()[1] + aHit.getPosOut()[1]) / 2. / mm
       << " Pos Z [mm]: " << (aHit.getPosIn()[2] + aHit.getPosOut()[2]) / 2. / mm
       << " DriftLength [mm]: "      << aHit.getDriftLength() / mm
       << " Time of flight [ns]: "      << aHit.getFlightTime() / ns
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

  INFO("   " << info << ":" << "\n");
  for (iterHits = hitVec.begin(); iterHits != hitVec.end(); iterHits++) {
    printSimCDCHitInfo(**iterHits);
  }
}

void CDCDigi::printModuleParams() const
{
  //-----------------------------
  // Printing module parameters.
  //-----------------------------
  INFO(" "
       << DUNDERL
       << DBLUE
       << "CDCDigi parameters:"
       << ENDCOLOR
       << " "
       << "\n");

  INFO("  Input collection name:                 " << std::setw(6) << m_inColName << "\n");

  INFO("                                         " << "\n");

  INFO("  Output collection name:                " << std::setw(6) << m_outColName << "\n");

  INFO("                                         " << "\n");

  //INFO(   "  Apply electronic effect?               " << std::setw(6) << m_electronicEffects << "\n" );
  //
  //INFO(   "                                         " << "\n"
  //     << std::setiosflags(std::ios::fixed | std::ios::internal )
  //     << std::setprecision(2)
  //     << "  Electronics noise - ENC [fC]:          " << std::setw(6) << m_elNoise      << "\n");

  INFO("                                         " << "\n"
       << "  Spatial resolution [um]:               " << std::setw(6) << m_spatialResolution / um  << "\n"
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

  INFO("  Digi results - " << info
       << ":");

  // Loop over all digits
  for (iterCDCMap = cdcSignalMap.begin(); iterCDCMap != cdcSignalMap.end(); iterCDCMap++) {
    int layerId = iterCDCMap->second->getLayerId();
    int wireId =  iterCDCMap->second->getWireId();

    INFO("   Layer: " << layerId
         << "   Wire: "  << wireId);

    // cdc signal (digits)
    INFO("                 " << "\n"
         << std::setiosflags(std::ios::fixed | std::ios::internal)
         << std::setprecision(2)
         << " Total charge [keV]: "  << iterCDCMap->second->getCharge() / keV
         << " Drift length [mm]: " << iterCDCMap->second->getTime() / mm
         << std::setprecision(0));
  } // end loop
}
