/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <alignment/modules/UpdateParticleTrackCand/UpdateParticleTrackCandModule.h>

#include <analysis/dataobjects/ParticleList.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/geometry/B2Vector3.h>
#include <mdst/dataobjects/Track.h>

#include <genfit/TrackCand.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(UpdateParticleTrackCand)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

UpdateParticleTrackCandModule::UpdateParticleTrackCandModule() : Module()
{
  // Set module properties
  setDescription("Updates the seed in TrackCand based on fitted state (at vertex)");

  // Parameter definitions
  addParam("motherListNames", m_motherListNames, "Names of particle list which daughters have to be updated");

  addParam("removePXD", m_removePXD, "Remove PXD hits from TrackCand?", bool(false));
  addParam("removeSVD", m_removeSVD, "Remove SVD hits from TrackCand?", bool(false));
  addParam("removeCDC", m_removeCDC, "Remove CDC hits from TrackCand?", bool(false));
  addParam("removeBKLM", m_removeBKLM, "Remove BKLM hits from TrackCand?", bool(false));
}

void UpdateParticleTrackCandModule::initialize()
{
}

void UpdateParticleTrackCandModule::event()
{
  for (auto listName : m_motherListNames) {
    StoreObjPtr<ParticleList> list(listName);
    auto listSize = list->getListSize(false);
    for (unsigned int iParticle = 0; iParticle < listSize; ++iParticle) {
      auto mother = list->getParticle(iParticle, false);
      for (auto daughter : mother->getDaughters()) {
        auto particle = daughter;
        auto track = particle->getTrack();
        auto trackFitResult = track->getTrackFitResult(Const::pion);
        if (not trackFitResult) {
          B2WARNING("Skipping track without valid pion hypothesis.");
          continue;
        }
        auto trackCand = trackFitResult->getRelatedFrom<genfit::TrackCand>();

        if (m_removePXD || m_removeSVD || m_removeCDC || m_removeBKLM) {
          genfit::TrackCand copyCand(*trackCand);
          trackCand->reset();
          for (unsigned int iCandHit = 0; iCandHit < copyCand.getNHits(); ++iCandHit) {
            int hitID, detID;
            double sortingParameter;
            copyCand.getHit(iCandHit, detID, hitID, sortingParameter);

            if (m_removePXD && detID == Const::PXD) continue;
            if (m_removeSVD && detID == Const::SVD) continue;
            if (m_removeCDC && detID == Const::CDC) continue;
            if (m_removeBKLM && detID == Const::KLM) continue;

            trackCand->addHit(detID, hitID, -1, sortingParameter);
          }
        }

        B2Vector3D vertexPos = particle->getVertex();
        B2Vector3D vertexMom = particle->getMomentum();
        trackCand->setPosMomSeed(vertexPos, vertexMom, particle->getCharge());
        trackCand->setPdgCode(particle->getPDGCode());
      }
    }
  }
}



