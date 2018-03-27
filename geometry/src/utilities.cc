/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <framework/gearbox/GearDir.h>
#include <geometry/utilities.h>
#include <geometry/GeometryManager.h>

#include <boost/algorithm/string.hpp>
#include <boost/tuple/tuple.hpp>

#include <G4Colour.hh>
#include <G4LogicalVolume.hh>
#include <G4VisAttributes.hh>
#include <G4Polycone.hh>

#include <vector>
#include <limits>
#include <list>

using namespace std;

namespace Belle2 {
  namespace geometry {
    /** Parse a color string of the form #rgb, #rrggbb, #rgba, #rrggbbaa or
     * rgb(r, g, b) and return a corresponding G4Colour.
     *
     * For #rgb and #rgba the color for red, green, blue (and optionally alpha)
     * is each represented by one hexadecimal digit, #rrggbb and #rrggbbaa is
     * the same with two digits per colour.
     *
     * rgb(r, g, b) expects the fraction of red, green and blue as float between 0 and 1.
     */
    G4Colour parseColor(string colorString)
    {
      boost::to_lower(colorString);
      double red(0), green(0), blue(0), alpha(0);
      if (colorString[0] == '#') {
        size_t size = colorString.size();
        unsigned int colorValue;
        stringstream in(colorString);
        in.get();
        in >> std::hex >> colorValue;
        if (in.fail()) size = 0;
        switch (size) {
          case 4: //#rgb, add alpha since none was specified
            colorValue = (colorValue << 4) + 15;
            // and then continue with rgba case
            [[fallthrough]];
          case 5: //#rgba
            red   = ((colorValue & 0xf000) >> 12) / 15.;
            green = ((colorValue & 0x0f00) >>  8) / 15.;
            blue  = ((colorValue & 0x00f0) >>  4) / 15.;
            alpha = ((colorValue & 0x000f) >>  0) / 15.;
            break;
          case 7: //#rrggbb, add alpha since none was specified
            colorValue = (colorValue << 8) + 255;
            // and then continue with #rrggbbaa case
            [[fallthrough]];
          case 9: //#rrggbbaa
            red   = ((colorValue & 0xff000000) >> 24) / 255.;
            green = ((colorValue & 0x00ff0000) >> 16) / 255.;
            blue  = ((colorValue & 0x0000ff00) >>  8) / 255.;
            alpha = ((colorValue & 0x000000ff) >>  0) / 255.;
            break;
          default:
            B2WARNING("Could not parse color string '" + colorString + "'" << endl);
        }
      } else if (colorString.substr(0, 3) == "rgb") {
        //Parse value of the type rgb(1.0,1.0,1.0)
        size_t startPos = colorString.find("(");
        size_t stopPos = colorString.find(")");
        string ws(" \t\r\n,");
        stringstream in(colorString.substr(startPos + 1, stopPos - startPos - 1));
        in >> red;
        while (ws.find(in.peek()) != string::npos) in.get();
        in >> green;
        while (ws.find(in.peek()) != string::npos) in.get();
        in >> blue;
        while (ws.find(in.peek()) != string::npos) in.get();
        in >> alpha;
        red   = min(1.0, max(0.0, red));
        green = min(1.0, max(0.0, green));
        blue  = min(1.0, max(0.0, blue));
        alpha = min(1.0, max(0.0, alpha));
      }
      return G4Colour(red, green, blue, alpha);
    }

    void setColor(G4LogicalVolume& volume, const string& color)
    {
      G4VisAttributes* attr = const_cast<G4VisAttributes*>(volume.GetVisAttributes());
      if (!attr) attr = GeometryManager::getInstance().newVisAttributes();
      attr->SetColor(parseColor(color));
      volume.SetVisAttributes(attr);
    }
    void setVisibility(G4LogicalVolume& volume, bool visible)
    {
      G4VisAttributes* attr = const_cast<G4VisAttributes*>(volume.GetVisAttributes());
      if (!attr) attr = GeometryManager::getInstance().newVisAttributes();
      attr->SetVisibility(visible);
      volume.SetVisAttributes(attr);
    }

