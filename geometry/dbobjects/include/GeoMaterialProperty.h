/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>

namespace Belle2 {
  /** Property of a material */
  class GeoMaterialProperty: public TObject {
  public:
    /** default constructor */
    GeoMaterialProperty() = default;
    /** full constructor */
    GeoMaterialProperty(const std::string& name, const std::vector<double>& energies, const std::vector<double>& values):
      m_name(name), m_energies(energies), m_values(values) {}
    /** set name of the property */
    void setName(const std::string& name) { m_name = name; }
    /** set energies for the property */
    void setEnergies(const std::vector<double>& energies) { m_energies = energies; }
    /** set values at the given energies */
    void setValues(const std::vector<double>& values) { m_values = values; }
    /** get name of the property */
    const std::string& getName() const { return m_name; }
    /** get energies for the property */
    const std::vector<double>& getEnergies() const { return m_energies; }
    /** get values at the given energies */
    const std::vector<double>& getValues() const { return m_values; }
    /** get the number of values */
    int getN() const { return m_energies.size(); }
  private:
    /** name of the property */
    std::string m_name;
    /** energies for the property */
    std::vector<double> m_energies;
    /** values at the given energies */
    std::vector<double> m_values;
    /** Add ROOT reflection info */
    ClassDef(GeoMaterialProperty, 1);
  };
}; // Belle2 namespace
