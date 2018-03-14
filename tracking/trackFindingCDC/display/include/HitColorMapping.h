/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: dschneider, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/display/ColorMapping.h>
#include <tracking/trackFindingCDC/display/Mapping.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>

#include <map>
#include <vector>
#include <string>

namespace Belle2 {
  class CDCHit;

  namespace TrackFindingCDC {

    class CDCWireHit;

    /// CDCHit to stroke width map highlighting the CDCHits with 0 drift length.
    class ZeroDriftLengthStrokeWidthMap : public Mapping<const CDCHit> {
    public:
      /// Function call to map the CDCHit id and object to a stroke width.
      std::string map(int index, const CDCHit& hit) override;
    };

    /// Interface class for CDCHit to color map objects.
    using HitColorMapping = Mapping<const CDCHit>;

    /// CDCHit to color map highlighting the CDCHits with 0 drift length.
    class ZeroDriftLengthColorMap : public HitColorMapping {
    public:
      /// Function call to map the CDCHit id and object to a color.
      std::string map(int index, const CDCHit& hit) override;
    };

    /// CDCHit to color map highlighting the CDCHits that posses the do not use flag.
    class TakenFlagColorMap : public HitColorMapping {
    public:
      /// Constructor checking if the CDCWireHits caring the taken flag are accessable.
      TakenFlagColorMap();

      /// Function call to map the CDCHit id and object to a color.
      std::string map(int index, const CDCHit& hit) override;

    private:
      /// Memory of the handle to the CDCWireHits on the DataStore.
      StoreWrappedObjPtr<std::vector<CDCWireHit> > m_storedWireHits{"CDCWireHitVector"};
    };

    /// CDCHit to color map by their local right left passage information from Monte Carlo truth
    class RLColorMap : public HitColorMapping {
    public:
      /// Function call to map the CDCHit id and object to a color.
      std::string map(int index, const CDCHit& hit) override;

      /// Informal string summarizing the translation from right left passage variable to colors.
      std::string info() override;
    };

    /// CDCHit to color map by their assoziated CDCSimHit::getPosFlag property.
    class PosFlagColorMap : public HitColorMapping {
    public:
      /// Function call to map the CDCHit id and object to a color.
      std::string map(int index, const CDCHit& hit) override;

      /// Informal string summarizing the translation from CDCSimHit::getPosFlag variable to colors.
      std::string info() override;
    };

    /// CDCHit to color map by their assoziated CDCSimHit::getBackgroundTag property.
    class BackgroundTagColorMap : public HitColorMapping {
    public:
      /// Function call to map the CDCHit id and object to a color.
      std::string map(int index, const CDCHit& hit) override;

      /// Informal string summarizing the translation from CDCSimHit::getBackgroundTag
      std::string info() override;
    };

    /// CDCHit to color map by their Monte Carlo segment id
    class MCSegmentIdColorMap : public HitColorMapping {
    public:
      /// Function call to map the CDCHit id and object to a color.
      std::string map(int index, const CDCHit& hit) override;
    };

    /// CDCHit to color map by their assoziated CDCSimHit::getFlightTime.
    class TOFColorMap : public HitColorMapping {
    public:
      /// Function call to map the CDCHit id and object to a color.
      std::string map(int index, const CDCHit& hit) override;

      /// Translates the given floating point time of flight to a color.
      std::string timeOfFlightToColor(const double timeOfFlight);
    };

    /// CDCHit to color map indicating the reassignment to a different MCParticle.
    class ReassignedSecondaryMap : public HitColorMapping {
    public:
      /// Function call to map the CDCHit id and object to a color.
      std::string map(int index, const CDCHit& hit) override;
    };

    /// CDCHit to color map by their assoziated MCParticle::getArrayId() property.
    class MCParticleColorMap : public HitColorMapping {
    public:
      /// Constructor setting up a Monte Carlo id to color map which is continously filled as new during the event.
      MCParticleColorMap();

      /// Function call to map the CDCHit id and object to a color.
      std::string map(int index, const CDCHit& hit) override;

      /// Informal string summarizing the translation from CDCSimHit::getBackgroundTag
      std::string info() override;
    private:
      /// List of colors to be cycled
      std::vector<std::string> m_colors;

      /// Index of the color to be used next.
      int m_iColor;

      /**
       *  Mapping of the already used colors by the MCParticle::getArrayId to map later CDCHits to
       *  the same color.
       */
      std::map<int, std::string> m_usedColors;
    };

    /// CDCHit to color map by the associated MCParticle::getPDG()
    class MCPDGCodeColorMap : public HitColorMapping {
    public:
      /// Function call to map the CDCHit id and object to a color.
      std::string map(int index, const CDCHit& hit) override;

      /// Informal string summarizing the translation from pdg codes to colors.
      std::string info() override;
    };

    /// CDCHit to color map by the isPrimary information as well as the secondary process type in case the particle is not primary.
    class MCPrimaryColorMap : public HitColorMapping {
    public:
      /// Function call to map the CDCHit id and object to a color.
      std::string map(int index, const CDCHit& hit) override;

      /// Informal string summarizing the translation from seconday process codes to colors.
      std::string info() override;
    };

    /// CDCHit to color map by the associated CDCSimHit::getPDG().
    class SimHitPDGCodeColorMap : public HitColorMapping {
    public:
      /// Function call to map the CDCHit id and object to a color.
      std::string map(int index, const CDCHit& hit) override;

      /// Informal string summarizing the translation from pdg codes to colors.
      std::string info() override;
    };

    /// CDCHit to color map by CDCSimHit::getBackgroundTag().
    class SimHitIsBkgColorMap : public HitColorMapping {
    public:
      /// Function call to map the CDCHit id and object to a color.
      std::string map(int index, const CDCHit& hit) override;
    };

    /// Segment to color map by the number of passed loops.
    class NLoopsColorMap: public HitColorMapping {
    public:
      /// Function call to map a segments object from the local finder to a color.
      std::string map(int index, const CDCHit& hit) override;
    };
  }
}
