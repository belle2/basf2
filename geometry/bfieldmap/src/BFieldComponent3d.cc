/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-20XX  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/bfieldmap/BFieldComponent3d.h>
#include <geometry/bfieldmap/BFieldComponentBeamline.h>

#include <framework/utilities/FileSystem.h>
#include <framework/logging/Logger.h>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include <cmath>

using namespace std;
using namespace Belle2;
namespace io = boost::iostreams;

void BFieldComponent3d::initialize()
{

  // Input field map
  if (m_mapFilename.empty()) {
    B2ERROR("The filename for the 3d magnetic field component is empty !");
    return;
  }
  string fullPath = FileSystem::findFile("/data/" + m_mapFilename);
  if (!FileSystem::fileExists(fullPath)) {
    B2ERROR("The 3d magnetic field map file '" << m_mapFilename << "' could not be found !");
    return;
  }

  // Options to reduce 3D to 2D
  if (m_mapEnable != "rphiz" && m_mapEnable != "rphi" && m_mapEnable != "phiz" && m_mapEnable != "rz") {
    B2ERROR("BField3d:: enabled coordinates must be \"rphiz\", \"rphi\", \"phiz\" or \"rz\"");
    return;
  }

  // Excluded region
  m_exRegion = true;
  if ((m_exRegionR[0] == m_exRegionR[1]) || (m_exRegionZ[0] == m_exRegionZ[1])) m_exRegion = false;

  // Print initial input parameters
  B2DEBUG(100, "BField3d:: initial input parameters");
  B2DEBUG(100, Form("   map filename:          %s",                   m_mapFilename.c_str()));
  B2DEBUG(100, Form("   map dimension:         %s",                   m_mapEnable.c_str()));
  if (m_interpolate) { B2DEBUG(100, Form("   map interpolation:     on")); }
  else               { B2DEBUG(100, Form("   map interpolation:     off")); }
  B2DEBUG(100, Form("   map r pitch & range:   %.2e [%.2e, %.2e] cm", m_gridPitch[0], m_mapRegionR[0], m_mapRegionR[1]));
  B2DEBUG(100, Form("   map phi pitch:         %.2e deg",             m_gridPitch[1] * 180 / M_PI));
  B2DEBUG(100, Form("   map z pitch & range:   %.2e [%.2e, %.2e] cm", m_gridPitch[2], m_mapRegionZ[0], m_mapRegionZ[1]));
  if (m_exRegion) {
    B2DEBUG(100, Form("   map r excluded region: [%.2e, %.2e] cm",    m_exRegionR[0], m_exRegionR[1]));
    B2DEBUG(100, Form("   map z excluded region: [%.2e, %.2e] cm",    m_exRegionZ[0], m_exRegionZ[1]));
  }

  m_bmap.reserve(m_mapSize[0]*m_mapSize[1]*m_mapSize[2]);
  // Load B-field map file
  io::filtering_istream fieldMapFile;
  fieldMapFile.push(io::gzip_decompressor());
  fieldMapFile.push(io::file_source(fullPath));

  char tmp[256];
  for (int k = 0; k < m_mapSize[2]; k++) { // z
    for (int i = 0; i < m_mapSize[0]; i++) { // r
      for (int j = 0;  j < m_mapSize[1]; j++) { // phi
        double Br, Bz, Bphi;
        //r[m]  phi[deg]  z[m]  Br[T]  Bphi[T]  Bz[T]
        fieldMapFile.getline(tmp, 256);
        // sscanf(tmp+33,"%lf %lf %lf",&Br,&Bphi,&Bz);
        char* next;
        Br   = strtod(tmp + 33, &next);
        Bphi = strtod(next, &next);
        Bz   = strtod(next, NULL);
        m_bmap.emplace_back(-Br, -Bphi, -Bz);
      }
    }
  }

  // Introduce error on B field
  if (m_errRegionR[0] != m_errRegionR[1]) {
    auto it = m_bmap.begin();
    for (int k = 0; k < m_mapSize[2]; k++) { // z
      double r = m_mapRegionR[0];
      for (int i = 0; i < m_mapSize[0]; i++, r += m_gridPitch[0]) { // r
        if (!(r >= m_errRegionR[0] && r < m_errRegionR[1])) { it += m_mapSize[1];  continue;}
        for (int j = 0;  j < m_mapSize[1]; j++) { // phi
          B2Vector3F& B = *it;
          B.SetX(B.x() * m_errB[0]);
          B.SetY(B.y() * m_errB[1]);
          B.SetZ(B.z() * m_errB[2]);
        }
      }
    }
  }

  m_igridPitch[0] = 1 / m_gridPitch[0];
  m_igridPitch[1] = 1 / m_gridPitch[1];
  m_igridPitch[2] = 1 / m_gridPitch[2];

  B2DEBUG(100, Form("BField3d:: final map region & pitch: r [%.2e,%.2e] %.2e, phi %.2e, z [%.2e,%.2e] %.2e",
                    m_mapRegionR[0], m_mapRegionR[1], m_gridPitch[0], m_gridPitch[1],
                    m_mapRegionZ[0], m_mapRegionZ[1], m_gridPitch[2]));
  B2DEBUG(100, "Memory consumption: " << m_bmap.size()*sizeof(B2Vector3F) / (1024 * 1024.) << " Mb");
}

