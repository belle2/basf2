/*
 * CDCHitColorMaps.h
 *
 *  Created on: Jun 8, 2015
 *      Author: dschneider
 */

#pragma once

#include <tracking/trackFindingCDC/display/ColorMaps.h>
#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <map>
#include <cdc/dataobjects/CDCHit.h>


namespace Belle2 {
  namespace TrackFindingCDC {

    class CDCWireHit;

    /**
     * Virtual base class for CDCHit to color map objects.
     */
    class CDCHitColorMap {
    public:

      /**
       * Constructor.
       *
       * Sets m_bkgHitColor to "orange".
       */
      CDCHitColorMap()
        : m_bkgHitColor("orange") {};

      /**
       * Destructor.
       */
      virtual ~CDCHitColorMap() = default;

      /**
       * Function call to map the CDCHit id and object to a color.
       */
      virtual std::string map(const int, const CDCHit&)
      {
        return m_bkgHitColor;
      }

      /**
       * Informal string summarizing the translation variables to colors.
       */
      virtual std::string info()
      {
        return m_bkgHitColor;
      }

    protected:
      /**
       * The default color to be used.
       */
      std::string m_bkgHitColor;
    };

    /**
     * Defines a PDGCode to Color Map.
     */
    class ColorByPDGCode : public CDCHitColorMap {
    public:
      /**
       * Constructor setting up a PDGCode to Color map.
       */
      ColorByPDGCode() : m_missingPDGColor("lime"),
        m_colorByPDGCode(
      {
        { -999, m_bkgHitColor},
        {11, "blue"},
        { -11, "blue"},
        {13, "turquoise"},
        { -13, "turquoise"},
        {15, "cyan"},
        { -15, "cyan"},
        {211, "green"},
        { -211, "green"},
        {321, "olive"},
        { -321, "olive"},
        {2212, "red"},
        { -2212, "red"},
      }
      ) {};
    protected:
      /**
       * Color for the case a particle a pdg code not mentioned in the color_by_pdgcode map.
       */
      std::string m_missingPDGColor;
      /**
       * Map to define the color for the most relevant.
       */
      std::map<int, std::string> m_colorByPDGCode;
    };

    /**
     * CDCHit to color map highlighting the CDCHits with 0 drift length.
     */
    class ZeroDriftLengthColorMap : public CDCHitColorMap {
    public:
      /**
       * Function call to map the CDCHit id and object to a color.
       */
      std::string map(const int iCDCHit, const CDCHit& cdcHit) override;
    };

    /**
     * CDCHit to stroke width map highlighting the CDCHits with 0 drift length.
     */
    class ZeroDriftLengthStrokeWidthMap : public CDCHitColorMap {
    public:
      /**
       * Function call to map the CDCHit id and object to a stroke width.
       */
      std::string map(const int iCDCHit, const CDCHit& cdcHit) override;
    };

    /**
     * CDCHit to color map highlighting the CDCHits that posses the do not use flag.
     */
    class TakenFlagColorMap : public CDCHitColorMap {
    public:
      /// Constructor checking if the CDCWireHits caring the taken flag are accessable.
      TakenFlagColorMap();

      /**
       * Function call to map the CDCHit id and object to a color.
       */
      std::string map(const int iCDCHit, const CDCHit& cdcHit) override;

    private:
      /// Memory of the handle to the CDCWireHits on the DataStore.
      StoreWrappedObjPtr<std::vector<CDCWireHit> > m_storedWireHits{"CDCWireHitVector"};
    };

    /**
     * CDCHit to color map by their local right left passage information from Monte Carlo truth
     */
    class RLColorMap : public CDCHitColorMap {
    public:
      /**
       * Function call to map the CDCHit id and object to a color.
       */
      std::string map(const int iCDCHit, const CDCHit& cdcHit) override;
      /**
       * Informal string summarizing the translation from right left passage variable to colors.
       */
      std::string info() override;
    };

