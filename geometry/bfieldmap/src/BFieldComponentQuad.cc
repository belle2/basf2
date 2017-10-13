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

/** Linear search with a sentinel at the end of the sequence. It
 * should be faster than a binary search in short arrays.
 *
 * @param it  starting iterator
 * @param val  key
 * @param comp  compare function
 * @return    points to element which first makes comp(val,*it) true
 */
template <class ForwardIterator, class T, class Compare>
ForwardIterator linear_sentinel(ForwardIterator it, const T& val, Compare comp)
{
  do {
    if (comp(val, *it)) break;
    ++it;
  } while (1);
  return it;
}

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
    r.push_back({numeric_limits<double>::infinity(), m_maxr2});
    return r;
  };

  io::filtering_istream apertFileHER, apertFileLER;
  apertFileHER.push(io::file_source(fullPathApertHER));
  apertFileLER.push(io::file_source(fullPathApertLER));

  m_ah = readAperturePoints(apertFileHER);
  m_al = readAperturePoints(apertFileLER);

  m_maxr2 *= m_maxr2;

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

#define ROTATE_DIRECTION 1 // 1: default, 0: rotate-off, -1: inversely rotate
#if ROTATE_DIRECTION==0
      double sphi = 0, cphi = 1;
#else
      double sphi, cphi;
      sincos(-b->ROTATE * (M_PI / 180), &sphi, &cphi);
#if ROTATE_DIRECTION==-1
      sphi = -sphi;
#endif
#endif
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
  vector<ParamPoint3> hleak3 = proc3(params_herl, p0_HER);
  m_l3 = proc3(params_ler, p0_LER);

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
      if (p.s == t.s && p.L == t.L)
        p += t;
      else
        v.insert(i, t);
    }
  };
  merge(m_h3, hleak3);

  /** calculate ranges where quadrupole lenses continously fill the
   * beamline
   *
   * @param v lense vector
   * @return  vector of ranges
   */
  auto getranges = [](const vector<ParamPoint3>& v) {
    ranges_t r;
    double s0 = v[0].s, smax = v.back().s + v.back().L;
    for (int i = 0, N = v.size(); i < N - 1; i++) {
      const ParamPoint3& t0 = v[i], t1 = v[i + 1];
      double sn = t0.s + t0.L;
      if (abs(sn - t1.s) > 1e-10) {
        r.push_back({s0, sn});
        s0 = t1.s;
      }
    }
    r.push_back({s0, smax});
    return r;
  };

  m_ranges_her  = getranges(m_h3);
  m_ranges_ler  = getranges(m_l3);

  const double inf = numeric_limits<double>::infinity();
  // add sentinels
  m_ranges_her.insert(m_ranges_her.begin(), { -inf, -inf});
  m_ranges_her.insert(m_ranges_her.end(), {inf, inf});
  m_ranges_ler.insert(m_ranges_ler.begin(), { -inf, -inf});
  m_ranges_ler.insert(m_ranges_ler.end(), {inf, inf});

  /** associate ranges with the vector of aperture points
   *
   * @param ap the vector of aperture points
   * @param v  the vector of ranges
   * @return   the vector of iterators for more effective search
   */
  auto associate_aperture = [](const vector<ApertPoint>& ap, const ranges_t& v) {
    auto less = [](double s, const ApertPoint & b) {return s < b.s;};
    vector<std::vector<ApertPoint>::const_iterator> res;
    for (auto r : v) {
      auto i0 = upper_bound(ap.begin(), ap.end(), r.r0, less);
      if (i0 == ap.begin() || i0 == ap.end()) continue;
      res.push_back(i0 - 1);
    }
    return res;
  };
  m_offset_ap_her = associate_aperture(m_ah, m_ranges_her);
  m_offset_ap_ler = associate_aperture(m_al, m_ranges_ler);

  /** associate ranges with the vector of lenses
   *
   * @param ap the vector of lenses
   * @param v  the vector of ranges
   * @return   the vector of iterators which points to the begining of the continous area of lenses
   */
  auto associate_lenses = [](const vector<ParamPoint3>& ap, const ranges_t& v) {
    vector<std::vector<ParamPoint3>::const_iterator> res;
    for (auto r : v) {
      auto i0 = find_if(ap.begin(), ap.end(), [r](const ParamPoint3 & b) {return abs(b.s - r.r0) < 1e-10;});
      if (i0 == ap.end()) continue;
      res.push_back(i0);
    }
    return res;
  };
  m_offset_pp_her = associate_lenses(m_h3, m_ranges_her);
  m_offset_pp_ler = associate_lenses(m_l3, m_ranges_ler);
}