namespace {
  /** Fast quadrant based atan2 (arctangent of y/x in -pi to pi) approximation
   * using minimax approximation in different orders.  The maximum absolute
   * errors are
   *
   * - order 1: 0.07 radians (4 degree)
   * - order 2: 0.0038 radians (0.22 degree)
   * - order 3: 0.0015 radians (0.086 degree)
   * - order 4: 2.57e-05 radians (0.0015 degree)
   * - order 5: 7.57e-06 radians (0.00043 degree)
   * - order 6: 4.65e-07 radians (2.67e-05 degree)
   *
   * @tparam ORDER order of the minmax polynomial (1 to 6)
   * @param y Value representing the proportion of the y-coordinate.
   * @param x Value representing the proportion of the x-coordinate.
   */
  template<int ORDER = 4> constexpr double fast_atan2_minimax(double y, double x)
  {
    static_assert(ORDER >= 1 && ORDER <= 6, "fast_atan2_minimax: Only orders 1-6 are supported");
    constexpr double pi4 = M_PI / 4, pi2 = M_PI / 2;
    const double ax = std::abs(x), ay = std::abs(y);
    const double z = (ax - ay) / (ay + ax);
    const double v = std::abs(z), v2 = v * v;
    double p0{0}, p1{0};
    if (ORDER == 2) {
      p0 = 0.273;
    } else if (ORDER == 3) {
      p0 = 0.2447;
      p1 = 0.0663;
    } else if (ORDER == 4) {
      // 4th order polynomial minimax approximation
      // max error <=2.57373e-05 rad (0.00147464 deg)
      constexpr double c4[] = {
        +0.2132675884368258,
        +0.23481662556227,
        -0.2121597649928347,
        +0.0485854027042442
      };
      p0 = c4[0] + v2 * c4[2];
      p1 = c4[1] + v2 * c4[3];
    } else if (ORDER == 5) {
      // 5th order polynomial minimax approximation
      // max error <=7.57429e-06 rad (0.000433975 deg)
      constexpr double c5[] = {
        +0.2141439315495107,
        +0.2227491783659812,
        -0.1628994411740733,
        -0.02778537455524869,
        +0.03962954416153075
      };
      p0 = c5[0] + v2 * (c5[2] + v2 * c5[4]);
      p1 = c5[1] + v2 * (c5[3]);
    } else if (ORDER == 6) {
      // 6th order polynomial minimax approximation
      // max error <=4.65134e-07 rad (2.66502e-05 deg)
      constexpr double c6[] = {
        +0.2145843590230225,
        +0.2146820003230985,
        -0.116250549812964,
        -0.1428509550362758,
        +0.1660612278047719,
        -0.05086851503449636
      };
      p0 = c6[0] + v2 * (c6[2] + v2 * (c6[4]));
      p1 = c6[1] + v2 * (c6[3] + v2 * (c6[5]));
    }
    double phi = (z + 1) * pi4  - (z * (v - 1)) * (p0 + v * p1);
    phi = pi2 - copysign(phi, x);
    return copysign(phi, y);
  }
}

