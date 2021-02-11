/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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

    PhotonState::PhotonState(const TVector3& position, const TVector3& direction):
      m_x(position.X()), m_y(position.Y()), m_z(position.Z()),
      m_kx(direction.X()), m_ky(direction.Y()), m_kz(direction.Z()),
      m_status(true)
    {}


    PhotonState::PhotonState(const TVector3& position, double kx, double ky, double kz):
      m_x(position.X()), m_y(position.Y()), m_z(position.Z()),
      m_kx(kx), m_ky(ky), m_kz(kz),
      m_status(true)
    {}

    PhotonState::PhotonState(const TVector3& position, const TVector3& trackDir, double thc, double fic):
      m_x(position.X()), m_y(position.Y()), m_z(position.Z()),
      m_status(true)
    {
      TVector3 dir(cos(fic) * sin(thc), sin(fic) * sin(thc), cos(thc));
      dir.RotateUz(trackDir);
      m_kx = dir.X();
      m_ky = dir.Y();
      m_kz = dir.Z();
    }

    bool PhotonState::isInside(const RaytracerBase::BarSegment& bar) const
    {
      if (abs(m_x) > bar.A / 2) return false;
      if (abs(m_y) > bar.B / 2) return false;
      if (m_z < bar.zL or m_z > bar.zR) return false;
      return true;
    }


    bool PhotonState::isInside(const RaytracerBase::BarSegment& bar, const RaytracerBase::Mirror& mirror) const
    {
      if (abs(m_x) > bar.A / 2) return false;
      if (abs(m_y) > bar.B / 2) return false;
      if (m_z < bar.zL) return false;
      double Rsq = pow(m_x - mirror.xc, 2) + pow(m_y - mirror.yc, 2) + pow(m_z - mirror.zc, 2);
      if (Rsq > pow(mirror.R, 2)) return false;
      return true;
    }


    bool PhotonState::isInside(const RaytracerBase::Prism& prism) const
    {
      if (abs(m_x) > prism.A / 2) return false;
      if (m_z < prism.zL or m_z > prism.zR) return false;
      if (m_y > prism.yUp or m_y < prism.yDown) return false;
      double y = prism.yDown + (prism.yDown + prism.B / 2) / (prism.zFlat - prism.zR) * (m_z - prism.zFlat);
      if (m_y < y) return false;
      return true;
    }


    void PhotonState::propagate(const RaytracerBase::BarSegment& bar)
    {
      if (not m_status) return;

      m_A = bar.A;
      m_B = bar.B;
      m_type = c_BarSegment;

      double z = bar.zR;
      if (m_kz < 0) z = bar.zL;
      if (z == m_z) return;

      m_status = false;

      double len = (z - m_z) / m_kz;
      if (len < 0 or len > s_maxLen) return;
      m_propLen += len;

      func::fold(m_x + len * m_kx, bar.A, m_x, m_kx, m_nx);
      func::fold(m_y + len * m_ky, bar.B, m_y, m_ky, m_ny);
      m_z = z;

      m_status = true;
    }


    void PhotonState::propagateSemiLinear(const RaytracerBase::BarSegment& bar, const RaytracerBase::Mirror& mirror)
    {
      if (not m_status) return;

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
      int nx = lround(xm / bar.A);
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
        int nxx = lround(xmm / bar.A);
        if (nxx == nx) break;
        i++;
        if (i == 10) {
          if (abs(xmm - xm) < 0.001) break;
          B2WARNING("PhotonState::propagateSemiLinear: not converging");
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
      int nx = lround(xm / bar.A);
      double ym = m_y + len * m_ky;
      int ny = lround(ym / bar.B);
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
        int nxx = lround(xmm / bar.A);
        double ymm = m_y + len * m_ky;
        int nyy = lround(ymm / bar.B);
        if (nxx == nx and nyy == ny) break;
        i++;
        if (i == 10) {
          if (abs(xmm - xm) < 0.001 and abs(ymm - ym) < 0.001) break;
          B2WARNING("PhotonState::propagateExact: not converging");
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

      m_A = prism.A;
      m_B = prism.yUp - prism.yDown;
      m_y0 = (prism.yUp + prism.yDown) / 2;
      m_type = c_Prism;

      if (m_kz > 0) return;

      double ky_in = m_ky;
      double kz_in = m_kz;

      if (m_z > prism.zFlat) {

        int step = 1;
        if (m_ky < 0) {
          step = -1;
          m_y = std::min(m_y, prism.yUp);
        }

        unsigned k = prism.k0;
        while (k < prism.unfoldedWindows.size()) {
          const auto& win = prism.unfoldedWindows[k];
          double s = m_ky * win.sz - m_kz * win.sy;
          if (s == 0) {
            k += step;
            continue;
          }
          double len = ((win.y0 - m_y) * win.sz - (win.z0 - m_z) * win.sy) / s;
          m_yD = m_y + len * m_ky;
          m_zD = m_z + len * m_kz;
          double yu = m_yD - win.y0;
          double zu = m_zD - win.z0;
          double y = yu * win.sy + zu * win.sz;
          if (y >= prism.yDown and y <= prism.yUp) {
            if (len < 0 or len > s_maxLen) return;
            double ky = m_ky * win.sy + m_kz * win.sz;
            double kz = m_kz * win.sy - m_ky * win.sz;
            m_x += len * m_kx;
            m_y = y;
            m_ky = ky;
            m_ny = k - prism.k0;
            m_z = prism.zFlat;
            m_kz = m_ny % 2 == 0 ? kz : -kz;
            m_propLen += len;
            goto success;
          }
          k += step;
        }
        B2WARNING("PhotonState::propagate: unfolded prism window not found"
                  << LogVar("yUp", prism.yUp) << LogVar("yDown", prism.yDown)
                  << LogVar("y", m_y) << LogVar("z", m_z)
                  << LogVar("ky", ky_in) << LogVar("kz", kz_in));
        return;
      } else {
        m_yD = m_y;
        m_zD = m_z;
      }

success:
      double len = (prism.zD - m_z) / m_kz;
      if (len < 0 or len > s_maxLen) return;
      func::fold(m_x + len * m_kx, prism.A, m_x, m_kx, m_nx);
      m_y += len * m_ky;
      m_z = prism.zD;
      m_propLen += len;
      m_yD += len * ky_in;
      m_zD += len * kz_in;

      m_status = true;
    }


  } // namespace TOP
} // namespace Belle2


