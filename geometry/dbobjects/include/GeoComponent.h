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
  /** Describe one component of the Geometry */
  class GeoComponent: public TObject {
  public:
    /** empty constructor for ROOT */
    GeoComponent() = default;
    /** full constructor */
    GeoComponent(const std::string& name, const std::string& creator, const std::string& library): TObject(),
      m_name(name), m_creator(creator), m_library(library) {}
    /** set name of the component */
    void setName(const std::string& name) { m_name = name; }
    /** set name of the creator used to construct the component */
    void setCreator(const std::string& creator) { m_creator = creator; }
    /** set name of the library to load for the creator */
    void setLibrary(const std::string& library) { m_library = library; }
    /** get name of the component */
    const std::string& getName() const { return m_name; }
    /** get name of the creator used to construct the component */
    const std::string& getCreator() const { return m_creator; }
    /** get name of the library to load for the creator */
    const std::string& getLibrary() const { return m_library; }
  private:
    /** name of the component */
    std::string m_name;
    /** name of the creator used to construct the component */
    std::string m_creator;
    /** name of the library to load for the creator */
    std::string m_library;
    /** Add ROOT reflection info */
    ClassDef(GeoComponent, 1);
  };
}; // Belle2 namespace
