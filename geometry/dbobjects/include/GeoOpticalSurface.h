/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <vector>
#include <geometry/dbobjects/GeoMaterialProperty.h>

namespace Belle2 {
  /** Represent an optical finish of a surface */
  class GeoOpticalSurface: public TObject {
  public:
    /** empty constructor for ROOT */
    GeoOpticalSurface() = default;
    /** real constructor */
    GeoOpticalSurface(const std::string& name, int model, int finish, int type, double value): TObject(), m_name(name), m_model(model),
      m_finish(finish), m_type(type), m_value(value) {}
    /** set name of the optical surface */
    void setName(const std::string& name) { m_name = name; }
    /** set model for the surface */
    void setModel(int model) { m_model = model; }
    /** set finish of the surface */
    void setFinish(int finish) { m_finish = finish; }
    /** set type of the surface */
    void setType(int type) { m_type = type; }
    /** set value for the surface condition */
    void setValue(double value) { m_value = value; }
    /** add a property to the material */
    void addProperty(const GeoMaterialProperty& property) { m_properties.emplace_back(property); }
    /** get name of the optical surface */
    const std::string& getName() const { return m_name; }
    /** get model for the surface */
    int getModel() const { return m_model; }
    /** get finish of the surface */
    int  getFinish() const { return m_finish; }
    /** get type of the surface */
    int  getType() const { return m_type; }
    /** get value for the surface condition */
    double getValue() const { return m_value; }
    /** get all properties */
    const std::vector<GeoMaterialProperty>& getProperties() const { return m_properties; }
    /** check if the material has at least one property
     * @returns true if there is at least one property */
    bool hasProperties() const { return m_properties.size() > 0; }
  private:
    /** name of the optical surface */
    std::string m_name;
    /** model for the surface */
    int m_model;
    /** finish of the surface */
    int m_finish;
    /** type of the surface */
    int m_type;
    /** value for the surface condition */
    double m_value;
    /** properties of this material */
    std::vector<GeoMaterialProperty> m_properties;
    /** Add ROOT reflection info */
    ClassDef(GeoOpticalSurface, 1);
  };
}; // Belle2 namespace
