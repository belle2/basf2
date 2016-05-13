/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/geometry/TOPGeometryPar.h>

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {

  namespace TOP {

    TOPGeometryPar* TOPGeometryPar::s_instance = 0;

    TOPGeometryPar* TOPGeometryPar::Instance()
    {
      if (!s_instance) {
        s_instance = new TOPGeometryPar();
      }
      return s_instance;
    }

    void TOPGeometryPar::Initialize(const GearDir& content)
    {
      read(content);

      GearDir frontEndMapping(content, "FrontEndMapping");
      m_frontEndMapper.initialize(frontEndMapping);

      GearDir channelMapping0(content, "ChannelMapping[@type='IRS3B']");
      m_channelMapperIRS3B.initialize(channelMapping0);

      GearDir channelMapping1(content, "ChannelMapping[@type='IRSX']");
      m_channelMapperIRSX.initialize(channelMapping1);

      m_initialized = true;
    }

    void TOPGeometryPar::read(const GearDir& content)
    {

      // Support structure

      GearDir supParams(content, "Support");
      m_PannelThickness = supParams.getLength("PannelThickness");
      m_PlateThickness = supParams.getLength("PlateThickness");
      m_LowerGap = supParams.getLength("lowerGap");
      m_UpperGap = supParams.getLength("upperGap");
      m_SideGap = supParams.getLength("sideGap");
      m_forwardGap = supParams.getLength("forwardGap");
      m_backwardGap = supParams.getLength("backGap");
      m_pannelMaterial = supParams.getString("PannelMaterial");
      m_insideMaterial = supParams.getString("FillMaterial");

      // other

      m_brokenFraction = content.getDouble("Bars/BrokenJointFraction", 0);

    }


  } // End namespace TOP
} // End namespace Belle2
