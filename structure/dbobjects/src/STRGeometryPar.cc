/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Alexandre BEAULIEU                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <structure/dbobjects/STRGeometryPar.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

STRGeometryPar::STRGeometryPar(const GearDir& content)
{
  m_Version = 0;
  read(content);
}

STRGeometryPar::~STRGeometryPar()
{
}

// Get STR geometry parameters from Gearbox (no calculations here)
void STRGeometryPar::read(const GearDir& content)
{

  ReadShield(content, "FWD_Shield");
  ReadShield(content, "BWD_Shield");

}

void STRGeometryPar::ReadShield(const GearDir& content, std::string side)
{

  // Check if method was called using the correct name for the shields
  std::size_t foundF = side.find("FWD_Shield");
  std::size_t foundB = side.find("BWD_Shield");

  int iShield;
  if (foundF != std::string::npos) { iShield = FWD_ECLSHIELD; }
  else if (foundB != std::string::npos) { iShield = BWD_ECLSHIELD; }
  else { B2FATAL("No data for ECL shield called " << side << "( not found)");}



  std::string gearPath = (boost::format("%1%/Layers/Layer") % side).str();

  // Retrieve the number of layers in the shield
  m_NLayers[iShield] = content.getNumberNodes(gearPath);

  // Resize std::vectors according to the number of layers
  m_LayerMaterial[iShield].resize(m_NLayers[iShield]);
  m_LayerNPlanes[iShield].resize(m_NLayers[iShield]);
  m_LayerPlaneZ[iShield].resize(m_NLayers[iShield]);
  m_LayerPlaneInnerRadius[iShield].resize(m_NLayers[iShield]);
  m_LayerPlaneOuterRadius[iShield].resize(m_NLayers[iShield]);

  for (int iLayer = 0 ; iLayer < m_NLayers[iShield] ; ++iLayer) {
    //Thread the strings
    std::string layerPath   = (boost::format("/%1%[%2%]/") % gearPath % (iLayer + 1)).str();

    // Connect the appropriate Gearbox path
    GearDir layerContent(content);
    layerContent.append(layerPath);

    // Retrieve material material
    m_LayerMaterial[iShield].at(iLayer) = layerContent.getString("Material", "Air");
    // Read the shape parameters
    const std::vector<GearDir> planes = layerContent.getNodes("Plane");
    m_LayerNPlanes[iShield].at(iLayer) = planes.size();
    B2INFO("Number of planes on side  " << side << " layer " << iLayer
           << " : " << planes.size());

    // Resize std::vectors according to the number of planes in each layer
    m_LayerPlaneZ[iShield].at(iLayer).resize(m_LayerNPlanes[iShield].at(iLayer));
    m_LayerPlaneInnerRadius[iShield].at(iLayer).resize(m_LayerNPlanes[iShield].at(iLayer));
    m_LayerPlaneOuterRadius[iShield].at(iLayer).resize(m_LayerNPlanes[iShield].at(iLayer));

    for (unsigned int iPlane = 0; iPlane < planes.size(); iPlane++) {
      m_LayerPlaneZ[iShield].at(iLayer).at(iPlane) = planes.at(iPlane).getLength("posZ");
      m_LayerPlaneInnerRadius[iShield].at(iLayer).at(iPlane) = planes.at(iPlane).getLength("innerRadius");
      m_LayerPlaneOuterRadius[iShield].at(iLayer).at(iPlane) = planes.at(iPlane).getLength("outerRadius");
    }
  }
}
