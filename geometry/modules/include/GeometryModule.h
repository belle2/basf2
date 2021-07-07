/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <geometry/dbobjects/GeoConfiguration.h>

namespace Belle2 {
  /** Module to faciliate Geometry creation. */
  class GeometryModule: public Module {
  public:
    /** Constructor */
    GeometryModule();
    /** Destructor */
    ~GeometryModule() {};
    /** Create geometry */
    void initialize() override;
    /** Clean up the geometry */
    void terminate() override;
  protected:
    /** Path for the geometry in the parameter space */
    std::string m_geometryPath;
    /** Type of the geometry to be built */
    int m_geometryType;
    /** Components to be created, if empty all components will be created */
    std::vector<std::string> m_components;
    /** Components to excluded from creation */
    std::vector<std::string> m_excluded;
    /** Components to be added in addition to the default ones */
    std::vector<std::string> m_additional;
    /** Whether or not a region should be assigned to all volumes created by a given creator */
    bool m_assignRegions {false};
    /** Whether or not this module will raise an error if the geometry is
     * already present. This can be used to add the geometry multiple times if
     * it's not clear if it's already present in another path */
    bool m_ignoreIfPresent{false};
    /** Whether or not to build the geometry from the database */
    bool m_useDB{true};
    /** Payload iov when creating a geometry configuration */
    std::vector<int> m_payloadIov{0, 0, -1, -1};
    /** If true we need to create a payload */
    bool m_createGeometryPayload{false};
    /** Database object pointing to the geometry configuration in case we load
     * the geometry from database */
    DBObjPtr<GeoConfiguration>* m_geometryConfig{nullptr};
  };
} //Belle2 namespace
