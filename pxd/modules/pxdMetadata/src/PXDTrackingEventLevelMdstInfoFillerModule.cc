/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kurz                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdMetadata/PXDTrackingEventLevelMdstInfoFillerModule.h>

#include <vxd/dataobjects/VxdID.h>

using namespace Belle2;

REG_MODULE(PXDTrackingEventLevelMdstInfoFiller)

PXDTrackingEventLevelMdstInfoFillerModule::PXDTrackingEventLevelMdstInfoFillerModule() : Module()
{
  setDescription("This module adds additional global event level information about PXD track finding to the MDST object 'EventLevelTrackingInfo'");
  setPropertyFlags(c_ParallelProcessingCertified);
}


void PXDTrackingEventLevelMdstInfoFillerModule::initialize()
{
  m_pxdClusters.isRequired();
  m_eventLevelTrackingInfo.isRequired();
}


void PXDTrackingEventLevelMdstInfoFillerModule::event()
{

  // use a simple array for the PXD layers (if geometry is changed then also EventLevelTrackingInfo doesn't work)
  // index: layer number (shifted by 1)
  int nClustersPerLayer[2] = {};
  for (auto& cluster : m_pxdClusters) {
    nClustersPerLayer[((int)cluster.getSensorID().getLayerNumber()) - 1]++;
  }

  if (m_eventLevelTrackingInfo.isValid()) {
    for (int layer = 0; layer <= 1; layer++) {
      m_eventLevelTrackingInfo->setNVXDClustersInLayer(layer + 1, 0, nClustersPerLayer[layer]);
    }

  }
}

