/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka, Martin Ritter                              *
 *                                                                        *
 **************************************************************************/

#include <testbeam/vxd/modules/VXDHitModule.h>

#include <framework/logging/Logger.h>

#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <testbeam/vxd/dataobjects/VXDHit.h>
#include <generators/dataobjects/MCParticle.h>
#include <framework/datastore/RelationArray.h>


using namespace std;
using namespace Belle2;
using namespace Belle2::TB;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(VXDHit)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

VXDHitModule::VXDHitModule() : Module()
{
  //Set module properties
  setDescription("Module to analyze PXD+SVD TrueHits and primary particles");
}

void VXDHitModule::initialize()
{
  StoreArray<VXDHit>::registerPersistent();
}

void VXDHitModule::beginRun()
{

}

void VXDHitModule::event()
{
  StoreArray<MCParticle> particles;

  StoreArray<PXDTrueHit> pxdhits;
  StoreArray<SVDTrueHit> svdhits;
  StoreArray<VXDHit> hits;

  const unsigned int numPXDHits = pxdhits.getEntries();
  const unsigned int numSVDHits = svdhits.getEntries();

  for (unsigned int i = 0; i < numPXDHits; i++) {
    bool isFromPrimary = false;
    int pdg = 0;
    double pMomentum = 0.0;
    int nPrimary = 0;

    const PXDTrueHit* pxdhit = pxdhits[i];

    RelationVector<MCParticle> mcs = pxdhit->getRelationsFrom<MCParticle>();
    pdg = mcs[0]->getPDG();
    pMomentum = double(mcs[0]->getMomentum().Mag());
    for (unsigned int j = 0; j < mcs.size(); j++) {
      if (mcs[j]->getStatus() & MCParticle::c_PrimaryParticle) {
        isFromPrimary = true;
        nPrimary++;
      }
    }
    VXDHit* hit = hits.appendNew(
                    VXDHit(
                      pxdhit->getSensorID(),
                      pxdhit->getU(),
                      pxdhit->getV(),
                      pxdhit->getEntryU(),
                      pxdhit->getEntryV(),
                      pxdhit->getExitU(),
                      pxdhit->getExitV(),
                      pxdhit->getEnergyDep(),
                      pxdhit->getGlobalTime(),
                      pxdhit->getMomentum(),
                      pxdhit->getEntryMomentum(),
                      pxdhit->getExitMomentum()));

    hit->setFromPrimary(isFromPrimary);
    hit->setPdg(pdg);
    hit->setParticleMomentum(pMomentum);
    hit->setNumParticles(mcs.size());
    hit->setNumPrimary(nPrimary);

  }

  for (unsigned int i = 0; i < numSVDHits; i++) {
    bool isFromPrimary = false;
    int pdg = 0;
    double pMomentum = 0.0;
    int nPrimary = 0;
    const SVDTrueHit* svdhit = svdhits[i];

    RelationVector<MCParticle> mcs = svdhit->getRelationsFrom<MCParticle>();
    pdg = mcs[0]->getPDG();
    pMomentum = double(mcs[0]->getMomentum().Mag());
    for (unsigned int j = 0; j < mcs.size(); j++) {
      if (mcs[j]->getStatus() & MCParticle::c_PrimaryParticle) {
        isFromPrimary = true;
        nPrimary++;

      }
    }

    VXDHit* hit = hits.appendNew(
                    VXDHit(
                      svdhit->getSensorID(),
                      svdhit->getU(),
                      svdhit->getV(),
                      svdhit->getEntryU(),
                      svdhit->getEntryV(),
                      svdhit->getExitU(),
                      svdhit->getExitV(),
                      svdhit->getEnergyDep(),
                      svdhit->getGlobalTime(),
                      svdhit->getMomentum(),
                      svdhit->getEntryMomentum(),
                      svdhit->getExitMomentum()));

    hit->setFromPrimary(isFromPrimary);
    hit->setPdg(pdg);
    hit->setParticleMomentum(pMomentum);
    hit->setNumParticles(mcs.size());
    hit->setNumPrimary(nPrimary);
  }
}


void VXDHitModule::terminate()
{

}
