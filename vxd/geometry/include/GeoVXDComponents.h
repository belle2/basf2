/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Christian Oswald, Zbynek Drasal,           *
 *               Martin Ritter, Peter Kvasnicka, Jozef Koval              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOVXDCOMPONENTS_H
#define GEOVXDCOMPONENTS_H

#include <vxd/geometry/SensorInfoBase.h>

#include <string>
#include <vector>

class G4LogicalVolume;

namespace Belle2 {
  /** Namespace to provide code needed by both Vertex Detectors, PXD and SVD */
  namespace VXD {

    /** Struct holding all parameters to place a subcomponent */
    struct GeoVXDPlacement {
      /** Enum describing where to place the component vertically */
      enum EPosW {
        c_below,  /**< Place the component below the mother */
        c_bottom, /**< Place the component at the bottom of the mother */
        c_center, /**< Place the component at the center of the mother */
        c_top,    /**< Place the component at the top of the mother */
        c_above   /**< Place the component above the mother */
      };
      /** Constructor */
      GeoVXDPlacement(const std::string& name = "", double u = 0, double v = 0, std::string w = "bottom", double woffset = 0);
      std::string name; /**< Name of the component */
      double u;         /**< u coordinate where to place the component */
      double v;         /**< v coordinate where to place the component */
      EPosW w;          /**< w coordinate where to place the component */
      double woffset;   /**< Offset to the w placement of the component */
    };

    /** Struct holding all parameters for an component */
    struct GeoVXDComponent {
      /** Constructor */
      GeoVXDComponent(const std::string& material = "", const std::string& color = "",
                      double width = 0, double width2 = 0, double length = 0, double height = 0):
        volume(0), material(material), color(color), width(width), width2(width2), length(length),
        height(height) {}

      G4LogicalVolume* volume; /**< Pointer to the Logical volume if already created  */
      std::string material;    /**< Name of the material of the component */
      std::string color;       /**< Name of the color of the component */
      double width;            /**< width of the component */
      double width2;           /**< forward width of the senistive area, 0 for recangular */
      double length;           /**< length of the component */
      double height;           /**< height of the component */
    };

    /** Struct holding all parameters for a completeVXD Sensor */
    struct GeoVXDSensor: public GeoVXDComponent {
      /** Constructor */
      GeoVXDSensor(const std::string& material = "", const std::string& color = "",
                   double width = 0, double width2 = 0, double length = 0, double height = 0, bool slanted = false):
        GeoVXDComponent(material, color, width, width2, length, height), info(0), slanted(slanted) {}
      /** Paramerers of the active area */
      GeoVXDComponent activeArea;
      /** Position of the active area */
      GeoVXDPlacement activePlacement;
      /** Sensor Information instance */
      SensorInfoBase* info;
      /** Indication wether this is a slanted sensor */
      bool slanted;
      /** List of all components to be placed */
      std::vector<GeoVXDPlacement> components;
    };

    /** Struct holding the information where a sensor should be placed inside the ladder */
    struct GeoVXDSensorPlacement {
      GeoVXDSensorPlacement(int sensorID = 0, const std::string& sensorTypeID = "", double z = 0,
                            bool flipU = false, bool flipV = false, bool flipW = false):
        sensorID(sensorID), sensorTypeID(sensorTypeID), z(z), flipU(flipU), flipV(flipV), flipW(flipW) {}
      int sensorID;         /**< ID of the sensor inside the ladder */
      std::string sensorTypeID;     /**< Type of the sensor to be used */
      double z;             /**< global z position of the center of the sensor */
      bool flipU;           /**< flip sensor along u? */
      bool flipV;           /**< flip sensor along v? */
      bool flipW;           /**< flip sensor along w? */
    };

    /** Struct containing all parameters of one ladder */
    struct GeoVXDLadder {
      /** Constructor */
      GeoVXDLadder(int layerID = 0, double shift = 0, double radius = 0,
                   double slantedAngle = 0, double slantedRadius = 0,
                   double glueSize = -1, const std::string& glueMaterial = ""):
        layerID(layerID), shift(shift), radius(radius),
        slantedAngle(slantedAngle), slantedRadius(slantedRadius),
        glueSize(glueSize), glueMaterial(glueMaterial) {}
      /** ID of the ladder inside the layer */
      int layerID;
      /** Shift of the ladder along local u */
      double shift;
      /** Radius where to place the ladder */
      double radius;
      /** Angle of the module with respect to the global z-axis for slanted sensors */
      double slantedAngle;
      /** Radius of the center of the active area for slanted sensors */
      double slantedRadius;
      /** Oversize of the glue between the ladders, <0 means no glue */
      double glueSize;
      /** name of the Material used as glue */
      std::string glueMaterial;
      /** list of all sensors to be placed in the ladder */
      std::vector<GeoVXDSensorPlacement> sensors;
    };

  } //namespace VXD

} // namespace Belle2

#endif /* GEOVXDCOMPONENTS_H */
