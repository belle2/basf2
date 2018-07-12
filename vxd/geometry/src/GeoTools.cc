/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys, Peter Kvasnicka                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/geometry/GeoTools.h>
#include <vxd/geometry/GeoCache.h>

using namespace Belle2;
using namespace Belle2::VXD;

GeoTools::GeoTools()
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  m_listOfSensors = geo.getListOfSensors();
  // The list comes from a map, and is NOT sorted!
  std::sort(m_listOfSensors.begin(), m_listOfSensors.end());
  m_firstSVDIndex = std::distance(
                      m_listOfSensors.begin(),
                      std::find_if(m_listOfSensors.begin(), m_listOfSensors.end(),
                                   [](VxdID id)->bool { return (id.getLayerNumber() > 2); })
                    ); // works correctly even with no SVD sensors
  createListOfLayers();
  createListOfPXDChips();
  createListOfSVDChips();
}

/** Create list of VXD layers */
void GeoTools::createListOfLayers()
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  auto layerSet = geo.getLayers();
  std::transform(
    layerSet.begin(), layerSet.end(),
    std::back_inserter(m_listOfLayers),
    [](VxdID id)->int { return id.getLayerNumber(); }
  );
  // Better to sort this too, just in case.
  std::sort(m_listOfLayers.begin(), m_listOfLayers.end());
  m_firstSVDLayer = static_cast<unsigned short>(
                      std::distance(m_listOfLayers.begin(),
                                    std::find(m_listOfLayers.begin(), m_listOfLayers.end(), 3)
                                   ));
}

/** Create list of PXD chips. */
void GeoTools::createListOfPXDChips()
{
  // We set segment number to the number of chip.
  std::vector<unsigned short> sensorChips(c_nPXDChipsU + c_nPXDChipsV);
  std::iota(sensorChips.begin(), sensorChips.end(), 0);
  for (auto sensorID : m_listOfSensors) {
    if (sensorID.getLayerNumber() < getFirstPXDLayer() ||
        sensorID.getLayerNumber() > getLastPXDLayer())
      continue;
    std::vector<VxdID> sensorIDs;
    std::transform(
      sensorChips.begin(), sensorChips.end(),
      std::back_inserter(sensorIDs),
      [sensorID](unsigned short i)->VxdID
    { VxdID sID(sensorID); sID.setSegmentNumber(i); return sID; }
    );
    m_listOfPXDChips.insert(m_listOfPXDChips.end(), sensorIDs.begin(), sensorIDs.end());
  }
}

/** Create list of SVD chips. */
void GeoTools::createListOfSVDChips()
{
  // We set segment number to the number of chip.
  std::vector<unsigned short> sensorChipsL456(c_nSVDChipsLu + c_nSVDChipsLv);
  std::iota(sensorChipsL456.begin(), sensorChipsL456.end(), 0);
  std::vector<unsigned short> sensorChipsL3(2 * c_nSVDChipsL3);
  std::iota(sensorChipsL3.begin(), sensorChipsL3.end(), 0);
  for (auto sensorID : m_listOfSensors) {
    if (sensorID.getLayerNumber() < getFirstSVDLayer() ||
        sensorID.getLayerNumber() > getLastSVDLayer())
      continue;
    std::vector<VxdID> sensorIDs;
    std::vector<unsigned short>& sensorChips =
      sensorID.getLayerNumber() == 3 ? sensorChipsL3 : sensorChipsL456;
    std::transform(
      sensorChips.begin(), sensorChips.end(),
      std::back_inserter(sensorIDs),
      [sensorID](unsigned short i)->VxdID
    { VxdID sID(sensorID); sID.setSegmentNumber(i); return sID; }
    );
    m_listOfSVDChips.insert(m_listOfSVDChips.end(), sensorIDs.begin(), sensorIDs.end());
  }
}

