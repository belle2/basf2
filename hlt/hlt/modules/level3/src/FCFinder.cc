/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <fstream>

#include "skim/hlt/modules/level3/FCFinder.h"
#include "skim/hlt/modules/level3/FCCluster.h"

#include "ecl/geometry/ECLGeometryPar.h"
#include "framework/datastore/StoreArray.h"
#include "framework/logging/Logger.h"
#include "ecl/dataobjects/ECLDigit.h"
//#include "ecl/dataobjects/ECLSimHit.h"

using namespace Belle2;

//...Globals...
FCFinder*
FCFinder::s_cFinder = NULL;

const int
FCCrystal::m_phiMaxFE[13] = {47, 47, 63, 63, 63, 95, 95, 95, 95, 95, 95, 143, 143};

const int
FCCrystal::m_phiMaxBE[10] = {143, 143, 95, 95, 95, 95, 95, 63, 63, 63};

FCFinder&
FCFinder::instance(void)
{
  if (!s_cFinder) s_cFinder = new FCFinder();
  return *s_cFinder;
}

FCFinder::FCFinder()
  : m_crystal(NULL),
    m_ehits(*(new FTList<FCCrystal * >(500))),
    m_clusters(*(new FTList<FCCluster * >(20)))
{
}

void
FCFinder::init()
{
  ECL::ECLGeometryPar& geom = *ECL::ECLGeometryPar::Instance();

  static const int nCrystals(8736);

  if (!m_crystal) m_crystal = new FCCrystal[nCrystals + 1];

  int nPhiRing[69];
  int n = 0;
  int thetaId_save = 0;

  for (int cellId = 0; cellId < nCrystals; cellId++) {
    geom.Mapping(cellId);
    const int thetaId = geom.GetThetaID();
    if (thetaId != thetaId_save) {
      nPhiRing[thetaId_save] = n;
      n = 0;
    }
    thetaId_save = thetaId;
    n++;
  }
  nPhiRing[thetaId_save] = n;

  for (int cellId = 0; cellId < nCrystals; cellId++) {
    geom.Mapping(cellId);
    const int thetaId = geom.GetThetaID();
    const int phiId = geom.GetPhiID();

    FCCrystal* minusPhi = m_crystal + nCrystals;
    n = nPhiRing[thetaId];
    if (thetaId && thetaId != 13 && thetaId != 59) {
      minusPhi = m_crystal;
      int nPhiRingMinus = nPhiRing[thetaId - 1];
      int PhiRingMinusPhiId  = (phiId * nPhiRingMinus) / n;
      int remnant = (phiId * nPhiRingMinus) % n;
      if ((remnant << 1) >= n) PhiRingMinusPhiId++;
      minusPhi += (cellId - phiId - (nPhiRingMinus - PhiRingMinusPhiId));
    }
    FCCrystal* plusPhi = m_crystal + nCrystals;
    if (thetaId != 12 && thetaId != 58 && thetaId != 68) {
      plusPhi = m_crystal;
      int nPhiRingPlus = nPhiRing[thetaId + 1];
      int PhiRingPlusPhiId  = (phiId * nPhiRingPlus) / n;
      int remnant = (phiId * nPhiRingPlus) % n;
      if ((remnant << 1) >= n) PhiRingPlusPhiId++;
      plusPhi += (cellId + (n - phiId) + PhiRingPlusPhiId);
    }
    new(m_crystal + cellId) FCCrystal(thetaId, phiId, plusPhi, minusPhi);
    //B2INFO("crystal " << getCellId(m_crystal+cellId) << " plus:" << getCellId(plusPhi) << " minus:" << getCellId(minusPhi) << " theta:" << thetaId << " phi:" << phiId);
  }
  // make virtual cell object for the pointer of boundary's neighbor
  new(m_crystal + nCrystals) FCCrystal();

}

void
FCFinder::term()
{
  clear();
  delete &m_ehits;
  delete &m_clusters;
  if (m_crystal) delete[] m_crystal;
}

void
FCFinder::beginRun()
{
}

void
FCFinder::event(const double seedThreshold, const double clusterECut)
{
  //--
  // clear old information
  //--
  clear();

  //--
  // update ehits information
  //--
  updateEcl3();

  //--
  // clustering
  //--
  clustering(seedThreshold, clusterECut);
}