    /**
     * CDCHit to color map by their assoziated CDCSimHit::getPosFlag property.
     */
    class PosFlagColorMap : public CDCHitColorMap {
    public:
      /**
       * Function call to map the CDCHit id and object to a color.
       */
      std::string map(const int iCDCHit, const CDCHit& cdcHit) override;
      /**
       * Informal string summarizing the translation from CDCSimHit::getPosFlag variable to colors.
       */
      std::string info() override;
    };

    /**
     * CDCHit to color map by their assoziated CDCSimHit::getBackgroundTag property.
     */
    class BackgroundTagColorMap : public CDCHitColorMap {
    public:
      /**
       * Function call to map the CDCHit id and object to a color.
       */
      std::string map(const int iCDCHit, const CDCHit& cdcHit) override;
      /**
       * Informal string summarizing the translation from CDCSimHit::getBackgroundTag variable to colors.
       */
      std::string info() override;
    };

    /**
     * CDCHit to color map by their Monte Carlo segment id
     */
    class MCSegmentIdColorMap : public CDCHitColorMap {
    public:
      /**
       * Function call to map the CDCHit id and object to a color.
       */
      std::string map(const int iCDCHit, const CDCHit& cdcHit) override;
    };

    /**
     * CDCHit to color map by their assoziated CDCSimHit::getFlightTime.
     */
    class TOFColorMap : public CDCHitColorMap {
    public:
      /**
       * Function call to map the CDCHit id and object to a color.
       */
      std::string map(const int iCDCHit, const CDCHit& cdcHit) override;
      /**
       * Translates the given floating point time of flight to a color.
       */
      std::string timeOfFlightToColor(const double timeOfFlight);
    };

    /**
     * CDCHit to color map indicating the reassignment to a different MCParticle.
     */
    class ReassignedSecondaryMap : public CDCHitColorMap {
    public:
      /**
       * Function call to map the CDCHit id and object to a color.
       */
      std::string map(const int iCDCHit, const CDCHit& cdcHit) override;
    };



    /**
     * CDCHit to color map by their assoziated MCParticle::getArrayId() property.
     */
    class MCParticleColorMap : public CDCHitColorMap {
    public:
      /**
       * Constructor setting up a Monte Carlo id to color map which is continously filled
       * as new during the event.
       */
      MCParticleColorMap(): m_newColors( {}), m_iColor(0) {};
      /**
       * Function call to map the CDCHit id and object to a color.
       */
      std::string map(const int iCDCHit, const CDCHit& cdcHit) override;
    private:
      /// List of colors to be cycled
      ListColors m_listColors;

      /// Mapping of the already used colors by the MCParticle::getArrayId to map later CDCHits to the same color.
      std::map<int, std::string> m_newColors;

      /// Index of the color to be used next.
      int m_iColor;
    };

    /**
     * CDCHit to color map by the associated MCParticle::getPDG()
     */
    class MCPDGCodeColorMap : public ColorByPDGCode {
    public:
      /**
       * Function call to map the CDCHit id and object to a color.
       */
      std::string map(const int iCDCHit, const CDCHit& cdcHit) override;
      /**
       * Informal string summarizing the translation from pdg codes to colors.
       */
      std::string info() override;
    };

    /**
     * CDCHit to color map by the isPrimary information as well as the secondary process type in case the particle is not primary.
     */
    class MCPrimaryColorMap : public CDCHitColorMap {
    public:
      /**
       * Function call to map the CDCHit id and object to a color.
       */
      std::string map(const int iCDCHit, const CDCHit& cdcHit) override;
      /**
       * Informal string summarizing the translation from seconday process codes to colors.
       */
      std::string info() override;
    };

    /**
     * CDCHit to color map by the associated CDCSimHit::getPDG().
     */
    class SimHitPDGCodeColorMap : public ColorByPDGCode {
    public:
      /**
       * Function call to map the CDCHit id and object to a color.
       */
      std::string map(const int iCDCHit, const CDCHit& cdcHit) override;
    };

    /**
     * CDCHit to color map by CDCSimHit::getBackgroundTag().
     */
    class SimHitIsBkgColorMap : public CDCHitColorMap {
    public:
      /**
       * Function call to map the CDCHit id and object to a color.
       */
      std::string map(const int iCDCHit, const CDCHit& cdcHit) override;
    };

  }
}
