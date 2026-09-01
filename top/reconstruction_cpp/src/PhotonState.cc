/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/reconstruction_cpp/PhotonState.h>
#include <top/reconstruction_cpp/func.h>
#include <framework/logging/Logger.h>
#include <cmath>
#include <algorithm>

using namespace std;

namespace Belle2 {
  namespace TOP {

    double PhotonState::s_maxLen = 10000;

    PhotonState::PhotonState(const ROOT::Math::XYZPoint& position, const ROOT::Math::XYZVector& direction):
      m_x(position.X()), m_y(position.Y()), m_z(position.Z()),
      m_kx(direction.X()), m_ky(direction.Y()), m_kz(direction.Z()),
      m_status(true)
    {}


    PhotonState::PhotonState(const ROOT::Math::XYZPoint& position, double kx, double ky, double kz):
      m_x(position.X()), m_y(position.Y()), m_z(position.Z()),
      m_kx(kx), m_ky(ky), m_kz(kz),
      m_status(true)
    {}

    PhotonState::PhotonState(const ROOT::Math::XYZPoint& position, const ROOT::Math::XYZVector& trackDir,
                             double thc, double fic):
      m_x(position.X()), m_y(position.Y()), m_z(position.Z()),
      m_status(true)
    {
      ROOT::Math::XYZVector dir(cos(fic) * sin(thc), sin(fic) * sin(thc), cos(thc));
      func::rotateUz(dir, trackDir);
      m_kx = dir.X();
      m_ky = dir.Y();
      m_kz = dir.Z();
    }

    bool PhotonState::isInside(const RaytracerBase::BarSegment& bar) const
    {
      if (std::abs(m_x) > bar.A / 2) return false;
      if (std::abs(m_y) > bar.B / 2) return false;
      if (m_z < bar.zL or m_z > bar.zR) return false;
      return true;
    }


    bool PhotonState::isInside(const RaytracerBase::BarSegment& bar, const RaytracerBase::Mirror& mirror) const
    {
      if (std::abs(m_x) > bar.A / 2) return false;
      if (std::abs(m_y) > bar.B / 2) return false;
      if (m_z < bar.zL) return false;
      double Rsq = pow(m_x - mirror.xc, 2) + pow(m_y - mirror.yc, 2) + pow(m_z - mirror.zc, 2);
      if (Rsq > pow(mirror.R, 2)) return false;
      return true;
    }


    bool PhotonState::isInside(const RaytracerBase::Prism& prism) const
    {
      if (std::abs(m_x) > prism.A / 2) return false;
      if (m_z < prism.zL or m_z > prism.zR) return false;
      if (m_y > prism.yUp or m_y < prism.yDown) return false;
      double y = prism.yDown + (prism.yDown + prism.B / 2) / (prism.zFlat - prism.zR) * (m_z - prism.zFlat);
      if (m_y < y) return false;
      return true;
    }


    void PhotonState::propagate(const RaytracerBase::BarSegment& bar)
    {
      if (not m_status) return;

      // local copies: stores to the data members would otherwise be assumed to alias bar
      const double A = bar.A;
      const double B = bar.B;
      double x = m_x;
      double y = m_y;
      double kx = m_kx;
      double ky = m_ky;

      m_cosx = std::abs(kx);
      m_cosy = std::abs(ky);
      m_A = A;
      m_B = B;
      m_type = c_BarSegment;

      double z = bar.zR;
      if (m_kz < 0) z = bar.zL;
      if (z == m_z) return;

      m_status = false;

      double len = (z - m_z) / m_kz;
      if (len < 0 or len > s_maxLen) return;

      func::fold(x + len * kx, A, x, kx, m_nx);
      func::fold(y + len * ky, B, y, ky, m_ny);

      m_propLen += len;
      m_x = x;
      m_y = y;
      m_kx = kx;
      m_ky = ky;
      m_z = z;

      m_status = true;
    }


