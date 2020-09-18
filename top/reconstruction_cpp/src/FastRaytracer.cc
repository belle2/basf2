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
#include <top/geometry/TOPGeometryPar.h>
#include <framework/logging/Logger.h>

namespace Belle2 {
  namespace TOP {

    FastRaytracer::FastRaytracer(int moduleID, EGeometry geometry, EOptics optics):
      m_moduleID(moduleID), m_geometry(geometry), m_optics(optics)
    {
      const auto* geo = TOPGeometryPar::Instance()->getGeometry();
      if (not geo->isModuleIDValid(moduleID)) {
        B2ERROR("FastRaytracer: invalid slot number, moduleID = " << moduleID);
        return;
      }
      const auto& module = geo->getModule(moduleID);

      m_prism = PhotonState::Prism(module);
      m_mirror = PhotonState::Mirror(module);

      if (geometry == c_Unified) {
        m_bars.push_back(PhotonState::BarSegment(module));
      } else {
        m_bars.push_back(PhotonState::BarSegment(module.getBarSegment2(), m_prism.zR));
        m_bars.push_back(PhotonState::BarSegment(module.getBarSegment1(), m_bars.back().zR));
        m_bars.push_back(PhotonState::BarSegment(module.getMirrorSegment(), m_bars.back().zR));
      }
    }


    void FastRaytracer::propagate(const PhotonState& photon) const
    {
      m_status = false;
      m_photonStates.clear();
      m_Nxm = 0;
      m_Nxb = 0;
      m_Nxe = 0;
      m_Nym = 0;
      m_Nyb = 0;
      m_Nye = 0;
      m_photonStates.push_back(photon);

      int nbars = m_bars.size();
      if (photon.getKz() > 0) {
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
          m_Nxm += abs(newState.getNx());
          m_Nym += abs(newState.getNy());
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
        m_Nxb += abs(newState.getNx());
        m_Nyb += abs(newState.getNy());
      }

      const auto& lastState = m_photonStates.back();
      m_photonStates.push_back(lastState);
      auto& newState = m_photonStates.back();
      newState.propagate(m_prism);
      if (not newState.getPropagationStatus()) return;
      m_Nxe = abs(newState.getNx());
      m_Nye = abs(newState.getNy());

      if (photon.getKx() < 0) {
        m_Nxm = -m_Nxm;
        m_Nxb = -m_Nxb;
        m_Nxe = -m_Nxe;
      }
      if (photon.getKy() < 0) {
        m_Nym = -m_Nym;
        m_Nyb = -m_Nyb;
        m_Nye = -m_Nye;
      }

      m_status = true;
    }


    bool FastRaytracer::getTotalReflStatus(double cosTotal) const
    {
      for (const auto& photonState : m_photonStates) {
        if (not photonState.getTotalReflStatus(cosTotal)) return false;
      }
      return true;
    }


    double FastRaytracer::getXD() const
    {
      if (m_photonStates.empty()) return 0;

      double x = m_photonStates.back().getX();
      for (int i = m_photonStates.size() - 1; i > 0; i--) {
        const auto& photonState = m_photonStates[i];
        if (photonState.getSegmentType() == PhotonState::c_MirrorSegment) break;
        x = photonState.getUnfoldedX(x);
      }
      if (m_Nxm % 2 != 0) x = -x; // convention used in F77 code
      return x;
    }


    double FastRaytracer::getYD() const
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


  } // TOP
} // Belle2