    G4Polycone* createPolyCone(const string& name, const GearDir& params, double& minZ, double& maxZ)
    {
      if (!params) return 0;

      double minPhi = params.getAngle("minPhi", 0);
      double dPhi   = params.getAngle("maxPhi", 2 * M_PI) - minPhi;
      const std::vector<GearDir> planes = params.getNodes("Plane");
      int nPlanes = planes.size();
      if (nPlanes < 2) {
        B2ERROR("Polycone needs at least two planes");
        return 0;
      }
      std::vector<double> z(nPlanes, 0);
      std::vector<double> rMin(nPlanes, 0);
      std::vector<double> rMax(nPlanes, 0);
      int index(0);
      minZ = numeric_limits<double>::infinity();
      maxZ = -numeric_limits<double>::infinity();
      for (const GearDir& plane : planes) {
        z[index]    = plane.getLength("posZ") / Unit::mm;
        minZ = min(minZ, z[index]);
        maxZ = max(maxZ, z[index]);
        rMin[index] = plane.getLength("innerRadius") / Unit::mm;
        rMax[index] = plane.getLength("outerRadius") / Unit::mm;
        ++index;
      }
      G4Polycone* polycone = new G4Polycone(name, minPhi, dPhi, nPlanes, z.data(), rMin.data(), rMax.data());
      return polycone;
    }

    //Use unnamed namespace to hide helper functions/definitions from outside
    namespace {
      /** Struct representing the Z and X coordinate of a point */
      typedef pair<double, double> ZXPoint;
      /** List of points in the ZX plane */
      typedef list<ZXPoint> PointList;
      /** Helper function for createRotationSolid.
       * This function subdivides the polyline given by segments to contain a
       * point at every z position in points.  Furthermore, the polyline will
       * be extended to cover the same range of z as the points by appending
       * the first or last point of points to the polyline if neccessary.
       *
       * This function also checks if the points have ascending z positions.
       *
       * @param points List of points which z positions should be included in
       *        segments
       * @param[inout] segments List of points which forms a polyline. Points
       *        will be added to this list
       */
      void subdivideSegments(const PointList& points, PointList& segments)
      {
        double lastZ = -numeric_limits<double>::infinity();
        for (const ZXPoint p : points) {
          if (p.first < lastZ) {
            B2FATAL("createRotationSolid: Points have to be given with ascending z positions");
          }
          lastZ = p.first;
          //Now go over all segments. If the segments cross the points z
          //coordinate we know that we need a new segment. We calculate the x
          //position of the segment and insert a new point at that position in
          //the list of points
          PointList::iterator segStart = segments.begin();
          PointList::iterator segEnd = segStart;
          ++segEnd;
          for (; segEnd != segments.end(); ++segEnd) {
            if ((p.first > segStart->first && p.first < segEnd->first) ||
                (p.first < segStart->first && p.first > segEnd->first)) {
              double dZ = segEnd->first - segStart->first;
              double dX = segEnd->second - segStart->second;
              double newX = segStart->second + dX * (p.first - segStart->first) / dZ;
              segments.insert(segEnd, ZXPoint(p.first, newX));
            }
            segStart = segEnd;
          }
        }

        //Now make sure the polyline extend over the same z range by adding the
        //first/last point of points when neccessary
        if (points.front().first < segments.front().first) {
          segments.insert(segments.begin(), points.front());
        }
        if (points.back().first > segments.back().first) {
          segments.insert(segments.end(), points.back());
        }
      }
    }

