#include <tracking/modules/mcMatcher/MCV0MatcherModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/V0.h>

#include <TMath.h>
#include <TLorentzVector.h>

#include <genfit/Track.h>

using namespace Belle2;

REG_MODULE(MCV0Matcher);

MCV0MatcherModule::MCV0MatcherModule() : Module()
{
  setDescription("A module matching the V0s found by the V0Finder with MCParticles");
  setPropertyFlags(c_ParallelProcessingCertified);

  //input
  addParam("TrackColName", m_TrackColName,
           "Belle2::Track collection name (input).  Note that the V0s use "
           "pointers indices into these arrays, so all hell may break loose "
           "if you change this.", std::string(""));
  addParam("TFRColName", m_TFRColName,
           "Belle2::TrackFitResult collection name (input).  Note that the V0s "
           "use pointers indices into these arrays, so all hell may break loose "
           "if you change this.", std::string(""));
  addParam("V0ColName", m_V0ColName, "V0 collection name (input)", std::string(""));
  addParam("MCParticleColName", m_MCParticleColName, "MCParticle collection name (input)", std::string(""));
}


MCV0MatcherModule::~MCV0MatcherModule()
{
}


void MCV0MatcherModule::initialize()
{
  StoreArray<Track> tracks(m_TrackColName);
  tracks.isRequired();

  StoreArray<TrackFitResult> trackFitResults(m_TFRColName);
  trackFitResults.isRequired();

  StoreArray<V0> v0s(m_V0ColName);
  v0s.isRequired();

  StoreArray<MCParticle> mcParticles(m_MCParticleColName);
  mcParticles.isRequired();
  v0s.registerRelationTo(mcParticles);
  B2WARNING("This module is supposed to be used only for the debugging\n \
            of the V0Finder module, not as MC matching during analysis.");
}


void MCV0MatcherModule::beginRun()
{
}

void MCV0MatcherModule::event()
{
  StoreArray<V0> v0s(m_V0ColName);
  B2DEBUG(200, (v0s.getEntries() != 0 ? "V0s has entries." : " No V0s."));

  for (const auto& v0 : v0s) {
    // Try to match the tracks of each V0 with the MC V0.
    const Const::ParticleType v0hypothesis = v0.getV0Hypothesis();
    const std::pair<Track*, Track*> trackPtrs = v0.getTracks();

    const MCParticle* mcV0PartPlus = trackPtrs.first->getRelatedTo<MCParticle>(m_MCParticleColName);
    const MCParticle* mcV0PartMinus = trackPtrs.second->getRelatedTo<MCParticle>(m_MCParticleColName);

    if (mcV0PartPlus == nullptr or mcV0PartMinus == nullptr) {
      B2DEBUG(200, "At least one track of the V0 does not have a MC related particle. It will be skipped for matching.");
      continue;
    }

    const MCParticle* mcV0PartPlusMother = mcV0PartPlus->getMother();
    const MCParticle* mcV0PartMinusMother = mcV0PartMinus->getMother();

    if (!mcV0PartPlus->getMother() or !mcV0PartMinusMother) {
      B2DEBUG(200, "At least one track of the V0 does not have a mother MCParticle, skipping.");
      continue;
    }

    if (mcV0PartPlusMother != mcV0PartMinusMother) {
      B2DEBUG(200, "The V0 is most likely built up from combinatoric background, thus no MC relation can be set.");
      continue;
    }

    if (Const::ParticleType(mcV0PartPlusMother->getPDG()) == v0hypothesis) {
      B2DEBUG(200, "V0 successfully matched.");
      v0.addRelationTo(mcV0PartPlusMother);
    } else {
      B2DEBUG(200, "V0 did not match anything.");
    }
  }

  B2DEBUG(200, "MC matching finished.");
}

void MCV0MatcherModule::endRun()
{
}

void MCV0MatcherModule::terminate()
{
}
