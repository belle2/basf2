/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: bjoern.spruck@belle2.org                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/pxdDataReduction/PXDclusterFilterModule.h>
#include <map>

using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDclusterFilter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDclusterFilterModule::PXDclusterFilterModule() : Module()
{
  // Set module properties
  setDescription("The module produce a StoreArray of PXDClusters inside/overlapping any of the ROIs.");

  // Parameter definitions
  addParam("PXDClustersName", m_PXDClustersName, "The name of the StoreArray of PXDClusters to be filtered", std::string(""));
  addParam("PXDClustersInsideROIName", m_PXDClustersInsideROIName, "The name of the StoreArray of Filtered PXDClusters",
           std::string("PXDClustersIN"));
  addParam("PXDClustersOutsideROIName", m_PXDClustersOutsideROIName, "The name of the StoreArray of Filtered PXDClusters",
           std::string("PXDClustersOUT"));
  addParam("ROIidsName", m_ROIidsName, "The name of the StoreArray of ROIs", std::string(""));
  addParam("CreateOutside", m_CreateOutside, "Create the StoreArray of PXD clusters outside the ROIs", false);

}

PXDclusterFilterModule::~PXDclusterFilterModule()
{
}

void PXDclusterFilterModule::initialize()
{

  StoreArray<ROIid>::required(m_ROIidsName);

  // We have to change it once the hardware type clusters are well defined
  StoreArray<PXDCluster> PXDClusters(m_PXDClustersName);   /**< The PXDClusters to be filtered */
  PXDClusters.isRequired();

  m_selectorIN.registerSubset(PXDClusters, m_PXDClustersInsideROIName);
  m_selectorIN.inheritAllRelations();

  if (m_CreateOutside) {
    m_selectorOUT.registerSubset(PXDClusters, m_PXDClustersOutsideROIName);
    m_selectorOUT.inheritAllRelations();
  }
}

void PXDclusterFilterModule::beginRun()
{
}

bool PXDclusterFilterModule::Overlaps(const ROIid& theROI, const PXDCluster& thePXDCluster)
{
  // Not so easy, we have to check every pixel which is time consuming
  // maybe optimze: we can first check the cluster as rectangular but that only helps if we have that property before
  // PXDClusters do not have them ...

  if (theROI.getSensorID() != thePXDCluster.getSensorID()) return false; // anyway checked before?

  // Loop over all Pixel related to this CLuster
  for (auto thePixel : thePXDCluster.getRelationsTo<PXDDigit>()) {
    // if any pixel inside
    if (theROI.getMinUid() <= thePixel.getUCellID() &&
        theROI.getMaxUid() >= thePixel.getUCellID() &&
        theROI.getMinVid() <= thePixel.getVCellID() &&
        theROI.getMaxVid() >= thePixel.getVCellID())  return true;
  }
  // no pixel inside
  return false;
}

void PXDclusterFilterModule::event()
{
  // We have to change it once the hardware type clusters are well defined
  StoreArray<PXDCluster> PXDClusters(m_PXDClustersName);   /**< The PXDClusters to be filtered */
  StoreArray<ROIid> ROIids_store_array(m_ROIidsName); /**< The ROIs */

  multimap< VxdID, ROIid > ROIids;

  for (auto ROI : ROIids_store_array)
    ROIids.insert(pair<VxdID, ROIid> (ROI.getSensorID() , ROI));

  m_selectorIN.select([ROIids, this](const PXDCluster * thePxdCluster) {
    auto ROIidsRange = ROIids.equal_range(thePxdCluster->getSensorID()) ;
    for (auto theROI = ROIidsRange.first ; theROI != ROIidsRange.second; theROI ++)
      if (Overlaps(theROI->second, *thePxdCluster)) // *or* Cluster has intersting Properties. TODO
        return true;

    return false;
  });

  if (m_CreateOutside) {
    m_selectorOUT.select([ROIids, this](const PXDCluster * thePxdCluster) {
      auto ROIidsRange = ROIids.equal_range(thePxdCluster->getSensorID()) ;
      for (auto theROI = ROIidsRange.first ; theROI != ROIidsRange.second; theROI ++)
        if (Overlaps(theROI->second, *thePxdCluster)) // *and* Cluster has NO intersting Properties. TODO
          return false;

      return true;
    });
  }

}

void PXDclusterFilterModule::endRun()
{
}

void PXDclusterFilterModule::terminate()
{
}