    G4Polycone* createRotationSolid(const std::string& name, const GearDir& params, double& minZ, double& maxZ)
    {

      //Make a list of all the points (ZX coordinates)
      PointList innerPoints;
      PointList outerPoints;
      for (const GearDir point : params.getNodes("InnerPoints/point")) {
        innerPoints.push_back(ZXPoint(point.getLength("z") / Unit::mm, point.getLength("x") / Unit::mm));
      }
      for (const GearDir point : params.getNodes("OuterPoints/point")) {
        outerPoints.push_back(ZXPoint(point.getLength("z") / Unit::mm, point.getLength("x") / Unit::mm));
      }
      //Subdivide the segments to have an x position for each z specified for
      //either inner or outer boundary
      subdivideSegments(innerPoints, outerPoints);
      subdivideSegments(outerPoints, innerPoints);
      minZ = innerPoints.front().first;
      maxZ = outerPoints.front().first;

      //Now we create the array of planes needed for the polycone
      vector<double> z;
      vector<double> rMin;
      vector<double> rMax;

      double innerZ{0};
      double innerX{0};
      double outerZ{0};
      double outerX{0};
      //We go through both lists until both are empty
      while (!(innerPoints.empty() && outerPoints.empty())) {
        bool popInner = false;
        //We could have more than one point at the same z position for segments
        //going directly along x. because of that we check that the z
        //coordinates for inner and outer line are always the same, reusing one
        //point if neccessary
        if (!innerPoints.empty() && innerPoints.front().first <= outerPoints.front().first) {
          boost::tie(innerZ, innerX) = innerPoints.front();
          popInner = true;
        }
        if (!outerPoints.empty() && outerPoints.front().first <= innerPoints.front().first) {
          boost::tie(outerZ, outerX) = outerPoints.front();
          outerPoints.pop_front();
        }
        if (popInner) innerPoints.pop_front();
        if (innerZ != outerZ) {
          B2ERROR("createRotationSolid: Something is wrong, z values should be identical");
          return 0;
        }
        z.push_back(innerZ);
        rMin.push_back(innerX);
        rMax.push_back(outerX);
      }

      //Finally create the Polycone
      int nPlanes = z.size();
      double minPhi = params.getAngle("minPhi", 0);
      double dPhi   = params.getAngle("maxPhi", 2 * M_PI) - minPhi;
      return new G4Polycone(name, minPhi, dPhi, nPlanes, &z.front(), &rMin.front(), &rMax.front());
    }

    G4Polycone* createRotationSolid(const std::string& name,
                                    std::list< std::pair<double, double> > innerPoints,
                                    std::list< std::pair<double, double> > outerPoints,
                                    double minPhi, double maxPhi, double& minZ, double& maxZ)
    {
      //Subdivide the segments to have an x position for each z specified for
      //either inner or outer boundary
      subdivideSegments(innerPoints, outerPoints);
      subdivideSegments(outerPoints, innerPoints);
      minZ = innerPoints.front().first;
      maxZ = outerPoints.front().first;

      //Now we create the array of planes needed for the polycone
      vector<double> z;
      vector<double> rMin;
      vector<double> rMax;

      double innerZ{0};
      double innerX{0};
      double outerZ{0};
      double outerX{0};
      //We go through both lists until both are empty
      while (!(innerPoints.empty() && outerPoints.empty())) {
        bool popInner = false;
        //We could have more than one point at the same z position for segments
        //going directly along x. because of that we check that the z
        //coordinates for inner and outer line are always the same, reusing one
        //point if neccessary
        if (!innerPoints.empty() && innerPoints.front().first <= outerPoints.front().first) {
          boost::tie(innerZ, innerX) = innerPoints.front();
          popInner = true;
        }
        if (!outerPoints.empty() && outerPoints.front().first <= innerPoints.front().first) {
          boost::tie(outerZ, outerX) = outerPoints.front();
          outerPoints.pop_front();
        }
        if (popInner) innerPoints.pop_front();
        if (innerZ != outerZ) {
          B2ERROR("createRotationSolid: Something is wrong, z values should be identical");
          return 0;
        }
        z.push_back(innerZ / Unit::mm);
        rMin.push_back(innerX / Unit::mm);
        rMax.push_back(outerX / Unit::mm);
      }

      //Finally create the Polycone
      int nPlanes = z.size();
      double dPhi = maxPhi - minPhi;
      return new G4Polycone(name, minPhi, dPhi, nPlanes, &z.front(), &rMin.front(), &rMax.front());

    }


  }
} //Belle2 namespace
