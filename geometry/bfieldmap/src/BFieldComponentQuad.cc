/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hiroyuki Nakayama                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/bfieldmap/BFieldComponentQuad.h>

#include <framework/utilities/FileSystem.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include <cmath>

using namespace std;
using namespace Belle2;
namespace io = boost::iostreams;

void BFieldComponentQuad::initialize()
{
  // check if input name is not empty
  if (m_mapFilenameHER.empty()) {
    B2FATAL("The filename for the HER quadrupole magnetic field component is empty !");
    return;
  }
  if (m_mapFilenameLER.empty()) {
    B2FATAL("The filename for the LER quadrupole magnetic field component is empty !");
    return;
  }
  if (m_apertFilenameHER.empty()) {
    B2FATAL("The filename for the HER beam pipe aperture is empty !");
    return;
  }
  if (m_apertFilenameLER.empty()) {
    B2FATAL("The filename for the LER beam pipe aperture is empty !");
    return;
  }

  // check if input files exsits
  string fullPathMapHER = FileSystem::findFile("/data/" + m_mapFilenameHER);
  if (!FileSystem::fileExists(fullPathMapHER)) {
    B2FATAL("The HER quadrupole magnetic field map file '" << m_mapFilenameHER << "' could not be found !");
    return;
  }
  string fullPathMapLER = FileSystem::findFile("/data/" + m_mapFilenameLER);
  if (!FileSystem::fileExists(fullPathMapLER)) {
    B2FATAL("The LER quadrupole magnetic field map file '" << m_mapFilenameLER << "' could not be found !");
    return;
  }
  string fullPathMapHERleak = FileSystem::findFile("/data/" + m_mapFilenameHERleak);
  if (!FileSystem::fileExists(fullPathMapHERleak)) {
    B2FATAL("The HERleak quadrupole magnetic field map file '" << m_mapFilenameHERleak << "' could not be found !");
    return;
  }
  string fullPathApertHER = FileSystem::findFile("/data/" + m_apertFilenameHER);
  if (!FileSystem::fileExists(fullPathApertHER)) {
    B2FATAL("The HER aperture file '" << m_apertFilenameHER << "' could not be found !");
    return;
  }
  string fullPathApertLER = FileSystem::findFile("/data/" + m_apertFilenameLER);
  if (!FileSystem::fileExists(fullPathApertLER)) {
    B2FATAL("The LER aperture file '" << m_apertFilenameLER << "' could not be found !");
    return;
  }

  //--------------------------------------------------------------
  // Load the field map files
  //--------------------------------------------------------------

  /** Magnetic field data structure. */
  struct ParamPoint {
    double s;   /**< s in [m] */
    double L;   /**< element length in [m] */
    double K0;  /**< dipole component in [dimensionless] */
    double K1;  /**< quadrupole component in [1/m] */
    double SK0; /**< skew dipole component  in [dimensionless] */
    double SK1; /**< skew quadrupole component in [1/m] */
    double ROTATE; /**< rotation in [radian] */
    double DX;   /**< horizontal displacement in [m] */
    double DY;   /**< vertical displacement in [m] */
    /* Note that K parameters used in SAD is multiplied by the element length.
     * Popular definitions are:  K0,SK0[1/m] and K1,SK1[1/m^2]
     */
  };

  io::filtering_istream fieldMapFileHER, fieldMapFileLER, fieldMapFileHERleak;
  fieldMapFileHER.push(io::file_source(fullPathMapHER));
  fieldMapFileLER.push(io::file_source(fullPathMapLER));
  fieldMapFileHERleak.push(io::file_source(fullPathMapHERleak));

  //Create the parameter map and read the data from the file
  auto readLenseParameters = [](istream & IN) -> vector<ParamPoint> {
    vector<ParamPoint> r;
    string name;
    ParamPoint t;
    while (IN >> name >> t.s >> t.L >> t.K0 >> t.K1 >> t.SK0 >> t.SK1 >> t.ROTATE >> t.DX >> t.DY)
    {
      /* Convert parameters in basf2 default unit [cm].*/
      t.s *= 100;
      t.L *= 100;
      t.DX *= 100;
      t.DY *= 100;
      t.K1 /= 100;
      t.SK1 /= 100;
      r.push_back(t);
    }
    return r;
  };

  std::vector<ParamPoint> params_her, params_ler, params_herl;
  params_her = readLenseParameters(fieldMapFileHER);
  params_ler = readLenseParameters(fieldMapFileLER);
  params_herl = readLenseParameters(fieldMapFileHERleak);

  //--------------------------------------------------------------
  // Load the aperture map files
  //--------------------------------------------------------------
  auto readAperturePoints = [this](istream & IN) -> vector<ApertPoint> {
    vector<ApertPoint> r;
    ApertPoint t;
    while (IN >> t.s >> t.r)
    {
      /* s and r are in [mm] in Apert?ER.dat. */
      /* Transform parameters in basf2 default unit [cm].*/
      t.s /= 10; t.r /= 10;
      r.push_back(t);
      m_maxr2 = max(m_maxr2, t.r);
    }
    return r;
  };

  io::filtering_istream apertFileHER, apertFileLER;
  apertFileHER.push(io::file_source(fullPathApertHER));
  apertFileLER.push(io::file_source(fullPathApertLER));

  m_ah = readAperturePoints(apertFileHER);
  m_al = readAperturePoints(apertFileLER);

  m_maxr2 *= m_maxr2;

  /** range filled by lense structure */
  struct range_t {double r0, r1;};
  typedef vector<range_t> ranges_t;

  /** calculate ranges where quadruple lenses continously fill the beamline
   *
   * @param v lense vector
   * @return  vector of ranges
   */
  auto getranges = [](const vector<ParamPoint>& v) {
    ranges_t r;
    double s0 = v[0].s, smax = v.back().s + v.back().L;
    for (int i = 0, N = v.size(); i < N - 1; i++) {
      const ParamPoint& t0 = v[i], t1 = v[i + 1];
      double sn = t0.s + t0.L;
      if (abs(sn - t1.s) > 1e-10) {
        r.push_back({s0, sn});
        s0 = t1.s;
      }
    }
    r.push_back({s0, smax});
    return r;
  };

  ranges_t ranges_her  = getranges(params_her);
  ranges_t ranges_herl = getranges(params_herl);
  ranges_t ranges_ler  = getranges(params_ler);

  /** merge overlaped ranges in case we have several lense maps like in HER + leakage
   *
   * @param r0  vector of ranges
   * @param r1  vector of ranges
   * @return    vector of ranges
   */
  auto mergeranges = [](const ranges_t& r0, const ranges_t& r1) {
    ranges_t r = r0; r.insert(r.end(), r1.begin(), r1.end());
    for (auto it = r.begin(); it != r.end(); ++it) {
      for (auto jt = it + 1; jt != r.end();) {
        if (it->r1 >= jt->r0 and jt->r1 >= it->r0) {
          double smin = min(it->r0, jt->r0);
          double smax = max(it->r1, jt->r1);
          *it = {smin, smax};
          jt = r.erase(jt);
        } else {
          ++jt;
        }
      }
    }
    return r;
  };
  ranges_her = mergeranges(ranges_her, ranges_herl);

  /**
   * Returns the beam pipe aperture at given position.
   *
   * @param s   position in the beam-axis coordinate.
   * @param ap  aperture vector
   * @return    beam pipe aperture at given position.
   */
  auto getr = [](double s, const vector<ApertPoint>& ap) -> double {
    auto it0 = upper_bound(ap.begin(), ap.end(), s, [](double s, const ApertPoint & b) {return s < b.s;});
    if (it0 == ap.begin() || it0 == ap.end()) return 0;
    const ApertPoint& p0 = *(it0 - 1), &p1 = *it0;
    double r = p0.r + (s - p0.s) / (p1.s - p0.s) * (p1.r - p0.r);
    return r;
  };

  /** zero aperture in case of lense absence
   *
   * @param ap aperture vector to update
   * @param v  ranges where lenses are present
   */
  auto update_aperture = [getr](vector<ApertPoint>& ap, const ranges_t& v) -> void {
    auto less = [](double s, const ApertPoint & b) {return s < b.s;};
    double smin = v.front().r0;
    ap.insert(upper_bound(ap.begin(), ap.end(), smin, less), {smin, getr(smin, ap)});
    ap.erase(std::remove_if(ap.begin(), ap.end(), [smin](const ApertPoint & b) {return b.s < smin;}), ap.end());
    for (auto it = v.begin(); it + 1 != v.end(); it++)
    {
      double s0 = it->r1, s1 = (it + 1)->r0;
      double r0 = getr(s0, ap);
      double r1 = getr(s1, ap);
      auto it0 = upper_bound(ap.begin(), ap.end(), s0, less);
      it0 = ap.insert(it0, {s0, 0});
      it0 = ap.insert(it0, {s0, r0});
      auto it1 = upper_bound(ap.begin(), ap.end(), s1, less);
      it1 = ap.insert(it1, {s1, r1});
      it1 = ap.insert(it1, {s1, 0});
      ap.erase(std::remove_if(ap.begin(), ap.end(), [s0, s1](const ApertPoint & b) {return s0 < b.s && b.s < s1;}), ap.end());
    }
    double smax = v.back().r1;
    ap.insert(upper_bound(ap.begin(), ap.end(), smax, less), {smax, getr(smax, ap)});
    ap.erase(std::remove_if(ap.begin(), ap.end(), [smax](const ApertPoint & b) {return smax < b.s;}), ap.end());
  };

  update_aperture(m_ah, ranges_her);
  update_aperture(m_al, ranges_ler);

  const int ROTATE_DIRECTION = 1; // 1: default, 0: rotate-off, -1: inversely rotate

  /** fold rotation to/from lense frame to a single matrix multiplicaton and vector addition
   *
   * @param in  map with angle inside
   * @param p0  normalization constant
   * @return    structure where rotations are in the matrix form
   */
  auto proc3 = [](const vector<ParamPoint>& in, double p0) -> vector<ParamPoint3> {
    vector<ParamPoint3> out;
    out.resize(in.size());
    auto it = out.begin();
    for (auto b = in.begin(); b != in.end(); ++b, ++it)
    {
      ParamPoint3& t = *it;
      t.s = b->s;
      t.L = b->L;

      double K0  = b->K0;
      double K1  = b->K1;
      double SK0 = b->SK0;
      double SK1 = b->SK1;
      double DX  = b->DX;
      double DY  = b->DY;
      double k = p0 / t.L;
      K0  *= k;
      K1  *= k;
      SK0 *= k;
      SK1 *= k;

      double sphi, cphi;
      sincos(-b->ROTATE * (M_PI / 180), &sphi, &cphi);
      if (ROTATE_DIRECTION == 1)
        sphi = sphi, cphi = cphi;
      else if (ROTATE_DIRECTION == -1)
        sphi = -sphi, cphi = cphi;
      else if (ROTATE_DIRECTION == 0)
        sphi = 0, cphi = 1;

      double s2phi = 2 * cphi * sphi;
      double c2phi = cphi * cphi - sphi * sphi;
      double tp = DX * SK1 + DY * K1;
      double tm = DY * SK1 - DX * K1;

      t.mxx =  K1 * s2phi + SK1 * c2phi;
      t.mxy = -SK1 * s2phi +  K1 * c2phi;
      t.mx0 =  tm * s2phi -  tp * c2phi + SK0 * cphi +  K0 * sphi;

      t.myx = -SK1 * s2phi +  K1 * c2phi;
      t.myy = -K1 * s2phi - SK1 * c2phi;
      t.my0 =  tp * s2phi +  tm * c2phi +  K0 * cphi - SK0 * sphi;
    }
    return out;
  };

  const double c = Const::speedOfLight / (Unit::m / Unit::s) / 100;
  const double p0_HER = 7.0e+9 / c, p0_LER = 4.0e+9 / c;

  m_h3 = proc3(params_her, p0_HER);
  m_l3 = proc3(params_ler, p0_LER);
  vector<ParamPoint3> hleak3 = proc3(params_herl, p0_HER);

  /** In case several maps in the same position we can simply
   * sum up all matricies since magnetic field has superposition
   * properties as well as keep only one vector of parameters for each
   * beamline
   *
   * @param v   first map
   * @param a   map to add to the first map
   * @return    merged field map
   */
  auto merge = [](vector<ParamPoint3>& v, const vector<ParamPoint3>& a) {
    for (const auto& t : a) {
      auto i = upper_bound(v.begin(), v.end(), t.s, [](double s, const ParamPoint3 & b) {return s < b.s;});
      if (i == v.end()) continue;
      ParamPoint3& p = *(i - 1);
      if (p.s == t.s && p.L == t.L) {
        p.mxx += t.mxx;
        p.mxy += t.mxy;
        p.mx0 += t.mx0;
        p.myx += t.myx;
        p.myy += t.myy;
        p.my0 += t.my0;
      } else {
        v.insert(i, t);
      }
    }
  };
  merge(m_h3, hleak3);

  /** Since in any case we are going to calculate aperture of a
   * beamline we can precompute range of the field map which
   * corresponds to aperture range already fetched to narrow search of
   * a particular lense in the range.
   *
   * @param ap   aperture map
   * @param v    field map
   * @return     vector with field map ranges
   */
  auto make_index = [](const vector<ApertPoint>& ap, const vector<ParamPoint3>& v) {
    vector<irange_t> res;
    for (auto it = ap.begin(); it + 1 != ap.end(); it++) {
      const ApertPoint& p0 = *it, &p1 = *(it + 1);
      if (p0.r > 0 && p1.r > 0) {
        short int i0 = upper_bound(v.begin(), v.end(), p0.s, [](double s, const ParamPoint3 & b) {return s < b.s;}) - v.begin();
        short int i1 = upper_bound(v.begin(), v.end(), p1.s, [](double s, const ParamPoint3 & b) {return s < b.s;}) - v.begin();
        res.push_back({i0, i1});
      } else {
        res.push_back({0, 0});
      }
    }
    return res;
  };

  m_indexh = make_index(m_ah, m_h3);
  m_indexl = make_index(m_al, m_l3);
}

