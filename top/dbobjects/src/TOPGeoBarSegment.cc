/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dbobjects/TOPGeoBarSegment.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <math.h>
#include <iostream>

using namespace std;

namespace Belle2 {

  bool TOPGeoBarSegment::isConsistent() const
  {
    if (m_width <= 0) return false;
    if (m_thickness <= 0) return false;
    if (m_length <= 0) return false;
    if (m_material.empty()) return false;
    if (m_glueThickness <= 0) return false;
    if (m_glueMaterial.empty()) return false;
    if (m_surface.getName().empty() and !m_surface.hasProperties()) return false;
    if (m_sigmaAlpha < 0) return false;
    if (m_brokenFraction > 0 and m_brokenGlueMaterial.empty()) return false;
    return true;
  }


  void TOPGeoBarSegment::print(const std::string& title) const
  {
    TOPGeoBase::print(title);
    cout << " Vendor: " << getVendor() << ", serial number: " << getSerialNumber() << endl;
    cout << " Dimensions: " << getWidth() << " X " << getThickness() << " X " << getLength()
         << " " << s_unitName << endl;
    cout << " Material: " << getMaterial() << endl;
    cout << " Glue: " << getGlueMaterial() << ", thickness = " << getGlueThickness()
         << " " << s_unitName;
    cout << ", broken fraction = " << getBrokenGlueFraction();
    if (getBrokenGlueFraction() > 0) {
      cout << ", angle = " << getBrokenGlueAngle() / Unit::deg << " deg";
      cout << ", material = " << getBrokenGlueMaterial();
    }
    cout << endl;
    printSurface(m_surface);
    cout << "  - sigmaAlpha: " << getSigmaAlpha() << endl;
  }

  typedef std::pair<double, double> Pair; /**< Shorthand for std::pair<double, double> */

  std::vector<std::pair<double, double> > TOPGeoBarSegment::getBrokenGlueContour() const
  {

    std::vector<Pair> contour;
    constructContour(getWidth() / 2, getThickness() / 2, m_brokenFraction, m_brokenAngle,
                     contour);

    return contour;
  }


  void TOPGeoBarSegment::constructContour(double A, double B, double fraction, double angle,
                                          std::vector<Pair>& contour) const
  {
    if (fraction <= 0) {
      return;
    }
    if (fraction >= 1) {
      contour.push_back(Pair(-A, B));
      contour.push_back(Pair(A, B));
      contour.push_back(Pair(A, -B));
      contour.push_back(Pair(-A, -B));
      return;
    }

    double alpha0 = atan(B / A);
    double halfPi = alpha0 > 0 ? M_PI / 2 : -M_PI / 2;
    int quadrant = 0;
    while (fabs(angle) > alpha0) {
      angle -= halfPi;
      alpha0 = M_PI / 2 - alpha0;
      double a = A;
      A = B;
      B = a;
      quadrant++;
    }
    quadrant = quadrant % 4;
    if (halfPi < 0) quadrant = (4 - quadrant) % 4;

    double tanAngle = tan(fabs(angle));
    double S0 = A / B * tanAngle / 2;
    if (fraction <= S0) {
      double x = sqrt(8 * A * B * fraction / tanAngle);
      double y = x * tanAngle;
      if (angle > 0) {
        contour.push_back(Pair(-A, B - y));
        contour.push_back(Pair(-A, B));
        contour.push_back(Pair(-A + x, B));
      } else {
        contour.push_back(Pair(A, B - y));
        contour.push_back(Pair(A - x, B));
        contour.push_back(Pair(A, B));
      }
    } else if (1 - fraction <= S0) {
      double x = sqrt(8 * A * B * (1 - fraction) / tanAngle);
      double y = x * tanAngle;
      if (angle > 0) {
        contour.push_back(Pair(A, -B + y));
        contour.push_back(Pair(A - x, -B));
        contour.push_back(Pair(-A, -B));
        contour.push_back(Pair(-A, B));
        contour.push_back(Pair(A, B));
      } else {
        contour.push_back(Pair(-A + x, -B));
        contour.push_back(Pair(-A, -B + y));
        contour.push_back(Pair(-A, B));
        contour.push_back(Pair(A, B));
        contour.push_back(Pair(A, -B));
      }
    } else {
      double y = (1 - 2 * fraction) * B;
      double dy = angle > 0 ? A * tanAngle : -A * tanAngle;
      contour.push_back(Pair(-A, y  - dy));
      contour.push_back(Pair(-A, B));
      contour.push_back(Pair(A, B));
      contour.push_back(Pair(A, y  + dy));
    }

    switch (quadrant) {
      case 0:
        break;
      case 1:
        for (auto& point : contour) {
          double x = -point.second;
          double y = point.first;
          point.first = x;
          point.second = y;
        }
        break;
      case 2:
        for (auto& point : contour) {
          double x = -point.first;
          double y = -point.second;
          point.first = x;
          point.second = y;
        }
        break;
      case 3:
        for (auto& point : contour) {
          double x = point.second;
          double y = -point.first;
          point.first = x;
          point.second = y;
        }
        break;
      default:
        B2ERROR("TOPGeoBarSegment::constructContour: bug!");
    }

  }

} // end Belle2 namespace
