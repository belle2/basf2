/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kurz                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdMetadata/SVDTrackingEventLevelMdstInfoFillerModule.h>

#include <vxd/dataobjects/VxdID.h>

using namespace Belle2;

REG_MODULE(SVDTrackingEventLevelMdstInfoFiller)

SVDTrackingEventLevelMdstInfoFillerModule::SVDTrackingEventLevelMdstInfoFillerModule() : Module()
{
  setDescription("This module adds additional global event level information about SVD track finding to the MDST object 'EventLevelTrackingInfo'");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("EventLevelTrackingInfoName",
           m_eventLevelTrackingInfoName,
           "Name of the EventLevelTrackingInfo that should be used (different one for ROI-finding).",
           m_eventLevelTrackingInfoName);

  addParam("svdClustersName",
           m_svdClustersName,
           "Name of the SVDClusters that should be used (different one for ROI-finding).",
           m_svdClustersName);
}


void SVDTrackingEventLevelMdstInfoFillerModule::initialize()
{
  m_svdClusters.isRequired(m_svdClustersName);
  m_eventLevelTrackingInfo.isRequired(m_eventLevelTrackingInfoName);
}


void SVDTrackingEventLevelMdstInfoFillerModule::event()
{
  B2INFO(m_svdClusters.getName() << " - " << m_eventLevelTrackingInfo.getName());

  // use a simple array for the SVD layers (if geometry is changed then also EventLevelTrackingInfo doesn't work)
  // first index: layer number (shifted by 3), second index: isU
  int nClustersPerLayer[4][2] = {};
  for (const auto& cluster : m_svdClusters) {
    nClustersPerLayer[((int)cluster.getSensorID().getLayerNumber()) - 3][(int)cluster.isUCluster()]++;
  }

  if (m_eventLevelTrackingInfo.isValid()) {
    for (int layer = 0; layer <= 3; layer++) {
      for (int isU = 0; isU <= 1; isU++) {
        m_eventLevelTrackingInfo->setNVXDClustersInLayer(layer + 3, isU, nClustersPerLayer[layer][isU]);
      }
    }

  }
}

