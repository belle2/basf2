/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/mcTrackMatcher/MCTrackMatcherModule.h>

#include <framework/dataobjects/EventMetaData.h>

//datastore types
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>

#include <framework/gearbox/Const.h>

#include <mdst/dataobjects/MCParticle.h>
#include <genfit/TrackCand.h>
#include <genfit/TrackCandHit.h>

//hit types
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <cdc/dataobjects/CDCHit.h>

#include <time.h>
#include <map>

#include <Eigen/Dense>

namespace {
  //small anonymous helper construct making convering a pair of iterators usable
  //with range based for
  template<class Iter>
  struct iter_pair_range : std::pair<Iter, Iter> {
    iter_pair_range(std::pair<Iter, Iter> const& x) : std::pair<Iter, Iter>(x) {}
    Iter begin() const {return this->first;}
    Iter end()   const {return this->second;}
  };

  template<class Iter>
  inline iter_pair_range<Iter> as_range(std::pair<Iter, Iter> const& x)
  {
    return iter_pair_range<Iter>(x);
  }

}


using namespace std;
using namespace Belle2;

REG_MODULE(MCTrackMatcher);


MCTrackMatcherModule::MCTrackMatcherModule() : Module()
{


  setDescription("Performs patter recognition in the CDC based on local hit following");

  //Parameter definition
  // Inputs
  addParam("PRGFTrackCandsColName",
           m_param_prGFTrackCandsColName,
           "Name of the collection containing the tracks as generate a patter recognition algorithm to be evaluated ",
           string(""));

  addParam("MCGFTrackCandsColName",
           m_param_mcGFTrackCandsColName,
           "Name of the collection containing the reference tracks as generate by a Monte-Carlo-Tracker (e.g. MCTrackFinder)",
           string("MCGFTrackCands"));

  // Hit content to be evaluated
  //  Select if hit clusters should be used from PXD/SVD
  addParam("UseClusters", m_param_useClusters, "Set true if you want to use PXD/SVD clusters instead of PXD/SVD trueHits", bool(true));

  //  Choose which hits to use, all hits assigned to the track candidate will be used in the fit
  addParam("UsePXDHits", m_param_usePXDHits, "Set true if PXDHits or PXDClusters should be used", bool(true));
  addParam("UseSVDHits", m_param_useSVDHits, "Set true if SVDHits or SVDClusters should be used", bool(true));
  addParam("UseCDCHits", m_param_useCDCHits, "Set true if CDCHits should be used", bool(true));

  addParam("MinimalPurity",
           m_param_minimalPurity,
           "Minimal purity of a PRTrack to be considered matchable to a MCTrack. "
           "This number encodes how many correct hits are minimally need to compensate for a false hits. "
           "The default 2.0/3.0 suggests that for each background hit can be compensated by two correct hits.",
           2.0 / 3.0);


  // PRTracks purity minimal constrains
  /*addParam("MinimalExcessNDF",
     m_param_minimalExcessNdf,
     "Minimum number of correct of degrees of freedom in the PRtrack. 2D hits count as 2. "
     "The number can be given to insure that a minimum of correct degress of freedom "
     "is in the track for the fit as well as the rest of the correct hits is able to "
     "compensate for false hits. Essentially this number is subtracted from the number "
     "of signal hits before calculating the purity.",
     5);*/


  addParam("RelateClonesToMCParticles",
           m_param_relateClonesToMCParticles,
           "Indicates whether the mc matching to MCParticles should also govern clone tracks",
           bool(true));

  //Set up the standard number degress of freedom a hit contributes by detector
  m_ndf_by_detId.clear();
  m_ndf_by_detId[Const::PXD] = 2;
  m_ndf_by_detId[Const::SVD] = 2;
  m_ndf_by_detId[Const::CDC] = 1;

}

MCTrackMatcherModule::~MCTrackMatcherModule()
{
}

