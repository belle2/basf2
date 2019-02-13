/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa & Luigi Corona                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdPerformance/SVDShaperDigitsFromTracksModule.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <framework/datastore/StoreArray.h>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDShaperDigitsFromTracks)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDShaperDigitsFromTracksModule::SVDShaperDigitsFromTracksModule() : Module()
{
  B2DEBUG(1, "Constructor");
  // Set module properties
  setDescription("generates two new StoreArray from the input StoreArray. One contains all ShaperDigits related to Tracks and the other contains all SahperDigits not related to tracks");

  // Parameter definitions
  addParam("SVDShaperDigits", m_svdshaper, "StoreArray with the input shaperdigits", std::string("SVDShaperDigits"));
  addParam("SVDRecoDigits", m_svdreco, "StoreArray with the input recodigits", std::string("SVDRecoDigits"));
  addParam("SVDClusters", m_svdcluster, "StoreArray with the input clusters", std::string("SVDClusters"));
  addParam("Tracks", m_track, "StoreArray with the input tracks", std::string("Tracks"));
  addParam("RecoTracks", m_recotrack, "StoreArray with the input recotracks", std::string("RecoTracks"));
  addParam("outputINArrayName", m_outputINArrayName, "StoreArray with the output shaperdigits",
           std::string("SVDShaperDigitsFromTracks"));
  addParam("outputOUTArrayName", m_outputOUTArrayName, "StoreArray with the output shaperdigits",
           std::string("SVDShaperDigitsNotFromTracks"));
  addParam("InheritAllRelations", m_inheritance,
           "Set true if you want to inherit all relations between StoreArray, the default is false", bool(false));
}

SVDShaperDigitsFromTracksModule::~SVDShaperDigitsFromTracksModule()
{
  B2DEBUG(20, "Destructor");
}


void SVDShaperDigitsFromTracksModule::initialize()
{

  B2DEBUG(10, "SVDShaperDigits: " << m_svdshaper);
  B2DEBUG(10, "SVDRecoDigits: " << m_svdreco);
  B2DEBUG(10, "SVDClusters: " << m_svdcluster);
  B2DEBUG(10, "Tracks: " << m_track);
  B2DEBUG(10, "RecoTracks: " << m_recotrack);
  B2DEBUG(10, "outputINArrayName: " <<  m_outputINArrayName);
  B2DEBUG(10, "outputOUTArrayName: " <<  m_outputOUTArrayName);
  B2DEBUG(10, "InheritAllRelations: " <<  m_inheritance);

  StoreArray<SVDShaperDigit> ShaperDigits(m_svdshaper);
  StoreArray<SVDCluster> Clusters(m_svdcluster);
  StoreArray<SVDRecoDigit> RecoDigits(m_svdreco);
  StoreArray<RecoTrack> recoTracks(m_recotrack);
  StoreArray<Track> tracks(m_track);
  ShaperDigits.isRequired();
  Clusters.isRequired();
  RecoDigits.isRequired();
  recoTracks.isRequired();
  tracks.isRequired();
  m_selectedShaperDigits.registerSubset(ShaperDigits, m_outputINArrayName);
  if (m_inheritance) {
    m_selectedShaperDigits.inheritAllRelations();
  }
  if (m_outputOUTArrayName != "") {
    m_notSelectedShaperDigits.registerSubset(ShaperDigits, m_outputOUTArrayName);
    if (m_inheritance) {
      m_notSelectedShaperDigits.inheritAllRelations();
    }
  }

}


void SVDShaperDigitsFromTracksModule::beginRun()
{
}


void SVDShaperDigitsFromTracksModule::event()
{
  StoreArray<SVDShaperDigit> ShaperDigits(m_svdshaper);

  m_selectedShaperDigits.select([this](const SVDShaperDigit * theSVDShaperDigit) {
    return isRelatedToTrack(theSVDShaperDigit);
  });


  m_notSelectedShaperDigits.select([this](const SVDShaperDigit * theSVDShaperDigit) {
    return !isRelatedToTrack(theSVDShaperDigit);
  });
}

void SVDShaperDigitsFromTracksModule::endRun()
{
}

void SVDShaperDigitsFromTracksModule::terminate()
{
}

bool SVDShaperDigitsFromTracksModule::isRelatedToTrack(const SVDShaperDigit* shaperdigit)
{
  RelationVector<SVDRecoDigit> reco_rel_shape = shaperdigit->getRelationsFrom<SVDRecoDigit>();
  if (reco_rel_shape.size() == 0) {return false;}
  else {
    RelationVector<SVDCluster> cluster_rel_reco = reco_rel_shape[0]->getRelationsFrom<SVDCluster>();
    if (cluster_rel_reco.size() == 0) {return false;}
    else {
      RelationVector<RecoTrack> recotrack_rel_cluster = cluster_rel_reco[0]->getRelationsTo<RecoTrack>();
      if (recotrack_rel_cluster.size() == 0) {return false;}
      else {
        RelationVector<Track> track_rel_recotrack = recotrack_rel_cluster[0]->getRelationsFrom<Track>();
        if (track_rel_recotrack.size() == 0) {return false;}
        else {return true;}
      }
    }
  }
}