    void PhotonState::propagateSemiLinear(const RaytracerBase::BarSegment& bar, const RaytracerBase::Mirror& mirror)
    {
      if (not m_status) return;

      m_cosx = std::abs(m_kx);
      m_cosy = std::abs(m_ky);
      m_A = bar.A;
      m_B = bar.B;
      m_type = c_MirrorSegment;

      m_status = false;

      if (m_kz < 0) return;

      double len = 0;
      if (m_z < mirror.zb) {
        len = (mirror.zb - m_z) / m_kz;
        if (len > s_maxLen) return;
      }

      double xm = m_x + len * m_kx;
      int nx = func::lround(xm / bar.A);
      double ss = m_kx * m_kx + m_kz * m_kz;
      if (ss == 0) return;
      int i = 0;
      while (true) {
        double xc = func::unfold(mirror.xc, nx, bar.A);
        double x = m_x - xc;
        double z = m_z - mirror.zc;
        double rdir = x * m_kx + z * m_kz;
        double rr = x * x + z * z;
        double D = rdir * rdir + (mirror.R * mirror.R - rr) * ss;
        if (D < 0) return;
        D = sqrt(D);
        len = (D - rdir) / ss;
        if (len < 0 or len > s_maxLen) return;
        double xmm = m_x + len * m_kx;
        int nxx = func::lround(xmm / bar.A);
        if (nxx == nx) break;
        i++;
        if (i == 10) {
          if (std::abs(xmm - xm) < 0.001) break;
          B2DEBUG(20, "TOP::PhotonState::propagateSemiLinear: not converging");
          return;
        }
        nx = nxx;
        xm = xmm;
      }

      m_propLen += len;

      func::fold(m_x + len * m_kx, bar.A, m_x, m_kx, m_nx);
      func::fold(m_y + len * m_ky, bar.B, m_y, m_ky, m_ny);
      m_y = mirror.yc;
      m_z += len * m_kz;

      double normX = (m_x - mirror.xc) / mirror.R;
      double normZ = (m_z - mirror.zc) / mirror.R;
      double s = 2 * (m_kx * normX + m_kz * normZ);
      m_kx -= s * normX;
      m_kz -= s * normZ;

      m_status = true;
    }


    void PhotonState::propagateExact(const RaytracerBase::BarSegment& bar, const RaytracerBase::Mirror& mirror)
    {
      if (not m_status) return;

      m_cosx = std::abs(m_kx);
      m_cosy = std::abs(m_ky);
      m_A = bar.A;
      m_B = bar.B;
      m_type = c_MirrorSegment;

      m_status = false;

      if (m_kz < 0) return;

      double len = 0;
      if (m_z < mirror.zb) {
        len = (mirror.zb - m_z) / m_kz;
        if (len > s_maxLen) return;
      }

      double xm = m_x + len * m_kx;
      int nx = func::lround(xm / bar.A);
      double ym = m_y + len * m_ky;
      int ny = func::lround(ym / bar.B);
      int i = 0;
      while (true) {
        double xc = func::unfold(mirror.xc, nx, bar.A);
        double yc = func::unfold(mirror.yc, ny, bar.B);
        double x = m_x - xc;
        double y = m_y - yc;
        double z = m_z - mirror.zc;
        double rdir = x * m_kx + y * m_ky + z * m_kz;
        double rr = x * x + y * y + z * z;
        double D = rdir * rdir + (mirror.R * mirror.R - rr);
        if (D < 0) return;
        D = sqrt(D);
        len = (D - rdir);
        if (len < 0 or len > s_maxLen) return;
        double xmm = m_x + len * m_kx;
        int nxx = func::lround(xmm / bar.A);
        double ymm = m_y + len * m_ky;
        int nyy = func::lround(ymm / bar.B);
        if (nxx == nx and nyy == ny) break;
        i++;
        if (i == 10) {
          if (std::abs(xmm - xm) < 0.001 and std::abs(ymm - ym) < 0.001) break;
          B2DEBUG(20, "TOP::PhotonState::propagateExact: not converging");
          return;
        }
        nx = nxx;
        ny = nyy;
      }

      m_propLen += len;

      func::fold(m_x + len * m_kx, bar.A, m_x, m_kx, m_nx);
      func::fold(m_y + len * m_ky, bar.B, m_y, m_ky, m_ny);
      m_z += len * m_kz;

      double normX = (m_x - mirror.xc) / mirror.R;
      double normY = (m_y - mirror.yc) / mirror.R;
      double normZ = (m_z - mirror.zc) / mirror.R;
      double s = 2 * (m_kx * normX + m_ky * normY + m_kz * normZ);
      m_kx -= s * normX;
      m_ky -= s * normY;
      m_kz -= s * normZ;

      m_status = true;
    }