B2Vector3D BFieldComponent3d::calculate(const B2Vector3D& point) const
{
  auto getPhiIndexWeight = [this](double y, double x, double & wphi) -> unsigned int {
    double phi = fast_atan2_minimax<4>(y, x);
    wphi = phi * m_igridPitch[1];
    unsigned int iphi = static_cast<unsigned int>(wphi);
    iphi = min(iphi, static_cast<unsigned int>(m_mapSize[1] - 2));
    wphi -= iphi;
    return iphi;
  };

  B2Vector3D B(0, 0, 0);

  // If both '3d' and 'Beamline' components are defined in xml file,
  // '3d' component returns zero field where 'Beamline' component is defined.
  // If no 'Beamline' component is defined in xml file, the following function will never be called.
  if (BFieldComponentBeamline::Instance().isInRange(point)) {
    return B;
  }

  double z = point.z();
  // Check if the point lies inside the magnetic field boundaries
  if (z < m_mapRegionZ[0] || z > m_mapRegionZ[1]) return B;

  double r2 = point.Perp2();
  // Check if the point lies inside the magnetic field boundaries
  if (r2 < m_mapRegionR[0]*m_mapRegionR[0] || r2 >= m_mapRegionR[1]*m_mapRegionR[1]) return B;
  // Check if the point lies in the exclude region
  if (m_exRegion && (z >= m_exRegionZ[0]) && (z < m_exRegionZ[1]) &&
      (r2 >= m_exRegionR[0]*m_exRegionR[0]) && (r2 < m_exRegionR[1]*m_exRegionR[1])) return B;

  // Calculate the lower index of the point in the Z grid
  // Note that z coordinate is inverted to match ANSYS frame
  double wz = (m_mapRegionZ[1] - z) * m_igridPitch[2];
  unsigned int iz = static_cast<int>(wz);
  iz = min(iz, static_cast<unsigned int>(m_mapSize[2] - 2));
  wz -= iz;

  if (r2 > 0) {
    double r = sqrt(r2);

    // Calculate the lower index of the point in the R grid
    double wr = (r - m_mapRegionR[0]) * m_igridPitch[0];
    unsigned int ir = static_cast<unsigned int>(wr);
    ir = min(ir, static_cast<unsigned int>(m_mapSize[0] - 2));
    wr -= ir;

    // Calculate the lower index of the point in the Phi grid
    double ay = std::abs(point.y());
    double wphi;
    unsigned int iphi = getPhiIndexWeight(ay, point.x(), wphi);

    // Get B-field values from map
    B2Vector3D b = interpolate(ir, iphi, iz, wr, wphi, wz); // in cylindrical system
    double norm = 1 / r;
    double s = ay * norm, c = point.x() * norm;
    // Flip sign of By if y<0
    const double sgny = (point.y() >= 0) - (point.y() < 0);
    // in cartesian system
    B.SetXYZ(-(b.x() * c - b.y() * s), -sgny * (b.x() * s + b.y() * c), b.z());
  } else {
    // Get B-field values from map in cartesian system assuming phi=0, so Bx = Br and By = Bphi
    B = interpolate(0, 0, iz, 0, 0, wz);
  }

  return B;
}

void BFieldComponent3d::terminate()
{
}

B2Vector3D BFieldComponent3d::interpolate(unsigned int ir, unsigned int iphi, unsigned int iz,
                                          double wr1, double wphi1, double wz1) const
{
  const unsigned int strideZ = m_mapSize[0] * m_mapSize[1];
  const unsigned int strideR = m_mapSize[1];

  const double wz0 = 1 - wz1, wr0 = 1 - wr1, wphi0 = 1 - wphi1;
  const unsigned int j000 = iz * strideZ + ir * strideR + iphi;
  const unsigned int j001 = j000 + 1;
  const unsigned int j010 = j000 + strideR;
  const unsigned int j011 = j001 + strideR;
  const unsigned int j100 = j000 + strideZ;
  const unsigned int j101 = j001 + strideZ;
  const unsigned int j110 = j010 + strideZ;
  const unsigned int j111 = j011 + strideZ;
  const double w00 = wphi0 * wr0;
  const double w10 = wphi0 * wr1;
  const double w01 = wphi1 * wr0;
  const double w11 = wphi1 * wr1;
  const vector<B2Vector3F>& B = m_bmap;
  return
    (B[j000] * w00 + B[j001] * w01 + B[j010] * w10 + B[j011] * w11) * wz0 +
    (B[j100] * w00 + B[j101] * w01 + B[j110] * w10 + B[j111] * w11) * wz1;
}