double BFieldComponentQuad::getApertureHER(double s) const
{
  int i = getRange(s, m_ranges_her);
  if (i < 0) return 0;
  return getAperture(s, m_offset_ap_her[i]);
}

double BFieldComponentQuad::getApertureLER(double s) const
{
  int i = getRange(s, m_ranges_ler);
  if (i < 0) return 0;
  return getAperture(s, m_offset_ap_ler[i]);
}

int BFieldComponentQuad::getRange(double s, const ranges_t& r) const
{
  auto it0 = r.begin() + 1;
  auto it = linear_sentinel(it0, s, [](double s_, const range_t& r_) {return s_ <= r_.r0;});
  if (s > (--it)->r1) return -1;
  return it - it0;
}

double BFieldComponentQuad::getAperture(double s, std::vector<ApertPoint>::const_iterator jt0) const
{
  auto jt = linear_sentinel(jt0, s, [](double s_, const ApertPoint & r) {return s_ <= r.s;});
  const ApertPoint& p1 = *(jt - 1), &p2 = *jt;
  return p1.r + (p2.r - p1.r) / (p2.s - p1.s) * (s - p1.s);
}

B2Vector3D BFieldComponentQuad::calculate(const B2Vector3D& point) const
{
  const double sa = sin(0.0415), ca = cos(0.0415);
  //assume point is given in [cm]
  B2Vector3D B(0, 0, 0);

  const B2Vector3D& v{point};
  double xc = v.x() * ca, zs = v.z() * sa, zc = v.z() * ca, xs = v.x() * sa;
  B2Vector3D vh{(xc - zs), -v.y(), -(zc + xs)}; // to the HER beamline frame
  B2Vector3D vl{(xc + zs), -v.y(), -(zc - xs)}; // to the LER beamline frame

  double r2h = vh.Perp2(), r2l = vl.Perp2();

  if (r2h < r2l) { /* the point is closer to HER*/
    if (r2h < m_maxr2) { /* within max radius */
      double s = vh.z();
      int i = getRange(s, m_ranges_her);
      if (i < 0) return B;
      double r = getAperture(s, m_offset_ap_her[i]);
      if (r2h < r * r) {
        auto kt = m_offset_pp_her[i] + static_cast<unsigned int>(s - m_ranges_her[i + 1].r0);
        double Bx = kt->getBx(vh.x(), vh.y());
        double By = kt->getBy(vh.x(), vh.y());
        B.SetXYZ(Bx * ca, -By, -Bx * sa); // to the detector frame
      }
    }
  } else {      /* the point is closer to LER*/
    if (r2l < m_maxr2) { /* within max radius */
      double s = vl.z();
      int i = getRange(s, m_ranges_ler);
      if (i < 0) return B;
      double r = getAperture(s, m_offset_ap_ler[i]);
      if (r2l < r * r) {
        auto kt = m_offset_pp_ler[i] + static_cast<unsigned int>(s - m_ranges_ler[i + 1].r0);
        double Bx = kt->getBx(vl.x(), vl.y());
        double By = kt->getBy(vl.x(), vl.y());
        B.SetXYZ(Bx * ca, -By, Bx * sa); // to the detector frame
      }
    }
  }
  return B;
}
