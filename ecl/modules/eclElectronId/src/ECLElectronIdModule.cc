/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guglielmo De Nardo                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <ecl/modules/eclElectronId/ECLElectronIdModule.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLPidLikelihood.h>
#include <framework/datastore/RelationArray.h>
#include <tracking/dataobjects/Track.h>
#include <framework/datastore/RelationIndex.h>
#include <G4ParticleTable.hh>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <ecl/electronId/ECLMuonPdf.h>
#include <ecl/electronId/ECLElectronPdf.h>
#include <ecl/electronId/ECLPionPdf.h>
using namespace std;
using namespace Belle2;


//this line registers the module with the framework and actually makes it available
//in steering files or the the module list (basf2 -m).
//Note that the 'Module' part of the class name is missing, this is also the way it
//will be called in the module list.
REG_MODULE(ECLElectronId)

ECLElectronIdModule::ECLElectronIdModule() : Module()
{
  setDescription("First version of E/p based Electron ID. Likelihood values for each particle hypothesis are stored in an ECLPidLikelihood object.");

  //add module parameters here [see one of the following tutorials]
}


ECLElectronIdModule::~ECLElectronIdModule()
{
}


void ECLElectronIdModule::initialize()
{
  StoreArray<ECLPidLikelihood>::registerPersistent();
  RelationArray::registerPersistent<Track, ECLPidLikelihood>();

  (m_pdf[ Const::electron.getIndex() ] = new ECLElectronPdf) -> init();
  (m_pdf[ Const::muon.getIndex() ] = new ECLMuonPdf) ->init();
  (m_pdf[ Const::proton.getIndex() ] =
     m_pdf[ Const::kaon.getIndex() ] =
       m_pdf[ Const::pion.getIndex() ] = new ECLPionPdf) ->init();
}

void ECLElectronIdModule::beginRun()
{
}

void ECLElectronIdModule::event()
{
  StoreArray<Track> Tracks;
  StoreArray<ECLPidLikelihood> ecllogL;

  for (int t = 0; t < Tracks.getEntries(); ++t) {
    const Track* track = Tracks[t];
    auto relShowers = track->getRelationsTo<ECLShower>();
    double energy = 0;
    for (auto sh : relShowers) energy += sh.GetEnergy();

    float likelihoods [ Const::ChargedStable::c_SetSize ] ;
    for (Const::ChargedStable hypo = Const::chargedStableSet.begin();
         hypo != Const::chargedStableSet.end() ; ++hypo) {

      const TrackFitResult* fitRes = track -> getTrackFitResult(hypo);
      if (fitRes == 0) fitRes = track -> getTrackFitResult(Const::pion);

      if (fitRes != 0) {
        double p = fitRes  -> getMomentum() . Mag();
        double eop = energy / p;
        likelihoods[hypo.getIndex()] = m_pdf[hypo.getIndex()]->pdf(eop, p);
      } else likelihoods[hypo.getIndex()] = 1.e-20;

    } // end loop on hypo

    track-> addRelationTo(ecllogL.appendNew(ECLPidLikelihood(likelihoods)));
  } // end loop on Tracks
}

void ECLElectronIdModule::endRun()
{
}

void ECLElectronIdModule::terminate()
{
}