void MCTrackMatcherModule::initialize()
{

  // Require both GFTrackCand arrays to be present in the DataStore
  StoreArray<genfit::TrackCand>::required(m_param_prGFTrackCandsColName);
  StoreArray<genfit::TrackCand>::required(m_param_mcGFTrackCandsColName);
  StoreArray<MCParticle>::required("");


  // Actually retrieve the StoreArrays
  StoreArray<genfit::TrackCand> storePRTrackCands(m_param_prGFTrackCandsColName);
  StoreArray<genfit::TrackCand> storeMCTrackCands(m_param_mcGFTrackCandsColName);
  StoreArray<MCParticle> storeMCParticles;


  // Extract the default names for the case empty stings were given
  m_param_prGFTrackCandsColName = storePRTrackCands.getName();
  m_param_mcGFTrackCandsColName = storeMCTrackCands.getName();



  //Purity relation - for each PRTrack to store the purest MCTrack
  storeMCTrackCands.registerRelationTo(storePRTrackCands);
  //Efficiency relation - for each MCTrack to store the most efficient PRTrack
  storePRTrackCands.registerRelationTo(storeMCTrackCands);
  //MC matching relation
  storePRTrackCands.registerRelationTo(storeMCParticles);


  //Require the hits or clusters in case they should be used
  //PXD
  if (m_param_usePXDHits) {
    if (m_param_useClusters) {
      StoreArray<PXDCluster>::required();
    } else {
      StoreArray<PXDTrueHit>::required();
    }
  }

  //SVD
  if (m_param_useSVDHits) {
    if (m_param_useClusters) {
      StoreArray<SVDCluster>::required();
    } else {
      StoreArray<SVDTrueHit>::required();
    }
  }

  //CDC
  if (m_param_useCDCHits) {
    StoreArray<CDCHit>::required();
  }


}

void MCTrackMatcherModule::beginRun()
{

}