void
FCFinder::updateEcl3(void)
{

  StoreArray<ECLDigit> ECLDigits;
  //StoreArray<ECLSimHit> ECLSimHits;
  if (!ECLDigits) {
    B2WARNING("no ECLDigit");
  }

  const int nHits = ECLDigits.getEntries();
  //const int nSimHits = ECLSimHits.getEntries();
  //double simESum = 0.;
  for (int i = 0; i < nHits; i++) {
    ECLDigit& h = * ECLDigits[i];
    double energy = (h.getAmp()) / 20000.;
    int cellId = h.getCellId() - 1;
    if (cellId < 0 || cellId > 8735) continue;
    /*
    double simE = 0.;
    for (int j = 0; j < nSimHits; j++){
      if (ECLSimHits[j]->getCellId() - 1 == cellId){
    simE += ECLSimHits[j]->getEnergyDep();
    simESum += ECLSimHits[j]->getEnergyDep();
      }
    }
    if (energy > 0.1) B2INFO("Id = " << cellId << ", E=" << energy << " , simE=" << simE);
    */
    FCCrystal& c = *(m_crystal + cellId);
    c.energy(energy);
    m_ehits.append(&c);
    c.state(FCCrystal::EHit);
  }
  //B2INFO("simESum = " << simESum);
}

void
FCFinder::clear(void)
{
  m_ehits.clear();
  m_clusters.deleteAll();
}


// reconstruct energy of Connected Region
void
FCFinder::clustering(const double seedThreshold, const double clusterECut)
{
  //                +---+---+---+
  //                | 5 | 2 | 8 |
  //                +---+---+---+   phi
  //  Neighbor ID   | 3 | * | 6 |    ^
  //                +---+---+---+    |
  //                | 4 | 1 | 7 |    +--> theta
  //                +---+---+---+
  //
  if (!m_ehits.length()) return;
  ECL::ECLGeometryPar& geom = *ECL::ECLGeometryPar::Instance();

  FTList<FCCrystal*>* hits = new FTList<FCCrystal*>(25);
  FCCrystal** const last = m_ehits.lastPtr();
  FCCrystal** hptr = m_ehits.firstPtr();
  do {      // cluster loop
    if ((**hptr).state()) continue;
    FCCrystal* seed = *hptr;
    double seed_energy = seed->energy();
    double clusterEnergy = 0;
    hits->append(seed);
    for (int i = 0; i ^ hits->length(); i++) { // loop over hits in a cluster
      FCCrystal* const ehit = (*hits)[i];
      const unsigned int state = ehit->state();
      const double energy = ehit->energy();
      clusterEnergy += energy;
      if (energy > seed_energy) { // seed cell search
        seed = ehit;
        seed_energy = energy;
      }
      if ((state & FCCrystal::Neighbor) == FCCrystal::Neighbor) continue;

      if (!(state & FCCrystal::Neighbor1)) {  // check neighbor1
        ehit->neighborPlusPhi()->checkAndAppend(*hits,
                                                FCCrystal::AppendedNeighbor1);
      }
      if (!(state & FCCrystal::Neighbor2)) {  // check neighbor2
        ehit->neighborMinusPhi()->checkAndAppend(*hits,
                                                 FCCrystal::AppendedNeighbor2);
      }
      if (!(state & FCCrystal::Neighbor3)) {  // check neighbor3
        ehit->neighborMinusTheta()->checkAndAppend(*hits,
                                                   FCCrystal::AppendedNeighbor3);
      }
      if (!(state & FCCrystal::Neighbor6)) {  // check neighbor6
        ehit->neighborPlusTheta()->checkAndAppend(*hits,
                                                  FCCrystal::AppendedNeighbor6);
      }
      ehit->state(FCCrystal::CheckedOrNotHit);
    }
    // discard bad clusters
    if (seed_energy < seedThreshold || clusterEnergy < clusterECut) {
      hits->clear();
      continue;
    }

    // cluster energy correction here
    //if (clusterEnergy > 0.15) clusterEnergy += 0.048*(clusterEnergy - 0.15);

    m_clusters.append(new FCCluster(hits, clusterEnergy, seed,
                                    geom.GetCrystalPos(getCellId(seed))));
    hits = new FTList<FCCrystal*>(25);

  } while ((hptr++) != last);
  delete hits;
}
