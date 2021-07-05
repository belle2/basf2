/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

  addParam("overrideDB", m_overrideDB, "If set, ROI-finding settings in DB are overwritten", false);
  addParam("enableFiltering", m_enableFiltering, "enables/disables ROI-finding if overrideDB=True", false);

}

void PXDclusterFilterModule::initialize()
{

  StoreArray<ROIid> roiIDs;
  roiIDs.isRequired(m_ROIidsName);

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
  // reset variables used to enable/disable ROI-finding
  m_skipEveryNth = -1;
  if (m_roiParameters) {
    m_skipEveryNth = m_roiParameters->getDisableROIforEveryNth();
  } else {
    B2ERROR("No configuration for the current run found");
  }
  m_countNthEvent = 0;
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
  // parameters might also change on a per-event basis
  if (m_roiParameters.hasChanged()) {
    if (m_roiParameters) {
      m_skipEveryNth = m_roiParameters->getDisableROIforEveryNth();
    } else {
      B2ERROR("No configuration for the current run found");
    }
    // and reset counter
    m_countNthEvent = 0;
  }

  if (m_overrideDB) {
    if (m_enableFiltering) {
      filterClusters();
    } else {
      copyClusters();
    }
    return;
  }

  m_countNthEvent++;

  // Data reduction disabled -> simply copy everything
  if (m_skipEveryNth > 0 and m_countNthEvent % m_skipEveryNth == 0) {
    copyClusters();
    m_countNthEvent = 0;

    return;
  }

  // Perform data reduction
  filterClusters();
}

void PXDclusterFilterModule::filterClusters()
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

void PXDclusterFilterModule::copyClusters()
{
  // omitting the variable name; otherwise a warning is produced (un-used variable)
  m_selectorIN.select([](const PXDCluster* /* thePxdCluster */) {return true;});
}


