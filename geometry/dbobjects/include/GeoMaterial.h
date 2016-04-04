/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <vector>
#include <geometry/dbobjects/GeoMaterialProperty.h>
#include <geometry/dbobjects/GeoMaterialComponent.h>

namespace Belle2 {
  /** Class to represent a material informaion in the Database */
  class GeoMaterial: public TObject {
  public:
    /** default constructor */
    GeoMaterial() = default;
    /** set the name of the material */
    void setName(const std::string& name) { m_name = name; }
    /** set the state of the material */
    void setState(int state) { m_state = state; }
    /** set the density of the material (in default framework units */
    void setDensity(double density) { m_density = density; }
    /** set the temperature of the material (in default framework units */
    void setTemperature(double temperature) { m_temperature = temperature; }
    /** set the pressure of the material (in default framework units */
    void setPressure(double pressure) { m_pressure = pressure; }
    /** add a component to the material. Fall fractions should add up to one but this is not verified */
    void addComponent(const GeoMaterialComponent& component) { m_components.emplace_back(component); }
    /** add a property to the material */
    void addProperty(const GeoMaterialProperty& property) { m_properties.emplace_back(property); }
    /** get the name of the material */
    const std::string& getName() const { return m_name; }
    /** get the state of the material */
    int getState() const { return m_state; }
    /** get the density of the material (in default framework units */
    double getDensity() const { return m_density; }
    /** get the temperature of the material (in default framework units */
    double getTemperature() const { return m_temperature; }
    /** get the pressure of the material (in default framework units */
    double getPressure() const { return m_pressure; }
    /** get all components */
    std::vector<GeoMaterialComponent>& getComponents() { return m_components; }
    /** get all components */
    const std::vector<GeoMaterialComponent>& getComponents() const { return m_components; }
    /** get all properties */
    const std::vector<GeoMaterialProperty>& getProperties() const { return m_properties; }
    /** check if the material has at least one property
     * @returns true if there is at least one property */
    bool hasProperties() const { return m_properties.size() > 0; }
  private:
    /** name of the material */
    std::string m_name{""};
    /** Geant4 state of the material */
    int m_state{0};
    /** density of the material */
    double m_density{0};
    /** temperature of the material */
    double m_temperature{0};
    /** pressure of the material */
    double m_pressure{0};
    /** components of this material */
    std::vector<GeoMaterialComponent> m_components;
    /** properties of this material */
    std::vector<GeoMaterialProperty> m_properties;
    /** reflection info for ROOT */
    ClassDef(GeoMaterial, 1);
  };
} //Belle2 namespace
