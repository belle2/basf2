/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dbobjects/TOPGeoPrism.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <cmath>
#include <iostream>
#include <algorithm>

using namespace std;

namespace Belle2 {

  TOPGeoPrism::UnfoldedWindow::UnfoldedWindow(const TVector2& orig, const TVector2& dir,
                                              const TVector2& norm, const TVector2& slanted):
    y0(orig.X()), z0(orig.Y()), sy(dir.X()), sz(dir.Y()), ny(norm.X()), nz(norm.Y())
  {
    nsy[0] = sy;
    nsz[0] = sz;
    nsy[1] = slanted.X();
    nsz[1] = slanted.Y();
  }


  bool TOPGeoPrism::isConsistent() const
  {
    if (m_exitThickness <= 0) return false;
    if (m_flatLength < 0) return false;
    if (m_width <= 0) return false;
    if (m_thickness <= 0) return false;
    if (m_length <= 0) return false;
    if (m_material.empty()) return false;
    if (m_surface.getName().empty() and !m_surface.hasProperties()) return false;
    if (m_sigmaAlpha < 0) return false;
    if (m_brokenFraction > 0 and m_brokenGlueMaterial.empty()) return false;
    if (!m_peelOffRegions.empty()) {
      if (m_peelOffSize <= 0) return false;
      if (m_peelOffThickness <= 0) return false;
      if (m_peelOffMaterial.empty()) return false;
      double halfSize = (getWidth() - getPeelOffSize()) / 2;
      for (const auto& region : m_peelOffRegions) {
        if (fabs(getPeelOffCenter(region)) > halfSize) return false;
      }
    }
    return true;
  }


  void TOPGeoPrism::print(const std::string& title) const
  {
    TOPGeoBase::print(title);
    cout << " Vendor: " << getVendor() << ", serial number: " << getSerialNumber() << endl;
    cout << " Dimensions: " << getWidth() << " X " << getThickness() << " X " << getLength()
         << " " << s_unitName << endl;
    cout << " Exit window dimensions: " << getWidth() << " X "
         << getExitThickness() << " " << s_unitName << endl;
    cout << " Prism angle: " << getAngle() / Unit::deg << " deg";
    cout << ", flat surface length: " << getFlatLength() << " " << s_unitName << endl;
    cout << " Material: " << getMaterial() << endl;
    if (getFilterThickness() > 0) { // old payload
      cout << " Wavelenght filter: " << getFilterMaterial()
           << ", thickness: " << getFilterThickness() << " " << s_unitName << endl;
    }
    if (!m_peelOffRegions.empty()) {
      cout << " Peel-off cookie regions: ";
      cout << " size = " << getPeelOffSize() << " " << s_unitName;
      cout << " thickness = " << getPeelOffThickness() << " " << s_unitName;
      cout << " material = " << getPeelOffMaterial() << endl;
      for (const auto& region : m_peelOffRegions) {
        cout << "   ID = " << region.ID << ", fraction = " << region.fraction
             << ", angle = " << region.angle  / Unit::deg << " deg"
             << ", xc = " << getPeelOffCenter(region) << " " << s_unitName << endl;
      }
    } else {
      cout << " Peel-off regions: None" << endl;
    }
    printSurface(m_surface);
    cout << "  - sigmaAlpha: " << getSigmaAlpha() << endl;

  }


  void TOPGeoPrism::appendPeelOffRegion(unsigned ID, double fraction, double angle)
  {
    for (const auto& region : m_peelOffRegions) {
      if (region.ID == ID) {
        B2ERROR("TOPGeoPrism::appendPeelOffRegion: region already appended."
                << LogVar("region ID", ID));
        return;
      }
    }
    PeelOffRegion region;
    region.ID = ID;
    region.fraction = fraction;
    region.angle = angle;
    double halfSize = (getWidth() - getPeelOffSize()) / 2;
    if (fabs(getPeelOffCenter(region)) > halfSize) {
      B2ERROR("TOPGeoPrism::appendPeelOffRegion: region doesn't fit into prism."
              << LogVar("region ID", ID));
      return;
    }
    m_peelOffRegions.push_back(region);
  }

  typedef std::pair<double, double> Pair;

  std::vector<Pair> TOPGeoPrism::getPeelOffContour(const PeelOffRegion& region) const
  {
    std::vector<Pair> contour;
    constructContour(getPeelOffSize() / 2, getExitThickness() / 2,
                     region.fraction, region.angle,
                     contour);

    return contour;
  }


  void TOPGeoPrism::unfold() const
  {
    double z = -(m_length - m_flatLength);
    double yUp = m_thickness / 2;
    double yDown = yUp - m_exitThickness;
    TVector2 points[2] = {TVector2(yUp, z), TVector2(yDown, z)}; // points on upper and slanted surfaces

    double alpha = getAngle();
    TVector2 normals[2] = {TVector2(1, 0), TVector2(-cos(alpha), sin(alpha))}; // normals of upper and slanted surfaces

    TVector2 orig(0, z); // window origin
    TVector2 surf(1, 0); // window surface direction (= upper surface normal)
    TVector2 norm(0, -1); // window normal
    auto slanted = normals[1]; // slanted surface normal

    reflect(points, normals, orig, surf, norm, slanted, 1, m_unfoldedWindows); // unfolding down
    std::reverse(m_unfoldedWindows.begin(), m_unfoldedWindows.end());
    m_unfoldedWindows.push_back(UnfoldedWindow(orig, surf, norm, slanted)); // true window
    m_k0 = m_unfoldedWindows.size() - 1;
    reflect(points, normals, orig, surf, norm, slanted, 0, m_unfoldedWindows); // unfolding up
  }


  void TOPGeoPrism::reflect(const TVector2* points, const TVector2* normals,
                            const TVector2& orig, const TVector2& surf, const TVector2& norm,
                            const TVector2& slanted, int k,
                            std::vector<UnfoldedWindow>& result) const
  {
    TVector2 rp[2] = {points[0], points[1]};
    TVector2 n[2] = {normals[0], normals[1]};
    auto r = orig;
    auto s = surf;
    auto q = norm;
    auto sl = slanted;

    while (rp[k].Y() < 0) {
      r -= 2 * ((r - rp[k]) * n[k]) * n[k]; // reflect window origin
      s -= 2 * (s * n[k]) * n[k];           // reflect window surface direction
      q -= 2 * (q * n[k]) * n[k];           // reflect window normal
      sl -= 2 * (sl * n[k]) * n[k];         // reflect slanted normal
      result.push_back(UnfoldedWindow(r, s, q, sl));
      if (result.size() > 100) {
        B2ERROR("TOPGeoPrism::reflect: too many reflections -> must be a bug");
        return;
      }
      int i = (k + 1) % 2; // index of opposite surface
      rp[i] -= 2 * ((rp[i] - rp[k]) * n[k]) * n[k]; // reflect point on the opposite surface
      n[i] -= 2 * (n[i] * n[k]) * n[k];             // reflect normal of opposite surface
      k = i; // toggle the reflection surface
    }
  }


} // end Belle2 namespace
