/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/TrackMatchLookUp.h"

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationEntry.h>
#include <framework/datastore/DataStore.h>

using namespace std;
using namespace Belle2;

ClassImp(TrackMatchLookUp)

// Helper functions in anonymous namespace
namespace {

  // This method should eventually become part of the DataStore
  // In addtion to the method defined there it also yields the weight of the relation
  // If no relation is found the weight is not changed
  template <class TO>
  static TO* getRelatedFromObj(const TObject* fromObject, double& weight, const string& toName = "")
  {
    if (!fromObject) return nullptr;
    DataStore::StoreEntry* storeEntry = nullptr;
    int index = -1;
    RelationEntry toEntry = DataStore::Instance().getRelationWith(DataStore::c_ToSide, fromObject, storeEntry, index, TO::Class(), toName);
    TO* toObject = static_cast<TO*>(toEntry.object);

    if (toObject) weight = toEntry.weight;
    return toObject;
  }


  // Checks if the object is in the StoreArray given by its name.
  template <class T>
  int isInStoreArray(const T& object, const string& storeArrayName = "")
  {
    StoreArray<T> storeArray(storeArrayName);
    TClonesArray* clonesArray = storeArray.getPtr();
    if (clonesArray) {
      Int_t index = clonesArray->IndexOf(&object);
      Int_t lowerBound = clonesArray->LowerBound();
      return index > lowerBound;
    } else {
      return false;
    }
  }
}



TrackMatchLookUp::TrackMatchLookUp(const std::string& mcTrackCandStoreArrayName,
                                   const std::string& prTrackCandStoreArrayName)
  : m_mcTracksStoreArrayName(DataStore::arrayName<genfit::TrackCand>(mcTrackCandStoreArrayName)),
    m_prTracksStoreArrayName(DataStore::arrayName<genfit::TrackCand>(prTrackCandStoreArrayName))
{
  if (m_mcTracksStoreArrayName == m_prTracksStoreArrayName) {
    B2WARNING("Pattern recognition and Monte Carlo track candidate StoreArray are the same.")
  }
}


TrackMatchLookUp::~TrackMatchLookUp()
{
}



bool TrackMatchLookUp::isMCTrackCand(const genfit::TrackCand& trackCand)
{
  return isInStoreArray(trackCand, getMCTracksStoreArrayName());
}



bool TrackMatchLookUp::isPRTrackCand(const genfit::TrackCand& trackCand)
{
  return isInStoreArray(trackCand, getPRTracksStoreArrayName());
}




const genfit::TrackCand* TrackMatchLookUp::getRelatedMCTrackCand(const genfit::TrackCand& prTrackCand, float& purity)
{
  double double_purity = 0; //help variable because DataStore expects double
  const genfit::TrackCand* mcTrackCand = getRelatedFromObj<genfit::TrackCand >(&prTrackCand, double_purity, getMCTracksStoreArrayName());
  if (mcTrackCand) {
    purity = double_purity;
  } else {
    purity = NAN;
  }
  return mcTrackCand;
}





const genfit::TrackCand* TrackMatchLookUp::getRelatedPRTrackCand(const genfit::TrackCand& mcTrackCand, float& efficiency)
{
  double double_efficiency = 0; //help variable because DataStore expects double
  const genfit::TrackCand* prTrackCand = getRelatedFromObj<genfit::TrackCand >(&mcTrackCand, double_efficiency, getPRTracksStoreArrayName());
  if (prTrackCand) {
    efficiency = double_efficiency;
  } else {
    efficiency = NAN;
  }
  return prTrackCand;
}




//Helper functions to figure out the match of the PRParticle
TrackMatchLookUp::MCToPR::MatchInfo
TrackMatchLookUp::extractMCToPRMatchInfo(const genfit::TrackCand* prTrackCand, const float& efficiency)
{
  if (!prTrackCand) return MCToPR::MatchInfo::MISSING;
  if (efficiency < 0) return MCToPR::MatchInfo::MERGED;
  else if (efficiency > 0) return MCToPR::MatchInfo::MATCHED;

  return MCToPR::MatchInfo::INVALID;
}

