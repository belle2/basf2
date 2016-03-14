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
#include <mdst/dataobjects/Track.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>
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
  setDescription("E/p based Electron ID. Likelihood values for each particle hypothesis are stored in an ECLPidLikelihood object.");
  setPropertyFlags(c_ParallelProcessingCertified);
  for (unsigned int i = 0; i < Const::ChargedStable::c_SetSize; i++) m_pdf[i] = 0;
}


ECLElectronIdModule::~ECLElectronIdModule()
{
}


void ECLElectronIdModule::initialize()
{
  StoreArray<Track> tracks;
  StoreArray<ECLPidLikelihood> eclPidLikelihoods;
  eclPidLikelihoods.registerInDataStore();
  tracks.registerRelationTo(eclPidLikelihoods);

  const string eParams = FileSystem::findFile("/data/ecl/electrons.dat");
  const string muParams = FileSystem::findFile("/data/ecl/muons.dat");
  const string piParams = FileSystem::findFile("/data/ecl/pions.dat");

  if (eParams.empty()  || muParams.empty() || piParams.empty())
    B2FATAL("Electron ID pdfs parameter files not found.");

  (m_pdf[Const::electron.getIndex()] = new ECLElectronPdf)->init(eParams.c_str());
  (m_pdf[Const::muon.getIndex()] = new ECLMuonPdf)->init(muParams.c_str());
  (m_pdf[Const::proton.getIndex()] =
     m_pdf[Const::kaon.getIndex()] =
       m_pdf[Const::pion.getIndex()] = new ECLPionPdf)->init(piParams.c_str());
}

void ECLElectronIdModule::beginRun()
{
}

void ECLElectronIdModule::event()
{
  StoreArray<Track> tracks;
  StoreArray<ECLPidLikelihood> eclPidLikelihoods;

  for (const auto& track : tracks) {
    const TrackFitResult* fitRes = track.getTrackFitResult(Const::pion);
    if (fitRes == nullptr) continue;
    const auto relShowers = track.getRelationsTo<ECLShower>();
    if (relShowers.size() == 0) continue;

    const double p = fitRes->getMomentum().Mag();
    const double costheta = fitRes->getMomentum().CosTheta();
    double energy = 0;
    double maxEnergy = 0;
    double e9e25 = 0;
    int nCrystals = 0;
    int nClusters = relShowers.size();

    for (const auto& eclShower : relShowers) {
      const double shEnergy = eclShower.getEnergy();
      energy += shEnergy;
      if (shEnergy > maxEnergy) {
        maxEnergy = shEnergy;
        e9e25 = eclShower.getE9oE25();
      }
      nCrystals += int(eclShower.getNHits());
    }

    float likelihoods[Const::ChargedStable::c_SetSize];
    double eop = energy / p;

    for (const auto& hypo : Const::chargedStableSet) {
      ECLAbsPdf* currentpdf = m_pdf[hypo.getIndex()];
      if (currentpdf == 0) {
        currentpdf = m_pdf[Const::pion.getIndex()]; // use pion pdf when specialized pdf is not assigned.
      }
      double pdfval = currentpdf->pdf(eop, p, costheta);
      if (isnormal(pdfval) && pdfval > 0) likelihoods[hypo.getIndex()] = log(pdfval);
      else likelihoods[hypo.getIndex()] = m_minLogLike;
    } // end loop on hypo

    const auto eclPidLikelihood = eclPidLikelihoods.appendNew(likelihoods, energy, eop, e9e25, nCrystals, nClusters);
    track.addRelationTo(eclPidLikelihood);

  } // end loop on tracks
}

void ECLElectronIdModule::endRun()
{
}

void ECLElectronIdModule::terminate()
{
  delete m_pdf[Const::electron.getIndex()];
  delete m_pdf[Const::muon.getIndex()];
  delete m_pdf[Const::pion.getIndex()];
  if (m_pdf[Const::proton.getIndex()] != m_pdf[Const::pion.getIndex()]) delete m_pdf[Const::proton.getIndex()];
  if (m_pdf[Const::kaon.getIndex()] != m_pdf[Const::pion.getIndex()]) delete m_pdf[Const::kaon.getIndex()];
}
