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

#include <string>
#include <vector>

class G4LogicalVolume;

namespace Belle2 {
  namespace pxd {

    struct GeoPXDPlacement {
      enum EPosW { c_below, c_bottom, c_center, c_top, c_above };
      GeoPXDPlacement(const std::string &name, double u, double v, std::string w);
      std::string name;
      double u;
      double v;
      EPosW w;
    };

    struct GeoPXDActiveArea {
      GeoPXDActiveArea(double u = 0, double v = 0, double width = 0, double length = 0, double height = 0, double stepSize = 0):
          u(u), v(v), width(width), length(length), height(height), stepSize(stepSize) {}
      double u;
      double v;
      double width;
      double length;
      double height;
      double stepSize;
    };

    struct GeoPXDComponent {
      GeoPXDComponent(const std::string& material = "", const std::string& color = "", double width = 0, double length = 0, double height = 0):
          volume(0), material(material), color(color), width(width), length(length), height(height), flipU(false), flipV(false), flipW(false) {}

      G4LogicalVolume* volume;
      std::string material;
      std::string color;
      double width;
      double length;
      double height;
      bool flipU;
      bool flipV;
      bool flipW;
    };

    struct GeoPXDSensor: public GeoPXDComponent {
      GeoPXDSensor(const std::string& material = "", const std::string& color = "", double width = 0, double length = 0, double height = 0):
          GeoPXDComponent(material, color, width, length, height) {}
      int    sensorID;
      double z;
      GeoPXDActiveArea active;
      std::vector<GeoPXDPlacement> components;
    };

    struct GeoPXDLadder {
      GeoPXDLadder(int layerID = 0, double shift = 0, double radius = 0): layerID(layerID), shift(shift), radius(radius) {}
      int layerID;
      double shift;
      double radius;
      std::vector<GeoPXDSensor> sensors;
    };

  }
}

#endif /* GEOPXDCOMPONENTS_H_ */
