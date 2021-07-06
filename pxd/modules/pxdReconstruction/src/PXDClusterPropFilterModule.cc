/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <pxd/modules/pxdReconstruction/PXDClusterPropFilterModule.h>
#include <map>

using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDClusterPropFilter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDClusterPropFilterModule::PXDClusterPropFilterModule() : Module()
{
  // Set module properties
  setDescription("The module produce a StoreArray of PXDClusters with specific cuts on properties.");

  // Parameter definitions
  addParam("PXDClustersName", m_PXDClustersName, "The name of the StoreArray of PXDClusters to be filtered", std::string(""));
  addParam("PXDClustersInsideCutsName", m_PXDClustersInsideCutsName, "The name of the StoreArray of Filtered PXDClusters inside cuts",
           std::string("PXDClustersIN"));
  addParam("PXDClustersOutsideCutsName", m_PXDClustersOutsideCutsName,
           "The name of the StoreArray of Filtered PXDClusters outside cuts",
           std::string("PXDClustersOUT"));
  addParam("CreateInside", m_CreateInside, "Create the StoreArray of PXD clusters inside of cuts", true);
  addParam("CreateOutside", m_CreateOutside, "Create the StoreArray of PXD clusters outside of cuts", false);

  addParam("minCharge", m_minCharge , "minimum charge, including value", 0.0);
  addParam("maxCharge", m_maxCharge , "maximum charge, excluding value", 9999.0);
  addParam("minSize", m_minSize , "minimum size, including value", 1);
  addParam("maxSize", m_maxSize , "maximum size, excluding value", 99);
}

void PXDClusterPropFilterModule::initialize()
{

  // We have to change it once the hardware type clusters are well defined
  StoreArray<PXDCluster> PXDClusters(m_PXDClustersName);   /**< The PXDClusters to be filtered */
  PXDClusters.isRequired();

  if (m_CreateInside) {
    m_selectorIN.registerSubset(PXDClusters, m_PXDClustersInsideCutsName);
    m_selectorIN.inheritAllRelations();
  }
  if (m_CreateOutside) {
    m_selectorOUT.registerSubset(PXDClusters, m_PXDClustersOutsideCutsName);
    m_selectorOUT.inheritAllRelations();
  }
}

bool PXDClusterPropFilterModule::CheckCuts(const PXDCluster& cluster)
{
  return cluster.getSize() >= m_minSize && cluster.getSize() < m_maxSize &&
         cluster.getCharge() >= m_minCharge && cluster.getCharge() < m_maxCharge;
}

void PXDClusterPropFilterModule::event()
{
  // Perform filtering
  StoreArray<PXDCluster> PXDClusters(m_PXDClustersName);   /**< The PXDClusters to be filtered */

  if (m_CreateInside) {
    m_selectorIN.select([this](const PXDCluster * thePxdCluster) {
      return CheckCuts(*thePxdCluster); // the cluster has interesting Properties.
    });
  }

  if (m_CreateOutside) {
    m_selectorOUT.select([this](const PXDCluster * thePxdCluster) {
      return ! CheckCuts(*thePxdCluster); // the cluster has NO interesting Properties.
    });
  }
}
