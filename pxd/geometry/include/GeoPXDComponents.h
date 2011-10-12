/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Christian Oswald, Zbynek Drasal,           *
 *               Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOPXDCOMPONENTS_H_
#define GEOPXDCOMPONENTS_H_

#include <pxd/geometry/SensorInfo.h>

#include <string>
#include <vector>

class G4LogicalVolume;

namespace Belle2 {
  /** Namespace to encapsulate code needed for simulation and reconstrucion of the PXD */
  namespace PXD {

    /** Struct holding all parameters to place a subcomponent */
    struct GeoPXDPlacement {
      /** Enum describing where to place the component vertically */
      enum EPosW {
        c_below,  /**< Place the component below the mother */
        c_bottom, /**< Place the component at the bottom of the mother */
        c_center, /**< Place the component at the center of the mother */
        c_top,    /**< Place the component at the top of the mother */
        c_above   /**< Place the component above the mother */
      };
      /** Constructor */
      GeoPXDPlacement(const std::string &name, double u, double v, std::string w);
      std::string name; /**< Name of the component */
      double u;         /**< u coordinate where to place the component */
      double v;         /**< v coordinate where to place the component */
      EPosW w;          /**< w coordinate where to place the component */
    };

    /** Struct holding parameters of the active area of a sensor */
    struct GeoPXDActiveArea {
      /** Constructor */
      GeoPXDActiveArea(double u = 0, double v = 0, double stepSize = 0):
          u(u), v(v), stepSize(stepSize) {}
      double u;         /**< u coordinate where to start the active area */
      double v;         /**< v coordinate where to start the active area */
      double stepSize;  /**< maximum Geant4 steplength inside the active area */
    };

    /** Struct holding all parameters for an component */
    struct GeoPXDComponent {
      /** Constructor */
      GeoPXDComponent(const std::string& material = "", const std::string& color = "",
                      double width = 0, double length = 0, double height = 0):
          volume(0), material(material), color(color), width(width), length(length),
          height(height), flipU(false), flipV(false), flipW(false) {}

      G4LogicalVolume* volume; /**< Pointer to the Logical volume if already created  */
      std::string material;    /**< Name of the material of the component */
      std::string color;       /**< Name of the color of the component */
      double width;            /**< width of the component */
      double length;           /**< length of the component */
      double height;           /**< height of the component */
      bool flipU;              /**< flip subcomponents along u? */
      bool flipV;              /**< flip subcomponents along v? */
      bool flipW;              /**< flip subcomponents along w? */
    };

    /** Struct holding all parameters for a completePXD Sensor */
    struct GeoPXDSensor: public GeoPXDComponent {
      /** Constructor */
      GeoPXDSensor(const std::string& material = "", const std::string& color = "",
                   double width = 0, double length = 0, double height = 0):
          GeoPXDComponent(material, color, width, length, height) {}
      /** ID of the sensor inside the ladder */
      int    sensorID;
      /** global z position of the center of the sensor */
      double z;
      /** Paramerers of the active area */
      GeoPXDActiveArea active;
      /** Sensor Information instance */
      SensorInfo info;
      /** List of all components to be placed */
      std::vector<GeoPXDPlacement> components;
    };

    /** Struct containing all parameters of one ladder */
    struct GeoPXDLadder {
      /** Constructor */
      GeoPXDLadder(int layerID = 0, double shift = 0, double radius = 0,
                   double glueSize = -1, const std::string &glueMaterial = ""):
          layerID(layerID), shift(shift), radius(radius), glueSize(glueSize),
          glueMaterial(glueMaterial) {}
      /** ID of the ladder inside the layer */
      int layerID;
      /** Shift of the ladder along local u */
      double shift;
      /** Radius where to place the ladder */
      double radius;
      /** Oversize of the glue between the ladders, <0 means no glue */
      double glueSize;
      /** name of the Material used as glue */
      std::string glueMaterial;
      /** list of all sensors to be placed in the ladder */
      std::vector<GeoPXDSensor> sensors;
    };

  }
}

#endif /* GEOPXDCOMPONENTS_H_ */