double BFieldComponentQuad::getApertureHER(double s) const
{
  auto less = [](double s, const ApertPoint & b) {return s < b.s;};
  const auto it = upper_bound(m_ah.begin(), m_ah.end(), s, less);
  if (it == m_ah.begin() || it == m_ah.end()) return 0;
  const ApertPoint& p1 = *(it - 1), &p2 = *it;
  return p1.r + (p2.r - p1.r) / (p2.s - p1.s) * (s - p1.s);
}

double BFieldComponentQuad::getApertureLER(double s) const
{
  auto less = [](double s, const ApertPoint & b) {return s < b.s;};
  const auto it = upper_bound(m_al.begin(), m_al.end(), s, less);
  if (it == m_al.begin() || it == m_al.end()) return 0;
  const ApertPoint& p1 = *(it - 1), &p2 = *it;
  return p1.r + (p2.r - p1.r) / (p2.s - p1.s) * (s - p1.s);
}

struct vector3_t {
  double x, y, z;
  inline double rho2() const {return x * x + y * y;}
};

TVector3 BFieldComponentQuad::calculate(const TVector3& point) const
{
  const double sa = sin(0.0415), ca = cos(0.0415);
  int indx;
  auto getAperture = [&indx](double s, const vector<ApertPoint>& ap) {
    auto less = [](double s, const ApertPoint & b) {return s < b.s;};
    const auto it = upper_bound(ap.begin(), ap.end(), s, less);
    if (it == ap.begin() || it == ap.end()) return 0.0;
    indx = it - ap.begin();
    const ApertPoint& p1 = *(it - 1), &p2 = *it;
    return p1.r + (p2.r - p1.r) / (p2.s - p1.s) * (s - p1.s);
  };

  auto find = [&indx](double s, const vector<ParamPoint3>& v, const vector<irange_t>& a) -> vector<ParamPoint3>::const_iterator {
    const auto it = upper_bound(v.begin() + a[indx - 1].i0, v.begin() + a[indx - 1].i1, s, [](double s, const ParamPoint3 & b) {return s < b.s;});
    if (it == v.begin()) return v.end();
    return it - 1;
  };

  //assume point is given in [cm]
  TVector3 B(0, 0, 0);

  vector3_t v = {point.x(), point.y(), point.z()};
  double xc = v.x * ca, zs = v.z * sa, zc = v.z * ca, xs = v.x * sa;
  vector3_t vh = {(xc - zs), -v.y, -(zc + xs)}; // to the HER beamline frame
  vector3_t vl = {(xc + zs), -v.y, -(zc - xs)}; // to the LER beamline frame

  double r2h = vh.rho2(), r2l = vl.rho2();

  if (r2h < r2l) { /* the point is closer to HER*/
    if (r2h < m_maxr2) { /* within max radius */
      double r = getAperture(vh.z, m_ah);
      if (r2h < r * r) {
        const auto it = find(vh.z, m_h3, m_indexh);
        if (it != m_h3.end()) {
          double Bx = it->getBx(vh.x, vh.y);
          double By = it->getBy(vh.x, vh.y);
          B.SetXYZ(Bx * ca, -By, -Bx * sa); // to the detector frame
        }
      }
    }
  } else {      /* the point is closer to LER*/
    if (r2l < m_maxr2) { /* within max radius */
      double r = getAperture(vl.z, m_al);
      if (r2l < r * r) {
        const auto it = find(vl.z, m_l3, m_indexl);
        if (it != m_l3.end()) {
          double Bx = it->getBx(vl.x, vl.y);
          double By = it->getBy(vl.x, vl.y);
          B.SetXYZ(Bx * ca, -By, Bx * sa); // to the detector frame
        }
      }
    }
  }

  return B;
}

void BFieldComponentQuad::terminate()
{
}
