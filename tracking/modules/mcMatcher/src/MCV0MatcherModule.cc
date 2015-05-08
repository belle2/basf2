#include <tracking/modules/mcMatcher/MCV0MatcherModule.h>

#include <framework/datastore/RelationIndex.h>
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

#include "genfit/Track.h"

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
  StoreArray<Track>::required(m_TrackColName);
  StoreArray<TrackFitResult>::required(m_TFRColName);
  StoreArray<V0>::required(m_V0ColName);
  StoreArray<MCParticle>::required(m_MCParticleColName);

  StoreArray<V0> V0s(m_V0ColName);
  StoreArray<MCParticle> mcParticles(m_MCParticleColName);
  V0s.registerRelationTo(mcParticles);
}


void MCV0MatcherModule::beginRun()
{
}

void MCV0MatcherModule::event()
{
  StoreArray<V0> v0s;
  B2DEBUG(200, (v0s.getEntries() != 0 ? "V0s has entries." : " No V0s."));

  for (auto& v0 : v0s) {
    // Try to match the tracks of each V0 with the MC V0.
    auto v0hypothesis = v0.getV0Hypothesis();
    auto trackPtrs = v0.getTracks();

    const auto V0PartPlus = trackPtrs.first->getRelatedTo<MCParticle>();
    const auto V0PartMinus = trackPtrs.second->getRelatedTo<MCParticle>();

    if (V0PartPlus == nullptr or V0PartMinus == nullptr) {
      B2DEBUG(200, "At least one track of the V0 does not have a MC related particle. It will be skipped for matching.");
      continue;
    }

    const auto V0PartPlusMotherParticleType = Const::ParticleType(V0PartPlus->getMother()->getPDG());
    const auto V0PartMinusMotherParticleType = Const::ParticleType(V0PartMinus->getMother()->getPDG());

    if (V0PartPlusMotherParticleType != V0PartMinusMotherParticleType) {
      B2DEBUG(200, "The V0 is most likely built up from combinatoric background, thus no MC relation can be set.");
      continue;
    }

    if (V0PartPlusMotherParticleType == v0hypothesis) {
      B2DEBUG(200, "V0 successfully matched.");
      v0.addRelationTo(V0PartPlus->getMother());
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
