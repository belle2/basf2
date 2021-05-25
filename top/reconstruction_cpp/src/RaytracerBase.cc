/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/reconstruction_cpp/RaytracerBase.h>
#include <top/geometry/TOPGeometryPar.h>
#include <framework/logging/Logger.h>
#include <algorithm>

namespace Belle2 {
  namespace TOP {

    RaytracerBase::BarSegment::BarSegment(const TOPGeoBarSegment& bar, double zLeft):
      A(bar.getWidth()), B(bar.getThickness()), zL(zLeft), reflectivity(bar.getSurfaceReflectivity(3.0))
    {
      zR = zL + bar.getFullLength();
    }


    RaytracerBase::BarSegment::BarSegment(const TOPGeoModule& module):
      A(module.getBarWidth()), B(module.getBarThickness()), reflectivity(module.getSurfaceReflectivity(3.0))

    {
      zL = -module.getBarLength() / 2;
      zR =  module.getBarLength() / 2;
    }


    RaytracerBase::Mirror::Mirror(const TOPGeoModule& module):
      xc(module.getMirrorSegment().getXc()), yc(module.getMirrorSegment().getYc()),
      zc(module.getMirrorSegment().getZc()), R(module.getMirrorSegment().getRadius()),
      reflectivity(module.getMirrorSegment().getMirrorReflectivity(3.0))
    {
      zc += (module.getBarLength() - module.getMirrorSegment().getFullLength()) / 2;
      zb = zc + R;
      double Ah = std::max(module.getMirrorSegment().getWidth(), module.getBarWidth()) / 2;
      double Bh = std::max(module.getMirrorSegment().getThickness(), module.getBarThickness()) / 2;
      zb = std::min(zb, zc + sqrt(pow(R, 2) - pow(xc - Ah, 2) - pow(yc - Bh, 2)));
      zb = std::min(zb, zc + sqrt(pow(R, 2) - pow(xc + Ah, 2) - pow(yc - Bh, 2)));
      zb = std::min(zb, zc + sqrt(pow(R, 2) - pow(xc - Ah, 2) - pow(yc + Bh, 2)));
      zb = std::min(zb, zc + sqrt(pow(R, 2) - pow(xc + Ah, 2) - pow(yc + Bh, 2)));
    }


    RaytracerBase::Prism::Prism(const TOPGeoModule& module):
      A(module.getPrism().getWidth()), B(module.getPrism().getThickness())
    {
      yUp = B / 2;
      const auto& prism = module.getPrism();
      yDown = yUp - prism.getExitThickness();
      zR = -module.getBarLength() / 2;
      zL = zR - prism.getLength();
      zFlat = zL + prism.getFlatLength();
      const auto& pmtArray = module.getPMTArray();
      // a call to prism.getFilterThickness is added for backward compatibility
      double filterThickness = pmtArray.getFilterThickness() + prism.getFilterThickness();
      double cookieThickness = pmtArray.getCookieThickness();
      double pmtWindow = pmtArray.getPMT().getWinThickness();
      zD = zL - filterThickness - cookieThickness - pmtWindow;
      slope = (-B / 2 - yDown) / (zR - zFlat);

      k0 = prism.getK0();
      unfoldedWindows = prism.getUnfoldedWindows();
      for (auto& w : unfoldedWindows) w.z0 += zR;
    }


    RaytracerBase::RaytracerBase(int moduleID, EGeometry geometry, EOptics optics):
      m_moduleID(moduleID), m_geometry(geometry), m_optics(optics)
    {
      const auto* geo = TOPGeometryPar::Instance()->getGeometry();
      if (not geo->isModuleIDValid(moduleID)) {
        B2FATAL("TOP::RaytracerBase: invalid slot number, moduleID = " << moduleID);
        return;
      }
      const auto& module = geo->getModule(moduleID);

      m_prism = Prism(module);
      m_mirror = Mirror(module);

      if (geometry == c_Unified) {
        m_bars.push_back(BarSegment(module));
      } else {
        m_bars.push_back(BarSegment(module.getBarSegment2(), m_prism.zR));
        m_bars.push_back(BarSegment(module.getBarSegment1(), m_bars.back().zR));
        m_bars.push_back(BarSegment(module.getMirrorSegment(), m_bars.back().zR));
      }
    }



  } // TOP
} // Belle2


