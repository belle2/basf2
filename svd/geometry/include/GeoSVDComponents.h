/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Peter Kvasnicka, Jozef Koval              *
 *                                                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOSVDCOMPONENTS_H
#define GEOSVDCOMPONENTS_H

#include <svd/geometry/SensorInfo.h>

#include <string>
#include <vector>

class G4LogicalVolume;

namespace Belle2 {
  namespace SVD {

    struct GeoSVDPlacement {
      enum EPosW { c_below, c_bottom, c_center, c_top, c_above };
      GeoSVDPlacement(const std::string& name, double u, double v, std::string w);
      std::string name;
      double u;
      double v;
      EPosW w;
    };

    struct GeoSVDActiveArea {
      GeoSVDActiveArea(double u = 0, double v = 0, double width = 0, double width2 = 0, double length = 0, double height = 0, double stepSize = 0):
        u(u), v(v), width(width), width2(width2), length(length), height(height), stepSize(stepSize) {}
      double u;
      double v;
      double width;
      double width2;
      double length;
      double height;
      double stepSize;
    };

    struct GeoSVDComponent {
      GeoSVDComponent(const std::string& material = "", const std::string& color = "", double width = 0, double width2 = 0, double length = 0, double height = 0):
        volume(0), material(material), color(color), width(width), width2(width2), length(length), height(height), flipU(false), flipV(false), flipW(false) {}

      G4LogicalVolume* volume;
      std::string material;
      std::string color;
      double width;
      double width2;
      double length;
      double height;
      bool flipU;
      bool flipV;
      bool flipW;
    };

    struct GeoSVDSensor: public GeoSVDComponent {
      GeoSVDSensor(const std::string& material = "", const std::string& color = "", double width = 0, double width2 = 0, double length = 0, double height = 0):
        GeoSVDComponent(material, color, width, width2, length, height) {}
      SensorInfo info;
      int    sensorID;
      int    sensorTypeID;
      double z;
      GeoSVDActiveArea active;
      std::vector<GeoSVDPlacement> components;
    };

    struct GeoSVDLadder {
      GeoSVDLadder(int layerID = 0, double shift = 0, double radius = 0, double slantedAngle = 0, double slantedRadius = 0): layerID(layerID), shift(shift), radius(radius), slantedAngle(slantedAngle), slantedRadius(slantedRadius) {}
      int layerID;
      double shift;
      double radius;
      double slantedAngle;
      double slantedRadius;
      std::vector<GeoSVDSensor> sensors;
    };

  }  //namespace svd

}  // namespace Belle2

#endif /* GEOSVDCOMPONENTS_H */
