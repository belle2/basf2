/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 **************************************************************************/

#include <svd/modules/svdSpacePointCreator/SVDSpacePointCreatorModule.h>

#include <svd/modules/svdSpacePointCreator/SpacePointHelperFunctions.h>

#include <framework/logging/Logger.h>




using namespace std;
using namespace Belle2;


REG_MODULE(SVDSpacePointCreator)

SVDSpacePointCreatorModule::SVDSpacePointCreatorModule() :
  Module()
{
  InitializeCounters();

  setDescription("Imports Clusters of the SVD detector and converts them to spacePoints.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1. Collections.
  addParam("SVDClusters", m_svdClustersName,
           "SVDCluster collection name", string(""));
  addParam("SpacePoints", m_spacePointsName,
           "SpacePoints collection name", string("SVDSpacePoints"));

  // 2.Modification parameters:
  addParam("NameOfInstance", m_nameOfInstance,
           "allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module", string(""));
  addParam("OnlySingleClusterSpacePoints", m_onlySingleClusterSpacePoints,
           "standard is false. If activated, the module will not try to find combinations of U and V clusters for the SVD any more",
           bool(false));

  addParam("MinClusterTime", m_minClusterTime, "clusters with time below this value are not considered to make spacePoints.",
           float(-20));
  addParam("inputPDF", m_inputPDF,
           "Path containing pdf root file", std::string("/data/svd/spacePointQICalibration.root"));
}



void SVDSpacePointCreatorModule::initialize()
{
  // prepare all store- and relationArrays:
  m_spacePoints.registerInDataStore(m_spacePointsName, DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered);
  m_svdClusters.isRequired(m_svdClustersName);


  //Relations to cluster objects only if the ancestor relations exist:
  m_spacePoints.registerRelationTo(m_svdClusters, DataStore::c_Event, DataStore::c_DontWriteOut);


  B2DEBUG(1, "SVDSpacePointCreatorModule(" << m_nameOfInstance << ")::initialize: names set for containers:\n" <<
          "\nsvdClusters: " << m_svdClusters.getName() <<
          "\nspacePoints: " << m_spacePoints.getName());


  if (m_inputPDF.empty()) {
    B2ERROR("PDF File" << m_inputPDF << "not found");
  } else {
    std::string fullPath = FileSystem::findFile(m_inputPDF);
    if (fullPath.empty()) {
      B2ERROR("PDF file" << m_inputPDF << "not found");
    }
    m_inputPDF = fullPath;
  }

  m_calibrationFile = new TFile(m_inputPDF.c_str(), "READ");
  if (!m_calibrationFile->IsOpen())
    B2FATAL("Couldn't open pdf file:" << m_inputPDF);

  // set some counters for output:
  InitializeCounters();
}



void SVDSpacePointCreatorModule::event()
{



  if (m_onlySingleClusterSpacePoints == true) {
    provideSVDClusterSingles(m_svdClusters,
                             m_spacePoints); /// WARNING TODO: missing: possibility to allow storing of u- or v-type clusters only!
  } else {
    provideSVDClusterCombinations(m_svdClusters, m_spacePoints, m_minClusterTime, m_calibrationFile);

  }


  B2DEBUG(1, "SVDSpacePointCreatorModule(" << m_nameOfInstance <<
          ")::event: spacePoints for single SVDClusters created! Size of arrays:\n" <<
          ", svdClusters: " << m_svdClusters.getEntries() <<
          ", spacePoints: " << m_spacePoints.getEntries());


  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 10, PACKAGENAME()) == true) {
    for (int index = 0; index < m_spacePoints.getEntries(); index++) {
      const SpacePoint* sp = m_spacePoints[index];

      B2DEBUG(10, "SVDSpacePointCreatorModule(" << m_nameOfInstance << ")::event: spacePoint " << index <<
              " with type " << sp->getType() <<
              " and VxdID " << VxdID(sp->getVxdID()) <<
              " is tied to a cluster in: " << sp->getArrayName());
    }
  }

  m_TESTERSVDClusterCtr += m_svdClusters.getEntries();
  m_TESTERSpacePointCtr += m_spacePoints.getEntries();

}



void SVDSpacePointCreatorModule::terminate()
{
  B2DEBUG(1, "SVDSpacePointCreatorModule(" << m_nameOfInstance << ")::terminate: total number of occured instances:\n" <<
          ", svdClusters: " << m_TESTERSVDClusterCtr <<
          ", spacePoints: " << m_TESTERSpacePointCtr);
  m_calibrationFile->Close();
}


void SVDSpacePointCreatorModule::InitializeCounters()
{
  m_TESTERSVDClusterCtr = 0;
  m_TESTERSpacePointCtr = 0;
}
