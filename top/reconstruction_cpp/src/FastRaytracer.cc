/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/reconstruction_cpp/FastRaytracer.h>
#include <framework/logging/Logger.h>

namespace Belle2 {
  namespace TOP {

    void FastRaytracer::clear() const
    {
      m_photonStates.clear();
      m_extraStates.clear();
      m_status = false;
      m_Nxm = 0;
      m_Nxb = 0;
      m_Nxe = 0;
      m_Nym = 0;
      m_Nyb = 0;
      m_Nye = 0;
    }

    void FastRaytracer::propagate(const PhotonState& photon, bool averaging) const
    {
      clear();
      m_photonStates.push_back(photon);

      int nbars = m_bars.size();
      if (photon.getKz() > 0 and photon.getZ() > m_prism.zR) {
        for (int i = 0; i < nbars; i++) {
          const auto& bar = m_bars[i];
          const auto& lastState = m_photonStates.back();
          if (lastState.getZ() >= bar.zR) continue;
          m_photonStates.push_back(lastState);
          auto& newState = m_photonStates.back();
          if (i < nbars - 1) {
            newState.propagate(bar);
          } else if (m_optics == c_SemiLinear) {
            newState.propagateSemiLinear(bar, m_mirror);
          } else {
            newState.propagateExact(bar, m_mirror);
          }
          if (not newState.getPropagationStatus()) return;

          m_Nxm += (m_Nxm % 2 == 0) ? newState.getNx() : -newState.getNx();
          m_Nym += (m_Nym % 2 == 0) ? newState.getNy() : -newState.getNy();
        }
      }

      for (int i = nbars - 1; i >= 0; i--) {
        const auto& bar = m_bars[i];
        const auto& lastState = m_photonStates.back();
        if (lastState.getZ() <= bar.zL) continue;
        m_photonStates.push_back(lastState);
        auto& newState = m_photonStates.back();
        newState.propagate(bar);
        if (not newState.getPropagationStatus()) return;

        m_Nxb += (m_Nxb % 2 == 0) ? newState.getNx() : -newState.getNx();
        m_Nyb += (m_Nyb % 2 == 0) ? newState.getNy() : -newState.getNy();
      }

      const auto& lastState = m_photonStates.back();
      m_photonStates.push_back(lastState);
      auto& newState = m_photonStates.back();
      newState.propagate(m_prism);
      if (not newState.getPropagationStatus()) return;

      if (averaging) {
        m_extraStates.push_back(lastState);
        auto& flippedState = m_extraStates.back().flipKy();
        flippedState.propagate(m_prism);
        if (not flippedState.getPropagationStatus()) return;
      }

      m_Nxe = newState.getNx();
      m_Nye = newState.getNy();

      m_status = true;
    }

    bool FastRaytracer::getTotalReflStatus(double cosTotal) const
    {
      for (const auto& photonState : m_photonStates) {
        if (not photonState.getTotalReflStatus(cosTotal)) return false;
      }
      return true;
    }

    double FastRaytracer::getPropagationLen() const
    {
      if (m_photonStates.empty()) return 0;
      if (m_extraStates.empty()) return m_photonStates.back().getPropagationLen();
      return (m_photonStates.back().getPropagationLen() + m_extraStates.back().getPropagationLen()) / 2;
    }

    double FastRaytracer::getPropagationLenDelta() const
    {
      if (m_photonStates.empty() or m_extraStates.empty()) return 0;
      return (m_photonStates.back().getPropagationLen() - m_extraStates.back().getPropagationLen());
    }

    double FastRaytracer::getXD() const
    {
      if (m_photonStates.empty()) return 0;

      double x = m_extraStates.empty() ?
                 m_photonStates.back().getXD() : (m_photonStates.back().getXD() + m_extraStates.back().getXD()) / 2;

      for (int i = m_photonStates.size() - 2; i > 0; i--) {
        const auto& photonState = m_photonStates[i];
        if (photonState.getSegmentType() == PhotonState::c_MirrorSegment) break;
        x = photonState.getUnfoldedX(x);
      }
      return x;
    }


    double FastRaytracer::getYD() const
    {
      if (m_photonStates.empty()) return 0;

      double y = m_photonStates.back().getYD();
      for (int i = m_photonStates.size() - 2; i > 0; i--) {
        const auto& photonState = m_photonStates[i];
        if (photonState.getSegmentType() == PhotonState::c_MirrorSegment) break;
        y = photonState.getUnfoldedY(y);
      }
      return y;
    }


    double FastRaytracer::getInPlaneYD() const
    {
      if (m_photonStates.empty()) return 0;

      double y = m_photonStates.back().getY();
      for (int i = m_photonStates.size() - 1; i > 0; i--) {
        const auto& photonState = m_photonStates[i];
        if (photonState.getSegmentType() == PhotonState::c_MirrorSegment) break;
        y = photonState.getUnfoldedY(y);
      }
      return y;
    }


    double FastRaytracer::getZD() const
    {
      if (m_photonStates.empty()) return 0;
      if (m_extraStates.empty()) return m_photonStates.back().getZD();
      return (m_photonStates.back().getZD() + m_extraStates.back().getZD()) / 2;
    }


    double FastRaytracer::getYB() const
    {
      if (m_photonStates.size() < 2) return 0;

      int i0 = m_photonStates.size() - 2;
      double y = m_photonStates[i0].getY();
      for (int i = i0; i > 0; i--) {
        const auto& photonState = m_photonStates[i];
        if (photonState.getSegmentType() == PhotonState::c_MirrorSegment) break;
        y = photonState.getUnfoldedY(y);
      }
      return y;
    }


    int FastRaytracer::getNx() const
    {
      int Nx = m_Nxm;
      Nx += (Nx % 2 == 0) ? m_Nxb : -m_Nxb;
      Nx += (Nx % 2 == 0) ? m_Nxe : -m_Nxe;
      return Nx;
    }


    int FastRaytracer::getNy() const
    {
      int Ny = m_Nym;
      Ny += (Ny % 2 == 0) ? m_Nyb : -m_Nyb;
      Ny += (Ny % 2 == 0) ? m_Nye : -m_Nye;
      return Ny;
    }

    int FastRaytracer::getNys() const
    {
      int N = (m_Nye > 0) ? m_Nye / 2 : (abs(m_Nye) + 1) / 2;
      return N;
    }

  } // TOP
} // Belle2
