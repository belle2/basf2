/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <geometry/dbobjects/GeoMaterial.h>
#include <geometry/dbobjects/GeoComponent.h>
#include <vector>


namespace Belle2 {
  /** configuration of the geometry */
  class GeoConfiguration: public TObject {
  public:
    /** empty constructor for ROOT */
    GeoConfiguration() = default;
    /** Construct new geometry configuration
     * @param name name of the detector
     * @param width half width of the global volume
     * @param height half height of the global volume
     * @param length half length of the global volume
     * @param material material of the global volume
     */
    GeoConfiguration(const std::string& name, double width, double height, double length, const std::string& material): TObject(),
      m_name(name), m_globalWidth(width), m_globalHeight(height), m_globalLength(length), m_globalMaterial(material) {}
    /** add a material to the list of materials */
    void addMaterial(const GeoMaterial& material) { m_materials.emplace_back(material); }
    /** add a component to the list of components */
    void addComponent(const GeoComponent& component) { m_components.emplace_back(component); }
    /** get the name of the detector */
    const std::string& getName() const { return m_name; }
    /** get the half width of the global volume, that is length in + and - x direction.
     * Will be determined automatically to fit all volumes if set to non-positive value. */
    double getGlobalWidth() const { return m_globalWidth; }
    /** get the half height of the global volume, that is length in + and - y direction.
     * Will be determined automatically to fit all volumes if set to non-positive value. */
    double getGlobalHeight() const { return m_globalHeight; }
    /** get the half length of the global volume, that is length in + and - z direction.
     * Will be determined automatically to fit all volumes if set to non-positive value. */
    double getGlobalLength() const { return m_globalLength; }
    /** get the material of the global volume */
    const std::string& getGlobalMaterial() const { return m_globalMaterial; }
    /** get List of Materials */
    const std::vector<GeoMaterial>& getMaterials() const { return m_materials; }
    /** get list of components */
    const std::vector<GeoComponent>& getComponents() const { return m_components; }
  private:
    /** list of Materials */
    std::vector<GeoMaterial> m_materials;
    /** list of components */
    std::vector<GeoComponent> m_components;
    /** name of the detector configuration */
    std::string m_name;
    /** half width of the global volume */
    double m_globalWidth{0};
    /** half height of the global volume */
    double m_globalHeight{0};
    /** half length of the global volume */
    double m_globalLength{0};
    /** material of the global volume */
    std::string m_globalMaterial;
    /** Add ROOT reflection info */
    ClassDef(GeoConfiguration, 1);
  };
}; // Belle2 namespace