TrackMatchLookUp::PRToMC::MatchInfo
TrackMatchLookUp::extractPRToMCMatchInfo(const genfit::TrackCand& prTrackCand, const genfit::TrackCand* mcTrackCand, const float& purity)
{

  if (!mcTrackCand) {

    // The patter recognition track has no associated Monte Carlo track.
    // Figure out of it is a clone or a match by the McTrackId property assigned to the track cand.
    // That is also why we need the pattern recogntion track in this method as well.
    int mcTrackIdProperty = prTrackCand.getMcTrackId();

    if (mcTrackIdProperty == -999) return PRToMC::MatchInfo::GHOST;
    else if (mcTrackIdProperty == -99) return PRToMC::MatchInfo::BACKGROUND;
    else if (mcTrackIdProperty == -9) return PRToMC::MatchInfo::CLONE; // MCTrackMatcher is running without RelateClonesToMcParticles
    else if (mcTrackIdProperty < 0) return PRToMC::MatchInfo::INVALID;

  } else {

    // The patter recognition track has an associated Monte Carlo track.
    // Figure out of it is a clone or a match by the sign of the purity.
    if (purity < 0) return PRToMC::MatchInfo::CLONE;
    else if (purity > 0) return PRToMC::MatchInfo::MATCHED;
    else return PRToMC::MatchInfo::INVALID;

  }

  return PRToMC::MatchInfo ::INVALID;
}



const MCParticle*
TrackMatchLookUp::getRelatedMCParticle(const genfit::TrackCand& prTrackCand)
{
  double dummy_weight = NAN;
  std::string mcParticleStoreArrayName = "";
  return getRelatedFromObj<MCParticle>(&prTrackCand, dummy_weight, mcParticleStoreArrayName);
}

const genfit::TrackCand*
TrackMatchLookUp::getRelatedMCTrackCand(const genfit::TrackCand& prTrackCand)
{
  double dummy_weight = NAN;
  return getRelatedFromObj<genfit::TrackCand>(&prTrackCand, dummy_weight, getMCTracksStoreArrayName());
}

const genfit::TrackCand*
TrackMatchLookUp::getRelatedPRTrackCand(const genfit::TrackCand& mcTrackCand)
{
  double dummy_weight = NAN;
  return getRelatedFromObj<genfit::TrackCand>(&mcTrackCand, dummy_weight, getPRTracksStoreArrayName());
}



float TrackMatchLookUp::getRelatedPurity(const genfit::TrackCand& prTrackCand)
{
  float purity = NAN;
  getRelatedMCTrackCand(prTrackCand, purity);
  return std::fabs(purity);
}



float TrackMatchLookUp::getRelatedEfficiency(const genfit::TrackCand& mcTrackCand)
{
  float efficiency = NAN;
  getRelatedPRTrackCand(mcTrackCand, efficiency);
  return std::fabs(efficiency);
}



const genfit::TrackCand*
TrackMatchLookUp::getMatchedMCTrackCand(const genfit::TrackCand& prTrackCand)
{
  float purity = NAN;
  const genfit::TrackCand* mcTrackCand = getRelatedMCTrackCand(prTrackCand, purity);

  if (extractPRToMCMatchInfo(prTrackCand, mcTrackCand, purity) == PRToMC::MATCHED) {
    return mcTrackCand;

  } else {
    return nullptr;

  }
}



const genfit::TrackCand*
TrackMatchLookUp::getMatchedPRTrackCand(const genfit::TrackCand& mcTrackCand)
{
  float efficiency = NAN;
  const genfit::TrackCand* prTrackCand = getRelatedPRTrackCand(mcTrackCand, efficiency);

  if (extractMCToPRMatchInfo(prTrackCand, efficiency) == MCToPR::MATCHED) {
    return prTrackCand;

  } else {
    return nullptr;

  }
}



float TrackMatchLookUp::getMatchedPurity(const genfit::TrackCand& trackCand)
{
  if (isMCTrackCand(trackCand)) {
    const genfit::TrackCand& mcTrackCand = trackCand;
    const genfit::TrackCand* prTrackCand = getMatchedPRTrackCand(mcTrackCand);
    if (prTrackCand) {
      return getRelatedPurity(*prTrackCand);

    } else {
      return NAN;

    }

  } else {
    const genfit::TrackCand& prTrackCand = trackCand;
    return getRelatedPurity(prTrackCand);

  }
}


float TrackMatchLookUp::getMatchedEfficiency(const genfit::TrackCand& trackCand)
{
  if (isPRTrackCand(trackCand)) {
    const genfit::TrackCand& prTrackCand = trackCand;
    const genfit::TrackCand* mcTrackCand = getMatchedMCTrackCand(prTrackCand);
    if (mcTrackCand) {
      return getRelatedEfficiency(*mcTrackCand);

    } else {
      return NAN;

    }

  } else {
    const genfit::TrackCand& mcTrackCand = trackCand;
    return getRelatedPurity(mcTrackCand);

  }


}

