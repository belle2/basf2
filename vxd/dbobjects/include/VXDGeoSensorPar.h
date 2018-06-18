/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once


#include <TObject.h>
#include <string>
#include <vector>


#include <vxd/dbobjects/VXDSensorInfoBasePar.h>
#include <vxd/dbobjects/VXDGeoComponentPar.h>
#include <vxd/dbobjects/VXDGeoPlacementPar.h>

namespace Belle2 {



  /**
  * The Class for VXD Sensor payload
  */
  class VXDGeoSensorPar: public VXDGeoComponentPar {
  public:
    /** Constructor */
    VXDGeoSensorPar(const std::string& material = "", const std::string& color = "",
                    double width = 0, double width2 = 0, double length = 0, double height = 0, double angle = 0, bool slanted = false):
      VXDGeoComponentPar(material, color, width, width2, length, height, angle), m_info(0), m_slanted(slanted) {}
    //! Destructor
    ~VXDGeoSensorPar() {}
    /** set the active area
      * @param area component description of the active area
      * @param placement placement description of the active area
      */
    void setActive(const VXDGeoComponentPar& area, const VXDGeoPlacementPar& placement)
    {
      m_activeArea = area; m_activePlacement = placement;
    }
    /** get the component description for the active area */
    const VXDGeoComponentPar& getActiveArea() const { return m_activeArea; }
    /** get the placement description for the active area */
    const VXDGeoPlacementPar& getActivePlacement()  const { return m_activePlacement; }
    /** set the list of sub components to be placed */
    void setComponents(const std::vector<VXDGeoPlacementPar>& component) { m_components = component; }
    /** get the list of sub components */
    const std::vector<VXDGeoPlacementPar>& getComponents() const { return m_components; }
    /** set the pointer to the SensorInfo class */
    void setSensorInfo(VXDSensorInfoBasePar* info) { m_info = info; }
    /** get the pointer to the SensorInfo class */
    const VXDSensorInfoBasePar* getSensorInfo() const { return m_info; }
    /** return wether or not the sensor is slanted (usually only the first sensor in layers 4-6) */
    bool getSlanted() const { return m_slanted; }
    /** set wether or not the sensor is slanted (usually only the first sensor in layers 4-6) */
    void setSlanted(bool slanted) { m_slanted = slanted; }
  private:
    /** Paramerers of the active area */
    VXDGeoComponentPar m_activeArea;
    /** Position of the active area */
    VXDGeoPlacementPar m_activePlacement;
    /** Sensor Information instance */
    VXDSensorInfoBasePar* m_info;
    /** Indication wether this is a slanted sensor */
    bool m_slanted;
    /** list of all components to be placed */
    std::vector<VXDGeoPlacementPar> m_components;

    ClassDef(VXDGeoSensorPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

