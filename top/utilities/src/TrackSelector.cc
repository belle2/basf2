/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/utilities/TrackSelector.h>
#include <framework/logging/Logger.h>
#include <top/geometry/TOPGeometryPar.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <analysis/utility/PCmsLabTransform.h>

namespace Belle2 {
  namespace TOP {

    TrackSelector::TrackSelector(const std::string& sampleName): m_sampleName(sampleName)
    {
      if (sampleName == "dimuon") {
        m_sampleType = c_dimuon;
        m_chargedStable = Const::muon;
      } else if (sampleName == "bhabha") {
        m_sampleType = c_bhabha;
        m_chargedStable = Const::electron;
      } else if (sampleName == "cosmics") {
        m_sampleType = c_cosmics;
        m_chargedStable = Const::muon;
      } else {
        m_sampleType = c_undefined;
        m_sampleName = "undefined";
        B2ERROR("TOP::TrackSelector: unknown data sample."
                << LogVar("name", sampleName));
      }
    }


    bool TrackSelector::isSelected(const TOPTrack& trk) const
    {

      if (m_sampleType == c_undefined) {
        B2ERROR("TOP::TrackSelector:isSelected sample type is undefined, returning false");
        return false;
      }

      if (not trk.isValid()) return false;

      const auto* fit = trk.getTrack()->getTrackFitResultWithClosestMass(m_chargedStable);
      if (not fit) return false;

      // cut on POCA
      m_pocaPosition = fit->getPosition();
      if (m_pocaPosition.Rho() > m_dr) return false;
      if (fabs(m_pocaPosition.Z()) > m_dz) return false;

      // momentum/energy cut
      m_pocaMomentum = fit->getMomentum();
      if (m_sampleType == c_cosmics) {
        if (m_pocaMomentum.R() < m_minMomentum) return false;
      } else if (m_sampleType == c_dimuon or m_sampleType == c_bhabha) {
        ROOT::Math::PxPyPzMVector lorentzLab(m_pocaMomentum.X(), m_pocaMomentum.Y(), m_pocaMomentum.Z(), m_chargedStable.getMass());
        PCmsLabTransform T;
        auto lorentzCms = T.labToCms(lorentzLab);
        m_cmsEnergy = lorentzCms.energy();
        double dE = m_cmsEnergy - T.getCMSEnergy() / 2;
        if (fabs(dE) > m_deltaEcms) return false;
      } else {
        return false;
      }

      // cut on local z
      const auto* geo = TOPGeometryPar::Instance()->getGeometry();
      const auto& module = geo->getModule(trk.getModuleID());
      m_localPosition = module.pointToLocal(static_cast<ROOT::Math::XYZPoint>(trk.getExtHit()->getPosition()));
      m_localMomentum = module.momentumToLocal(trk.getExtHit()->getMomentum());
      if (m_localPosition.Z() < m_minZ or m_localPosition.Z() > m_maxZ) return false;

      return true;
    }

  } // namespace TOP
} // namespace Belle2

