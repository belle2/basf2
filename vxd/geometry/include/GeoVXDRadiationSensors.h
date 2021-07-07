/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#ifndef VXD_GEOMETRY_GEOVXDRADIATONSENSORS_H
#define VXD_GEOMETRY_GEOVXDRADIATONSENSORS_H

#include <simulation/kernel/SensitiveDetectorBase.h>
#include <vector>
#include <string>
#include <G4LogicalVolume.hh>
#include <framework/gearbox/GearDir.h>

namespace Belle2 {

  /** class to create the diamond radiation sensor geometry if defined */
  class GeoVXDRadiationSensors {
  public:
    /** construct the creator with the correct subdetector type
     * @param subdetector one of "PXD" or "SVD"
     */
    explicit GeoVXDRadiationSensors(const std::string& subdetector): m_subdetector(subdetector) {}
    /** delete sensitive detector implementations */
    ~GeoVXDRadiationSensors()
    {
      for (auto sensitive : m_sensitive) delete sensitive;
    }

    /** create the Sensor geometry and assign the sensitive detector implementation.
     * As the sensors are inside the SVD envelope but outside the PXD envelope
     * we have to give both, the top volume and the envelope volume so that the
     * appropriate volume can be chosen dependind on the parameters
     *
     * @param content pointer to the parameters
     * @param topVolume top volume of the simulation
     * @param envelopeVolume envelope volume of the pxd/svd
     */
    void create(const GearDir& content, G4LogicalVolume& topVolume, G4LogicalVolume& envelopeVolume);

  private:
    /** one of "PXD" or "SVD" */
    std::string m_subdetector;
    /** List to all created sensitive detector instances */
    std::vector<Simulation::SensitiveDetectorBase*> m_sensitive;
  };

} //Belle2 namespace
#endif // VXD_GEOMETRY_GEOVXDRADIATONSENSORS_H