void MCTrackMatcherModule::event()
{
  B2DEBUG(100, "########## MCTrackMatcherModule ############");



  //Fetch store array
  StoreArray<genfit::TrackCand> prGFTrackCands(m_param_prGFTrackCandsColName);
  StoreArray<genfit::TrackCand> mcGFTrackCands(m_param_mcGFTrackCandsColName);
  StoreArray<MCParticle> mcParticles("");

  int nMCTrackCands = mcGFTrackCands.getEntries();
  int nPRTrackCands = prGFTrackCands.getEntries();

  B2DEBUG(100, "Number patter recognition tracks is " << nPRTrackCands);
  B2DEBUG(100, "Number Monte-Carlo tracks is " << nMCTrackCands);



  //### Build a detector_id hit_id to trackcand map for easier lookup later ###
  typedef int DetId;
  typedef int HitId;
  typedef int TrackCandId;

  std::map< pair< DetId, HitId>, TrackCandId > mcTrackCandId_by_hitId;

  {
    std::map< pair< DetId, HitId>, TrackCandId>::iterator itMCInsertHint = mcTrackCandId_by_hitId.end();
    TrackCandId mcTrackCandId = -1;

    for (const genfit::TrackCand & mcTrackCand : mcGFTrackCands) {
      ++mcTrackCandId;
      const unsigned int nHits = mcTrackCand.getNHits();

      for (unsigned int iHit = 0; iHit < nHits; ++iHit) {
        const genfit::TrackCandHit* hit = mcTrackCand.getHit(iHit);
        const HitId hitId = hit->getHitId();
        const DetId detId = hit->getDetId();

        itMCInsertHint = mcTrackCandId_by_hitId.insert(itMCInsertHint, make_pair(pair<DetId, HitId>(detId, hitId), mcTrackCandId));

        // equivalent but not yet in the standard library ?
        //itMCInsertHint = mcTrackCandId_by_hitId.emplace_hint(itMCInsertHint, pair<DetId, HitId>(detId, hitId), iHit);
      }
    }
  }





  //  Use multimap here in case hits are assigned to more than one patter recognition track
  std::multimap< pair< DetId, HitId >, TrackCandId> prTrackCandId_by_hitId;

  {
    TrackCandId prTrackCandId = -1;
    std::multimap< pair< DetId, HitId>, TrackCandId>::iterator itPRInsertHint = prTrackCandId_by_hitId.end();

    for (const genfit::TrackCand & prTrackCand : prGFTrackCands) {
      ++prTrackCandId;
      const unsigned int nHits = prTrackCand.getNHits();

      for (unsigned int iHit = 0; iHit < nHits; ++iHit) {
        const genfit::TrackCandHit* hit = prTrackCand.getHit(iHit);
        const HitId hitId = hit->getHitId();
        const DetId detId = hit->getDetId();

        itPRInsertHint = prTrackCandId_by_hitId.insert(itPRInsertHint, make_pair(pair<DetId, HitId>(detId, hitId), prTrackCandId));

        // equivalent but not yet in the standard library ?
        //itPRInsertHint = prTrackCandId_by_hitId.emplace_hint(itPRInsertHint, pair<DetId, HitId>(detId, hitId), iHit);

      }
    }
  }




  // ### Get the number of relevant hits for each detector ###
  //Since we are mostly dealing with indices in this module, this is all we need from the StoreArray
  map< DetId, int>  nHits_by_detId;

  //PXD
  if (m_param_usePXDHits) {
    int nHits = 0;
    if (m_param_useClusters) {

      StoreArray<PXDCluster> pxdClusters;
      nHits = pxdClusters.getEntries();

    } else {

      StoreArray<PXDTrueHit> pxdTrueHits;
      nHits = pxdTrueHits.getEntries();

    }
    nHits_by_detId[Const::PXD] = nHits;
  }

  //SVD
  if (m_param_useSVDHits) {
    int nHits = 0;
    if (m_param_useClusters) {

      StoreArray<SVDCluster> svdClusters;
      nHits = svdClusters.getEntries();

    } else {

      StoreArray<SVDTrueHit> svdTrueHits;
      nHits = svdTrueHits.getEntries();

    }
    nHits_by_detId[Const::SVD] = nHits;
  }

  //CDC
  if (m_param_useCDCHits) {
    StoreArray<CDCHit> cdcHits;
    nHits_by_detId[Const::CDC] = cdcHits.getEntries();
  }










  //### Build the confusion matrix ###

  //Reserve enough space for the confusion matrix
  //The last row and column are meant for hits not assigned to a prTrackCand and/or mcTrackCand (aka background)
  Eigen::MatrixXi confusionMatrix = Eigen::MatrixXi::Zero(nPRTrackCands + 1, nMCTrackCands + 1);

  //Accumulated the total number of hits/ndf for each Monte-Carlo track seperatly to avoid double counting,
  //in case patter recognition tracks share hits.
  Eigen::RowVectorXi totalNDF_by_mcTrackCandId = Eigen::RowVectorXi::Zero(nMCTrackCands + 1);

  // The equivalent for the patter recognition tracks is just the rowise sum of the confusion matrix,
  // Double counting can not occure, since Monte-Carlo tracks do not share hits.
  // look below for  Eigen::VectorXi totalNDF_by_prTrackCandId

  //Row index for the hits not assigned to any PRTrackCand
  const int prBkgId = nPRTrackCands;

  //Column index for the hits not assigned to any MCTrackCand
  const int mcBkgId = nMCTrackCands;

  // for each detector examine every hit to which mcTrackCand and prTrackCand it belongs
  // if the hit is not part of any mcTrackCand or any prTrackCand or none of both, put the hit in the corresponding
  for (const pair<const DetId, NDF>& detId_nHits_pair : nHits_by_detId) {

    const DetId& detId = detId_nHits_pair.first;
    const int& nHits = detId_nHits_pair.second;
    const NDF& ndfForOneHit = m_ndf_by_detId[detId];

    for (HitId hitId = 0; hitId < nHits; ++hitId) {
      pair<DetId, HitId> detId_hitId_pair(detId, hitId);

      //First search the unique mcTrackCandId for the hit
      //If the hit is not assigned to any mcTrackCand to Id is set to the background column id iMC
      auto it_mcTrackCandId_for_detId_hitId_pair = mcTrackCandId_by_hitId.find(detId_hitId_pair);

      TrackCandId mcTrackCandId = -1;

      if (it_mcTrackCandId_for_detId_hitId_pair == mcTrackCandId_by_hitId.end()) {
        mcTrackCandId = mcBkgId;

      } else {
        mcTrackCandId = it_mcTrackCandId_for_detId_hitId_pair->second;
      }

      //Assign the hits/ndf to the total ndf vector seperatly here to avoid double counting, if patter recognition track share hits.
      totalNDF_by_mcTrackCandId(mcTrackCandId) += ndfForOneHit;

      //Second examine the prTrackCands if the hit is assigned to none or a couple of them
      size_t n_prTrackCands_for_detId_hitId_pair = prTrackCandId_by_hitId.count(detId_hitId_pair);

      if (n_prTrackCands_for_detId_hitId_pair == 0) {
        // hit is not assigned to any prTrack
        // add the hit with ndf to the row of unassigned hits
        B2DEBUG(200, "prBkgId : " << prBkgId << ";  mcTrackCandId : " <<  mcTrackCandId);

        confusionMatrix(prBkgId, mcTrackCandId) += ndfForOneHit;

        // Annotation : maybe the last row containing hits not assigned to any PRTrack is irrelevant.
        // Since we may assign the ndf of a hit more than once (for each PRTrack that contains it)
        // efficiency measures of the confusion matrix are screwed and only compairing the absolute hit content is applicable.

      } else { /*( n_prTrackCands_for_detId_hitId_pair != 0 )*/
        // hit is assigned to some prTracks

        //Seek all prTrackCands
        //  use as range adapter to convert the iterator pair form equal_range to a range base iterable struct
        auto range_prTrackCandIds_for_detId_hitId_pair = as_range(prTrackCandId_by_hitId.equal_range(detId_hitId_pair));

        // add the degrees of freedom to every prTrackCand that has this hit
        for (const pair<pair<DetId, HitId>, TrackCandId>& detId_hitId_pair_and_prTrackCandId :
             range_prTrackCandIds_for_detId_hitId_pair) {

          TrackCandId prTrackCandId = detId_hitId_pair_and_prTrackCandId.second;

          B2DEBUG(200, " prTrackCandId : " <<  prTrackCandId  << ";  mcTrackCandId : " <<  mcTrackCandId);
          confusionMatrix(prTrackCandId, mcTrackCandId) += ndfForOneHit;
        } //end for

      } //end if ( n_prTrackCands_for_detId_hitId_pair == 0 )

    } //end for hitId
  } // end for decId

  Eigen::VectorXi totalNDF_by_prTrackCandId = confusionMatrix.rowwise().sum();

  B2DEBUG(200, "Confusion matrix of the event : " << endl <<  confusionMatrix);

  B2DEBUG(200, "totalNDF_by_prTrackCandId : " << endl << totalNDF_by_prTrackCandId);
  B2DEBUG(200, "totalNDF_by_mcTrackCandId : " << endl << totalNDF_by_mcTrackCandId);










  // ### Building the patter recognition track to highest purity Monte-Carlo track relation ###
  typedef float Purity;

  vector< pair< TrackCandId, Purity> > purestMCTrackCandId_by_prTrackCandId(nPRTrackCands);

  for (TrackCandId prTrackCandId = 0; prTrackCandId < nPRTrackCands; ++prTrackCandId) {

    NDF totalNDF_for_prTrackCandId = totalNDF_by_prTrackCandId(prTrackCandId);

    Eigen::RowVectorXi prTrackRow = confusionMatrix.row(prTrackCandId);

    Eigen::RowVectorXi::Index purestMCTrackCandId_for_prTrackCandId;

    //Also sets the index of the highest entry in the row vector
    NDF highestNDF_for_prTrackCandId = prTrackRow.maxCoeff(&purestMCTrackCandId_for_prTrackCandId);

    Purity highestPurity = Purity(highestNDF_for_prTrackCandId) / totalNDF_for_prTrackCandId;

    purestMCTrackCandId_by_prTrackCandId[prTrackCandId] = pair< TrackCandId, Purity>(purestMCTrackCandId_for_prTrackCandId, highestPurity);

  }

  // Log the patter recognition track to highest purity Monte-Carlo track relation to debug output
  {
    TrackCandId prTrackCandId = -1;
    B2DEBUG(200, "PRTrack to highest purity MCTrack relation");
    for (const pair< TrackCandId, Purity>& purestMCTrackCandId_for_prTrackCandId : purestMCTrackCandId_by_prTrackCandId) {
      ++prTrackCandId;

      const Purity& purity = purestMCTrackCandId_for_prTrackCandId.second;
      const TrackCandId& mcTrackCandId = purestMCTrackCandId_for_prTrackCandId.first;
      B2DEBUG(200, "prTrackCandId : " << prTrackCandId << " ->  mcTrackCandId : " << mcTrackCandId << " with purity " << purity);
    }
  }










  // ### Building the Monte-Carlo track to highest efficiency patter recognition track relation ###
  typedef float Efficiency;
  vector< pair< TrackCandId,  Efficiency> > mostEfficientPRTrackCandId_by_mcTrackCandId(nMCTrackCands);

  for (TrackCandId mcTrackCandId = 0; mcTrackCandId < nMCTrackCands; ++mcTrackCandId) {

    NDF totalNDF_for_mcTrackCandId = totalNDF_by_mcTrackCandId(mcTrackCandId);

    Eigen::VectorXi mcTrackCandCol = confusionMatrix.col(mcTrackCandId);

    Eigen::VectorXi::Index highestNDFPRTrackCandId_for_mcTrackCandId;

    //Also sets the index of the highest entry in the column vector
    NDF highestNDF_for_mcTrackCandId = mcTrackCandCol.maxCoeff(&highestNDFPRTrackCandId_for_mcTrackCandId);

    Efficiency highestEfficiency = Purity(highestNDF_for_mcTrackCandId) / totalNDF_for_mcTrackCandId;

    mostEfficientPRTrackCandId_by_mcTrackCandId[mcTrackCandId] = pair< TrackCandId, Efficiency>(highestNDFPRTrackCandId_for_mcTrackCandId, highestEfficiency);

  }

  // Log the  Monte-Carlo track to highest efficiency patter recognition track relation
  {
    TrackCandId mcTrackCandId = -1;
    B2DEBUG(200, "PRTrack to highest purity MCTrack relation");
    for (const pair< TrackCandId, Efficiency>& mostEfficientPRTrackCandId_for_mcTrackCandId : mostEfficientPRTrackCandId_by_mcTrackCandId) {
      ++mcTrackCandId;

      const Efficiency& highestEfficiency = mostEfficientPRTrackCandId_for_mcTrackCandId.second;
      const TrackCandId& prTrackCandId = mostEfficientPRTrackCandId_for_mcTrackCandId.first;
      B2DEBUG(200, "mcTrackCandId : " << mcTrackCandId << " ->  prTrackCandId : " << prTrackCandId << " with efficiency " << highestEfficiency);

    }
  }











  // ### Classify the patter recognition tracks ###
  // Meaning saving the highest purity relation to the data store
  // + setup the PRTrack to MCParticle relation
  // + save the McTrackId property
  for (TrackCandId prTrackCandId = 0; prTrackCandId < nPRTrackCands; ++prTrackCandId) {
    genfit::TrackCand* prTrackCand = prGFTrackCands[prTrackCandId];

    const pair<TrackCandId, Purity>& purestMCTrackCandId = purestMCTrackCandId_by_prTrackCandId[prTrackCandId];

    const TrackCandId& mcTrackCandId = purestMCTrackCandId.first;
    const Purity& purity = purestMCTrackCandId.second;

    if (purity < m_param_minimalPurity) {
      //GHOST
      prTrackCand->setMcTrackId(-999);
      B2DEBUG(100, "Stored PRTrack " << prTrackCandId << " as ghost (McTrackId=" << prTrackCand->getMcTrackId()  << ")");

    } else if (mcTrackCandId == mcBkgId) {
      //BACKGROUND
      prTrackCand->setMcTrackId(-99);
      B2DEBUG(100, "Stored PRTrack " << prTrackCandId << " as background (McTrackId=" << prTrackCand->getMcTrackId() << ")");
    } else {

      //after the classification for bad purity and background we examine, whether
      //the highest purity Monte-Carlo track has in turn the highest efficiency patter recognition track
      //equal to this track.
      //All extra patter recognition tracks are stored with negativelly signed purity

      genfit::TrackCand* mcTrackCand = mcGFTrackCands[mcTrackCandId];

      const pair<TrackCandId, Efficiency>& mostEfficientPRTrackCandId_for_mcTrackCandId = mostEfficientPRTrackCandId_by_mcTrackCandId[mcTrackCandId];

      const TrackCandId& mostEfficientPRTrackCandId = mostEfficientPRTrackCandId_for_mcTrackCandId.first;

      //may be one can also impose a minimal efficiency cut on the PRTracks here,
      //but efficiency seem rather unimportant as long as the track is pure and not a ghost
      //const Efficiency & efficiency = mostEfficientPRTrackCandId_for_mcTrackCandId.second;

      if (prTrackCandId != mostEfficientPRTrackCandId) {
        //CLONE

        if (m_param_relateClonesToMCParticles) {

          //Set the McTrackId to the related MCParticle
          prTrackCand->setMcTrackId(mcTrackCand->getMcTrackId());
          B2DEBUG(100, "Set McTrackId property of PRTrack to " << prTrackCand->getMcTrackId());

          //Add the mc matching relation
          MCParticle* mcParticle = DataStore::getRelatedFromObj<MCParticle>(mcTrackCand);
          DataStore::addRelationFromTo(prTrackCand, mcParticle);
          B2DEBUG(100, "MC Match prId " << prTrackCandId << " to mcPartId " << mcParticle->getArrayIndex());


        } else {

          prTrackCand->setMcTrackId(-9);
          B2DEBUG(100, "Set McTrackId property of PRTrack to " << prTrackCand->getMcTrackId());

        }

        //Setup the relation with negative purity for this case
        DataStore::addRelationFromTo(prTrackCand, mcTrackCand, -purity);
        B2DEBUG(100, "Purity rel: mcId " << prTrackCandId << " -> prId " << mcTrackCandId << " : " << -purity);

      } else {
        //MATCHED

        //Set the McTrackId to the related MCParticle
        prTrackCand->setMcTrackId(mcTrackCand->getMcTrackId());
        B2DEBUG(100, "Set McTrackId property of PRTrack to " << prTrackCand->getMcTrackId());

        //Add the mc matching relation
        MCParticle* mcParticle = DataStore::getRelatedFromObj<MCParticle>(mcTrackCand);
        DataStore::addRelationFromTo(prTrackCand, mcParticle);
        B2DEBUG(100, "MC Match prId " << prTrackCandId << " to mcPartId " <<  mcParticle->getArrayIndex());

        //Setup the relation with positive purity for this case
        DataStore::addRelationFromTo(prTrackCand, mcTrackCand, purity);
        B2DEBUG(100, "Purity rel: prId " << prTrackCandId << " -> mcId " << mcTrackCandId << " : " << purity);


      }
    }

  }










  // ### Classify the Monte-Carlo tracks ###
  // Meaning save the highest efficiency relation to the data store.
  for (TrackCandId mcTrackCandId = 0; mcTrackCandId < nMCTrackCands; ++mcTrackCandId) {
    genfit::TrackCand* mcTrackCand = mcGFTrackCands[mcTrackCandId];

    const pair<TrackCandId, Efficiency>& mostEfficiencyPRTrackCandId = mostEfficientPRTrackCandId_by_mcTrackCandId[mcTrackCandId];

    const TrackCandId& prTrackCandId = mostEfficiencyPRTrackCandId.first;
    const Efficiency& efficiency = mostEfficiencyPRTrackCandId.second;

    if (prTrackCandId == prBkgId or prGFTrackCands[prTrackCandId]->getMcTrackId() < 0) {
      //MISSING
      //No related pattern recognition track
      //Do not create a relation.
      B2DEBUG(100, "mcId " << mcTrackCandId << " is missing. No relation created.");

    } else {

      genfit::TrackCand* prTrackCand = prGFTrackCands[prTrackCandId];

      const pair<TrackCandId, Purity>& purestMCTrackCandId_for_prTrackCandId = purestMCTrackCandId_by_prTrackCandId[prTrackCandId];
      const TrackCandId& purestMCTrackCandId = purestMCTrackCandId_for_prTrackCandId.first;

      if (mcTrackCandId != purestMCTrackCandId) {
        //MERGED
        // this MCTrack is in a PRTrack which in turn better describes a MCTrack different form this.

        //Setup the relation with negative efficiency for this case.
        DataStore::addRelationFromTo(mcTrackCand, prTrackCand, -efficiency);
        B2DEBUG(100, "Efficiency rel: mcId " << mcTrackCandId << " -> prId " << prTrackCandId << " : " << -efficiency);

      } else {
        //MATCHED

        //Setup the relation with positive efficiency for this case.
        DataStore::addRelationFromTo(mcTrackCand, prTrackCand, efficiency);
        B2DEBUG(100, "Efficiency rel: mcId " << mcTrackCandId << " -> prId " << prTrackCandId << " : " << efficiency);

      }
    }

  }

  B2DEBUG(100, "########## MCTrackMatcherModule ############");

} //end event()


void MCTrackMatcherModule::endRun()
{
}

void MCTrackMatcherModule::terminate()
{
}


