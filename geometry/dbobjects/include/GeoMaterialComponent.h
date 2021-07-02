/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>

namespace Belle2 {
  /** Component of a material */
  class GeoMaterialComponent: public TObject {
  public:
    /** default constructor */
    GeoMaterialComponent() = default;
    /** full constructor */
    GeoMaterialComponent(const std::string& name, bool isElement, double fraction):
      m_name(name), m_isElement(isElement), m_fraction(fraction) {}
    /** set name of the component */
    void setName(const std::string& name) { m_name = name; }
    /** set true if the component is a chemical element, false if it is a composite material */
    void setIselement(bool isElement) { m_isElement = isElement; }
    /** set fraction of the component from the full materials */
    void setFraction(double fraction) { m_fraction = fraction; }
    /** get name of the component */
    const std::string& getName() const { return m_name; }
    /** get true if the component is a chemical element, false if it is a composite material */
    bool getIselement() const { return m_isElement; }
    /** get fraction of the component from the full materials */
    double getFraction() const { return m_fraction; }
  private:
    /** name of the component */
    std::string m_name;
    /** true if the component is a chemical element, false if it is a composite material */
    bool m_isElement;
    /** fraction of the component from the full materials */
    double m_fraction;
    /** Add ROOT reflection info */
    ClassDef(GeoMaterialComponent, 1);
  };
}; // Belle2 namespace