    void PhotonState::propagate(const RaytracerBase::Prism& prism)
    {
      if (not m_status) return;

      m_status = false;

      // Work on local copies of the state and of the prism geometry. Writing to the data members
      // inside the loop below would force the compiler to re-load the window data on every
      // iteration (a double stored through 'this' may alias the doubles of the unfolded windows).

      double x = m_x;
      double y = m_y;
      double z = m_z;
      double kx = m_kx;
      double ky = m_ky;
      double kz = m_kz;
      double propLen = m_propLen;
      double cosy = m_cosy;

      const double prismA = prism.A;
      const double yUp = prism.yUp;
      const double yDown = prism.yDown;
      const double zR = prism.zR;
      const double zFlat = prism.zFlat;
      const double zDet = prism.zD;

      m_cosx = std::abs(kx);
      m_A = prismA;
      m_B = yUp - yDown;
      m_y0 = (yUp + yDown) / 2;
      m_type = c_Prism;

      if (kz > 0) {
        if (z >= zR or std::abs(ky / kz) < std::abs(prism.slope)) return;
        if (std::abs(y + ky / kz * (zR - z)) < prism.B / 2) return;
      }
      const double ky_in = ky;
      const double kz_in = kz;

      double yD = m_yD;
      double zD = m_zD;

      if (z > zFlat) {

        int step = 1;
        int ii = 0;
        if (ky < 0) {
          step = -1;
          ii = 1;
          y = std::min(y, yUp);
        }

        const auto* windows = prism.unfoldedWindows.data();
        const unsigned numWindows = prism.unfoldedWindows.size();
        const int k0 = prism.k0;

        bool found = false;
        unsigned k = k0;
        while (k < numWindows) {
          const auto& win = windows[k];
          double s = ky * win.sz - kz * win.sy;
          if (s != 0) {
            double len = ((win.y0 - y) * win.sz - (win.z0 - z) * win.sy) / s;
            yD = y + len * ky;
            zD = z + len * kz;
            double yu = yD - win.y0;
            double zu = zD - win.z0;
            double yw = yu * win.sy + zu * win.sz;
            if (yw >= yDown and yw <= yUp) {
              m_y = y;
              m_yD = yD;
              m_zD = zD;
              m_cosy = cosy;
              if (len < 0 or len > s_maxLen) return;
              double kyNew = ky * win.sy + kz * win.sz;
              double kzNew = kz * win.sy - ky * win.sz;
              int ny = k - k0;
              x += len * kx;
              y = yw;
              ky = kyNew;
              m_ny = ny;
              z = zFlat;
              kz = ny % 2 == 0 ? kzNew : -kzNew;
              propLen += len;
              found = true;
              break;
            }
            cosy = std::max(cosy, std::abs(ky_in * win.nsy[ii] + kz_in * win.nsz[ii]));
          }
          k += step;
          ii ^= 1;
        }

        if (not found) {
          m_y = y;
          m_yD = yD;
          m_zD = zD;
          m_cosy = cosy;
          B2DEBUG(20, "TOP::PhotonState::propagate: unfolded prism window not found"
                  << LogVar("yUp", prism.yUp) << LogVar("yDown", prism.yDown) << LogVar("zR", prism.zR)
                  << LogVar("y", y) << LogVar("z", z)
                  << LogVar("ky", ky_in) << LogVar("kz", kz_in));
          return;
        }
      } else {
        yD = y;
        zD = z;
      }

      double len = (zDet - z) / kz;
      if (len < 0 or len > s_maxLen) {
        m_x = x;
        m_y = y;
        m_z = z;
        m_ky = ky;
        m_kz = kz;
        m_propLen = propLen;
        m_yD = yD;
        m_zD = zD;
        m_cosy = cosy;
        return;
      }
      func::fold(x + len * kx, prismA, x, kx, m_nx);
      y += len * ky;
      z = zDet;
      propLen += len;
      yD += len * ky_in;
      zD += len * kz_in;

      m_x = x;
      m_y = y;
      m_z = z;
      m_kx = kx;
      m_ky = ky;
      m_kz = kz;
      m_propLen = propLen;
      m_yD = yD;
      m_zD = zD;
      m_cosy = cosy;

      m_status = true;
    }


  } // namespace TOP
} // namespace Belle2


