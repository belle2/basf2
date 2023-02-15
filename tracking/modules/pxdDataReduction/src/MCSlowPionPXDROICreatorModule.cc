/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/pxdDataReduction/MCSlowPionPXDROICreatorModule.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <pxd/dataobjects/PXDDigit.h>
#include <mdst/dataobjects/MCParticle.h>
#include <tracking/dataobjects/ROIid.h>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(MCSlowPionPXDROICreator);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

MCSlowPionPXDROICreatorModule::MCSlowPionPXDROICreatorModule() : Module()
{
  // Set module properties
  setDescription("Create artificial ROI just for PXDDigits from slow pions from D* decays based on MC information.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Parameter definitions
  addParam("PXDDigitsName", m_pxdDigitsName, "StoreArray name of the input PXD clusters", std::string(""));
  addParam("MCParticlesName", m_MCParticlesName, "StoreArray name of the input tracks", std::string(""));
  addParam("ROIsName", m_ROIsName, "StoreArray name of the ROIs", std::string(""));
  addParam("ROISize", m_ROISize,
           "Size of the ROIs. Set to a rather large value to only create on ROI per sensor instead of one ROI per PXDDigit.",
           m_ROISize);
}


void MCSlowPionPXDROICreatorModule::initialize()
{
  m_PXDDigits.isRequired(m_pxdDigitsName);
  m_MCParticles.isRequired(m_MCParticlesName);
  m_ROIs.isRequired(m_ROIsName);

  B2DEBUG(29, "StoreArray names in use: \n"
          "    MCParticles: " << m_MCParticlesName << "\n"
          "    PXDDigits: " << m_pxdDigitsName << "\n"
          "    ROIs: " << m_ROIsName);
}


void MCSlowPionPXDROICreatorModule::event()
{
  // Just skip if there are no MCParticles by accident in simulation,
  // or because the module is added to the path for data reconstruction
  if (m_MCParticles.getEntries() == 0) {
    return;
  }

  for (const MCParticle& mcParticle : m_MCParticles) {
    // Nothing to do if the particle is not a slow pion from a D* -> D pi^{\pm} decay
    // or if there is no mother particle e.g. when using particle gun.
    if (not mcParticle.getMother() or
        not(std::abs(mcParticle.getPDG()) == 211 and
            std::abs(mcParticle.getMother()->getPDG()) == 413)) {
      continue;
    }
    const RelationVector<PXDDigit>& relatedPXDDigits = mcParticle.getRelationsFrom<PXDDigit>(m_pxdDigitsName);
    // Don't try to create ROIs if there are no PXDDigits for this slow pion
    if (relatedPXDDigits.size() == 0) {
      continue;
    }

    // Only count slow pions with related PXDDigits
    m_slowPiCounter++;
    VxdID lastSensor(0);
    for (const PXDDigit& digit : relatedPXDDigits) {
      const VxdID& sensor = digit.getSensorID();
      // Only create one ROI per sensor instead of one per PXDDigit.
      // So if there already is a ROI on this sensor from a previous PXDDigit of this MCParticle, just skip this PXDDigit.
      if (sensor == lastSensor) {
        continue;
      }
      lastSensor = sensor;
      const short uCell = digit.getUCellID();
      const short vCell = digit.getVCellID();

      // Check consistency of cells to only create ROIs that are fully contained on a sensor
      const short minU = uCell - m_ROISize >= 0   ? uCell - m_ROISize : 0;
      const short maxU = uCell + m_ROISize <= 249 ? uCell + m_ROISize : 249;
      const short minV = vCell - m_ROISize >= 0   ? vCell - m_ROISize : 0;
      const short maxV = vCell + m_ROISize <= 767 ? vCell + m_ROISize : 767;

      m_ROIs.appendNew(ROIid(minU, maxU, minV, maxV, sensor));
      m_ROICounter++;
    }
  }
}

void MCSlowPionPXDROICreatorModule::endRun()
{
  B2DEBUG(29, "Created " << m_ROICounter << " ROIs for " << m_slowPiCounter << " slow Pions, that's " <<
          (double)m_ROICounter / (double)m_slowPiCounter << " per slow pion.");
}
