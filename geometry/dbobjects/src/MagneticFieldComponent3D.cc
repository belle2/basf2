/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/dbobjects/MagneticFieldComponent3D.h>

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


namespace Belle2 {
  B2Vector3D MagneticFieldComponent3D::getField(const B2Vector3D& pos) const
  {
    using std::get;

    /** small helper function to calculate the bin index and fraction inside
     * the index given a relative coordinate and the coordinate index (0=R,
     * 1=Phi, 2=Z).
     *
     * Example: If the z grid starts at 5 and goes to 15 with 6 points (pitch size of 2),
     * then calling getIndexWeight(10.5, 2) will return tuple(2, 0.75) since
     * the 10.5 lies in bin "2" and is already 75% to the next bin
     *
     * It will also cap the index to be inside the valid grid range
     *
     * Returns a tuple with the bin index as first element and the weight
     * fraction inside the bin as second element
     */
    auto getIndexWeight = [this](double value, int coordinate) -> std::tuple<unsigned int, double> {
      double weight = value * m_invgridPitch[coordinate];
      unsigned int index = static_cast<unsigned int>(weight);
      index = std::min(index, static_cast<unsigned int>(m_mapSize[coordinate] - 2));
      weight -= index;
      return std::make_tuple(index, weight);
    };

    const double z = pos.z();
    const double r2 = pos.Perp2();

    // Calculate the lower index of the point in the Z grid
    // Note that z coordinate is inverted to match ANSYS frame
    const auto iz = getIndexWeight(m_maxZ - z, 2);

    // directly on z axis: get B-field values from map in cartesian system assuming phi=0, so Bx = Br and By = Bphi
    if (r2 == 0) return interpolate(0, 0, get<0>(iz), 0, 0, get<1>(iz));

    const double r = std::sqrt(r2);
    const auto ir = getIndexWeight(r - m_minR, 0);

    // Calculate the lower index of the point in the Phi grid
    const double ay = std::abs(pos.y());
    const auto iphi = getIndexWeight(fast_atan2_minimax<4>(ay, pos.x()), 1);

    // Get B-field values from map in cylindrical coordinates
    B2Vector3D b = interpolate(get<0>(ir), get<0>(iphi), get<0>(iz), get<1>(ir), get<1>(iphi), get<1>(iz));
    // and convert it to cartesian
    const double norm = 1 / r;
    const double s = ay * norm;
    const double c = pos.x() * norm;
    // Flip sign of By if y<0
    const double sgny = (pos.y() >= 0) - (pos.y() < 0);
    // in cartesian system
    return B2Vector3D(-(b.x() * c - b.y() * s), -sgny * (b.x() * s + b.y() * c), b.z());
  }

  B2Vector3D MagneticFieldComponent3D::interpolate(unsigned int ir, unsigned int iphi, unsigned int iz,
                                                   double wr1, double wphi1, double wz1) const
  {
    const unsigned int strideZ = m_mapSize[0] * m_mapSize[1];
    const unsigned int strideR = m_mapSize[1];

    const double wz0 = 1 - wz1;
    const double wr0 = 1 - wr1;
    const double wphi0 = 1 - wphi1;
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
    const std::vector<B2Vector3F>& B = m_bmap;
    return
      (B[j000] * w00 + B[j001] * w01 + B[j010] * w10 + B[j011] * w11) * wz0 +
      (B[j100] * w00 + B[j101] * w01 + B[j110] * w10 + B[j111] * w11) * wz1;
  }
}
