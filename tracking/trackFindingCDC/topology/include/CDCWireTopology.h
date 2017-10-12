/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/topology/CDCWireSuperLayer.h>
#include <tracking/trackFindingCDC/topology/CDCWireLayer.h>

#include <tracking/trackFindingCDC/topology/WireNeighborKind.h>
#include <tracking/trackFindingCDC/topology/WireNeighborPair.h>
#include <tracking/trackFindingCDC/topology/CDCWire.h>

#include <tracking/trackFindingCDC/topology/ISuperLayer.h>
#include <tracking/trackFindingCDC/topology/ILayer.h>
#include <tracking/trackFindingCDC/topology/IWire.h>
#include <tracking/trackFindingCDC/topology/EWirePosition.h>

#include <tracking/trackFindingCDC/utilities/MayBePtr.h>

#include <cdc/dataobjects/WireID.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Class representating the sense wire arrangement in the whole of the central drift chamber.
     *  This class represents the the whole CDC layer geometry and the neighborhood relations in it. \n
     *  Since their is only one CDC at a time it is implemented as a singletone object, with getInstance(). \n
     *  Their should rarely be the need to construct additional instances. \n
     *  It provides getters for all wires, layers and superlayers and can check the validity of wire id combinations. \n
     *  Also there are methods for getting to the closest and secondary of each wire. \n
     *  The class does provide the memory for all instances of wires, layers and superlayers.
     */
    class CDCWireTopology  {

    public:
      /// Getter for the singleton instance of the wire topology.
      static CDCWireTopology& getInstance();

    public:
      /// Default constructor for ROOT compatibility. Use rather getInstance to get the shared singleton instance.
      CDCWireTopology()
      { initialize(); }

      /// Disallow copy construction of the wire topology.
      CDCWireTopology(const CDCWireTopology& wireTopology) = delete;

      /// Disallow copy assignment of the wire topology.
      void operator=(const CDCWireTopology& wireTopology) = delete;

    public:
      /// Total number of wires
      static const IWire c_NWires = 14336;

      /// Total number of layers
      static const ILayer c_NLayers = 56;

      /// Total number of superlayers
      static const ISuperLayer c_NSuperLayers = 9;

    private:
      /**
       * Create all wires, wire layers and wire superlayers and their interdependences
       * with base geometry from the CDCGeometryPar.
       */
      void initialize();

    public:
      /// Reload all geometry parameters form the CDCGeometryPar to adjust to changes in geometry
      void reinitialize(EWirePosition wirePosition, bool ignoreWireSag);

    public:
      /**
       *  @name Wire getters
       *  Gets the wires from a WireID.
       */
      ///@{

      /// Checks the validity of a wireID convinience object.
      bool isValidWireID(const WireID& wireID) const
      { return isValidIWire(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire()); }

      /// Checks the validity of the continuous layer id, wire id combination.
      bool isValidIWire(ILayer iCLayer, IWire iWire) const
      { return isValidICLayer(iCLayer) and getWireLayer(iCLayer).isValidIWire(iWire); }

      /// Checks the validity of the superlayer id, layer id, wire id combination.
      bool isValidIWire(ISuperLayer iSuperLayer, ILayer iLayer, IWire iWire) const
      { return isValidISuperLayer(iSuperLayer) and getWireSuperLayer(iSuperLayer).isValidIWire(iLayer, iWire); }

      /// Getter for wire getter by wireID object.
      const CDCWire& getWire(const WireID& wireId) const
      { return getWireLayer(wireId.getICLayer()).getWire(wireId.getIWire()) ; }

      /// Getter for wire getter by continuous layer id and wire id.
      const CDCWire& getWire(ILayer iCLayer, IWire iWire) const
      { return getWireLayer(iCLayer).getWire(iWire) ; }

      /// Getter for wire getter by superlayer id, layer id and wire id.
      const CDCWire& getWire(ISuperLayer iSuperLayer, ILayer iLayer , IWire iWire) const
      { return getWireSuperLayer(iSuperLayer).getWireLayer(iLayer).getWire(iWire) ; }

      /// Getter for the underlying storing wire vector.
      const std::vector<Belle2::TrackFindingCDC::CDCWire>& getWires() const
      { return m_wires; }
      ///@}

      /**
       *  @name Wire layer getters
       *  Getters for the wire layer
       */
      ///@{

      /// Getter for the total number of wire layers
      ILayer getNLayers() const
      { return getWireLayers().size(); }

      /// Checks the validity of the continuous layer id.
      bool isValidICLayer(ILayer iCLayer) const
      { return 0 <= iCLayer and iCLayer < int(getWireLayers().size()); }

      /// Checks the validity of the superlayer id, layer id combination.
      bool isValidILayer(ISuperLayer iSuperLayer, ILayer iLayer) const
      { return isValidISuperLayer(iSuperLayer) and getWireSuperLayer(iSuperLayer).isValidILayer(iLayer); }

      /// Getter for wire layer getter by wireID object.
      const CDCWireLayer& getWireLayer(const WireID& wireId) const
      { return getWireLayer(wireId.getICLayer()); }

      /// Getter for wire layer getter by continuous layer id.
      const CDCWireLayer& getWireLayer(ILayer iCLayer) const
      { return m_wireLayers[ iCLayer ]; }

      /// Getter for wire layer getter by superlayer id and layer id.
      const CDCWireLayer& getWireLayer(ISuperLayer iSuperLayer, ILayer iLayer) const
      { return getWireSuperLayer(iSuperLayer).getWireLayer(iLayer); }

      /// Getter for the underlying storing layer vector
      const std::vector<Belle2::TrackFindingCDC::CDCWireLayer>& getWireLayers() const
      { return m_wireLayers; }
      ///@}

      /**
       *  @name Wire superlayer getters
       *  Getters for the wire superlayers
       */
      ///@{

      /// Getter for the total number of superlayers
      ISuperLayer getNSuperLayers() const
      { return getWireSuperLayers().size(); }

      /// Checks the validity of the superlayer id.
      bool isValidISuperLayer(ISuperLayer iSuperLayer) const
      { return 0 <= iSuperLayer and iSuperLayer < int(getWireSuperLayers().size()); }

      /// Returns the logical superlayer number at the given radius
      ISuperLayer getISuperLayerAtCylindricalR(double cylindricalR);

      /// Getter for wire superlayer getter by wireID object.
      const CDCWireSuperLayer& getWireSuperLayer(const WireID& wireID) const
      { return getWireSuperLayer(wireID.getISuperLayer()); }

      /// Getter for wire superlayer getter by superlayer id.
      const CDCWireSuperLayer& getWireSuperLayer(ISuperLayer iSuperLayer) const
      {
        return m_wireSuperLayers[iSuperLayer];
      }

      /// Getter for the underlying storing superlayer vector
      const std::vector<Belle2::TrackFindingCDC::CDCWireSuperLayer>& getWireSuperLayers() const
      { return m_wireSuperLayers; }
      ///@}

      /**
       *  @name Getters for the closest neighbors of a wire by wireID
       *  They do not cross superlayer boundaries. \n
       *  Note : Not all neighbors might be present at the boundaries of the superlayer. \n
       *  In case the neighbor asked for does not exist the function return nullptr instead.
       */
      /**@{*/

      /**
       *  Checks if two wires are closest neighbors. see details.
       *  Returns the relation of the first wire to the second wire give by their
       *  layer id within the superlayer and the wire id.
       */
      WireNeighborKind getNeighborKind(const WireID& wireID, const WireID& otherWireID) const;

      /// Checks if two wires are primary neighbors.
      bool arePrimaryNeighbors(const WireID& wireID, const WireID& otherWireID) const
      { return getNeighborKind(wireID, otherWireID).getCellDistance() == 1; }

      /// Checks if two wires are secondary neighbors.
      bool areSeconaryNeighbors(const WireID& wireID, const WireID& otherWireID) const
      { return getNeighborKind(wireID, otherWireID).getCellDistance() == 2; }

      /// Getter for the primary neighbor of the given wire id
      MayBePtr<const CDCWire> getPrimaryNeighbor(short oClockDirection, const WireID& wireID) const
      { return getPrimaryNeighbor(oClockDirection, wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire()); }

      /// Getter for the two closest neighbors in the layer outwards.
      WireNeighborPair getNeighborsOutwards(const WireID& wireID) const
      { return getNeighborsOutwards(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire()); }

      /// Getter for the two closest neighbors in the layer inwards.
      WireNeighborPair getNeighborsInwards(const WireID& wireID) const
      { return getNeighborsInwards(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire()); }

      /// Getter for the nearest counterclockwise neighbor in the next layer outwards.
      MayBePtr<const CDCWire> getNeighborCCWOutwards(const WireID& wireID) const
      { return getNeighborCCWOutwards(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire()); }

      /// Getter for the nearest clockwise neighbor in the next layer outwards.
      MayBePtr<const CDCWire> getNeighborCWOutwards(const WireID& wireID) const
      { return getNeighborCWOutwards(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire()); }

      /// Getter for the nearest counterclockwise neighbor.
      MayBePtr<const CDCWire> getNeighborCCW(const WireID& wireID) const
      { return getNeighborCCW(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire()); }

      /// Getter for the nearest clockwise neighbor.
      MayBePtr<const CDCWire> getNeighborCW(const WireID& wireID) const
      { return getNeighborCW(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire()); }

      /// Getter for the nearest counterclockwise neighbor in the next layer outwards.
      MayBePtr<const CDCWire> getNeighborCCWInwards(const WireID& wireID) const
      { return getNeighborCCWInwards(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire()); }

      /// Getter for the nearest clockwise neighbor in the next layer outwards.
      MayBePtr<const CDCWire> getNeighborCWInwards(const WireID& wireID) const
      { return getNeighborCWInwards(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire()); }
      /**@}*/


      /**
       *  @name Getters for the closest neighbors of a wire by superlayer id, layer id and wire id
       *  They do not cross superlayer boundaries. \n
       *  Note : Not all neighbors might be present at the boundaries of the superlayer. \n
       *  In case the neighbor asked for does not exist the function return nullptr instead. \n
       */
      /**@{*/

      /// Getter for the primary neighbor of the given wire id following the direction on the clock
      MayBePtr<const CDCWire> getPrimaryNeighbor(short oClockDirection,
                                                 ISuperLayer iSuperLayer,
                                                 ILayer iLayer,
                                                 IWire iWire) const
      { return getWireSuperLayer(iSuperLayer).getPrimaryNeighbor(oClockDirection, iLayer, iWire);}


      /// Getter for the two closest neighbors in the layer outwards.
      WireNeighborPair getNeighborsOutwards(ISuperLayer iSuperLayer,
                                            ILayer iLayer,
                                            IWire iWire) const
      { return getWireSuperLayer(iSuperLayer).getNeighborsOutwards(iLayer, iWire); }

      /// Getter for the two closest neighbors in the layer inwards.
      WireNeighborPair getNeighborsInwards(ISuperLayer iSuperLayer,
                                           ILayer iLayer,
                                           IWire iWire) const
      { return getWireSuperLayer(iSuperLayer).getNeighborsInwards(iLayer, iWire); }

      /// Getter for the nearest counterclockwise neighbor in the next layer outwards.
      MayBePtr<const CDCWire> getNeighborCCWOutwards(ISuperLayer iSuperLayer,
                                                     ILayer iLayer,
                                                     IWire iWire) const
      { return getWireSuperLayer(iSuperLayer).getNeighborCCWOutwards(iLayer, iWire); }

      /// Getter for the nearest clockwise neighbor in the next layer outwards.
      MayBePtr<const CDCWire> getNeighborCWOutwards(ISuperLayer iSuperLayer,
                                                    ILayer iLayer,
                                                    IWire iWire) const
      { return getWireSuperLayer(iSuperLayer).getNeighborCWOutwards(iLayer, iWire); }

      /// Getter for the nearest counterclockwise neighbor.
      MayBePtr<const CDCWire> getNeighborCCW(ISuperLayer iSuperLayer,
                                             ILayer iLayer,
                                             IWire iWire) const
      { return getWireSuperLayer(iSuperLayer).getNeighborCCW(iLayer, iWire); }

      /// Getter for the nearest clockwise neighbor.
      MayBePtr<const CDCWire> getNeighborCW(ISuperLayer iSuperLayer,
                                            ILayer iLayer,
                                            IWire iWire) const
      { return getWireSuperLayer(iSuperLayer).getNeighborCW(iLayer, iWire); }

      /// Getter for the nearest counterclockwise neighbor in the next layer outwards.
      MayBePtr<const CDCWire> getNeighborCCWInwards(ISuperLayer iSuperLayer,
                                                    ILayer iLayer,
                                                    IWire iWire) const
      { return getWireSuperLayer(iSuperLayer).getNeighborCCWInwards(iLayer, iWire); }

      /// Getter for the nearest clockwise neighbor in the next layer outwards.
      MayBePtr<const CDCWire> getNeighborCWInwards(ISuperLayer iSuperLayer,
                                                   ILayer iLayer,
                                                   IWire iWire) const
      { return getWireSuperLayer(iSuperLayer).getNeighborCWInwards(iLayer, iWire); }
      /**@}*/

      /**
       *  @name Secondary neighbor getters by wireID object
       *  The secondary neighbors are numbered like positions on the normal twelve hour clock.
       *
       *  The rational of this is that the secondary neighbors are positioned \n
       *  just like the numbers on the clock if you took the center of the clock to the wire position \n
       *  and the twelve o'clock points in the direction outwards. \n
       *
       *  Note : Not all neighbors might be present at the boundaries of the superlayer. \n
       *  In case the neighbor asked for does not exist the function return nullptr instead.
       */
      /**@{*/

      /// Getter for the secondary neighbor of the given wire id
      MayBePtr<const CDCWire> getSecondaryNeighbor(short oClockDirection, const WireID& wireID) const
      { return getSecondaryNeighbor(oClockDirection, wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire()); }

      /// Getter for secondary neighbor at the one o'clock position
      MayBePtr<const CDCWire> getSecondNeighborOneOClock(const WireID& wireID) const
      { return getSecondNeighborOneOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire()); }

      /// Getter for secondary neighbor at the two o'clock position
      MayBePtr<const CDCWire> getSecondNeighborTwoOClock(const WireID& wireID) const
      { return getSecondNeighborTwoOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire()); }

      /// Getter for secondary neighbor at the three o'clock position - always exists
      MayBePtr<const CDCWire> getSecondNeighborThreeOClock(const WireID& wireID) const
      { return getSecondNeighborThreeOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire()); }

      /// Getter for secondary neighbor at the four o'clock position
      MayBePtr<const CDCWire> getSecondNeighborFourOClock(const WireID& wireID) const
      { return getSecondNeighborFourOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire()); }

      /// Getter for secondary neighbor at the five o'clock position
      MayBePtr<const CDCWire> getSecondNeighborFiveOClock(const WireID& wireID) const
      { return getSecondNeighborFiveOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire()); }

      /// Getter for secondary neighbor at the six o'clock position
      MayBePtr<const CDCWire> getSecondNeighborSixOClock(const WireID& wireID) const
      { return getSecondNeighborSixOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire()); }

      /// Getter for secondary neighbor at the seven o'clock position
      MayBePtr<const CDCWire> getSecondNeighborSevenOClock(const WireID& wireID) const
      { return getSecondNeighborSevenOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire()); }

      /// Getter for secondary neighbor at the eight o'clock position
      MayBePtr<const CDCWire> getSecondNeighborEightOClock(const WireID& wireID) const
      { return getSecondNeighborEightOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire()); }

      /// Getter for secondary neighbor at the nine o'clock position - always exists
      MayBePtr<const CDCWire> getSecondNeighborNineOClock(const WireID& wireID) const
      { return getSecondNeighborNineOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire()); }

      /// Getter for secondary neighbor at the ten o'clock position
      MayBePtr<const CDCWire> getSecondNeighborTenOClock(const WireID& wireID) const
      { return getSecondNeighborTenOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire()); }

      /// Getter for secondary neighbor at the eleven o'clock position
      MayBePtr<const CDCWire> getSecondNeighborElevenOClock(const WireID& wireID) const
      { return getSecondNeighborElevenOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire()); }

      /// Getter for secondary neighbor at the twelve o'clock position
      MayBePtr<const CDCWire> getSecondNeighborTwelveOClock(const WireID& wireID) const
      { return getSecondNeighborTwelveOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire()); }
      /**@}*/

      /** @name Secondary neighbor getters by superlayer id , layer id and wire id.
       *  The secondary neighbors are numbered like positions on the normal twelve hour clock.
       *
       *  The rational of this is that the secondary neighbors are positioned \n
       *  just like the numbers on the clock if you took the center of the clock to the wire position \n
       *  and the twelve o'clock points in the direction outwards.
       *
       *  Note : Not all neighbors might be present at the boundaries of the superlayer. \n
       *  In case the neighbor asked for does not exist the function return nullptr instead.
       */
      /**@{*/

      /// Getter for the secondary neighbor of the given wire id following the direction on the clock
      MayBePtr<const CDCWire> getSecondaryNeighbor(short oClockDirection,
                                                   ISuperLayer iSuperLayer,
                                                   ILayer iLayer,
                                                   IWire iWire) const
      { return getWireSuperLayer(iSuperLayer).getSecondaryNeighbor(oClockDirection, iLayer, iWire);}

      /// Getter for secondary neighbor at the one o'clock position
      MayBePtr<const CDCWire> getSecondNeighborOneOClock(ISuperLayer iSuperLayer,
                                                         ILayer iLayer,
                                                         IWire  iWire) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighborOneOClock(iLayer, iWire); }

      /// Getter for secondary neighbor at the two o'clock position
      MayBePtr<const CDCWire> getSecondNeighborTwoOClock(ISuperLayer iSuperLayer,
                                                         ILayer iLayer,
                                                         IWire  iWire) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighborTwoOClock(iLayer, iWire); }

      /// Getter for secondary neighbor at the three o'clock position - always exists
      MayBePtr<const CDCWire> getSecondNeighborThreeOClock(ISuperLayer iSuperLayer,
                                                           ILayer iLayer,
                                                           IWire  iWire) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighborThreeOClock(iLayer, iWire); }

      /// Getter for secondary neighbor at the four o'clock position
      MayBePtr<const CDCWire> getSecondNeighborFourOClock(ISuperLayer iSuperLayer,
                                                          ILayer iLayer,
                                                          IWire  iWire) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighborFourOClock(iLayer, iWire); }

      /// Getter for secondary neighbor at the five o'clock position
      MayBePtr<const CDCWire> getSecondNeighborFiveOClock(ISuperLayer iSuperLayer,
                                                          ILayer iLayer,
                                                          IWire  iWire) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighborFiveOClock(iLayer, iWire); }

      /// Getter for secondary neighbor at the six o'clock position
      MayBePtr<const CDCWire> getSecondNeighborSixOClock(ISuperLayer iSuperLayer,
                                                         ILayer iLayer,
                                                         IWire  iWire) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighborSixOClock(iLayer, iWire); }

      /// Getter for secondary neighbor at the seven o'clock position
      MayBePtr<const CDCWire> getSecondNeighborSevenOClock(ISuperLayer iSuperLayer,
                                                           ILayer iLayer,
                                                           IWire  iWire) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighborSevenOClock(iLayer, iWire); }

      /// Getter for secondary neighbor at the eight o'clock position
      MayBePtr<const CDCWire> getSecondNeighborEightOClock(ISuperLayer iSuperLayer,
                                                           ILayer iLayer,
                                                           IWire  iWire) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighborEightOClock(iLayer, iWire); }

      /// Getter for secondary neighbor at the nine o'clock position - always exists
      MayBePtr<const CDCWire> getSecondNeighborNineOClock(ISuperLayer iSuperLayer,
                                                          ILayer iLayer,
                                                          IWire  iWire) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighborNineOClock(iLayer, iWire); }

      /// Getter for secondary neighbor at the ten o'clock position
      MayBePtr<const CDCWire> getSecondNeighborTenOClock(ISuperLayer iSuperLayer,
                                                         ILayer iLayer,
                                                         IWire  iWire) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighborTenOClock(iLayer, iWire); }

      /// Getter for secondary neighbor at the eleven o'clock position
      MayBePtr<const CDCWire> getSecondNeighborElevenOClock(ISuperLayer iSuperLayer,
                                                            ILayer iLayer,
                                                            IWire  iWire) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighborElevenOClock(iLayer, iWire); }

      /// Getter for secondary neighbor at the twelve o'clock position
      MayBePtr<const CDCWire> getSecondNeighborTwelveOClock(ISuperLayer iSuperLayer,
                                                            ILayer iLayer,
                                                            IWire  iWire) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighborTwelveOClock(iLayer, iWire); }
      /**@}*/

    public:
      /// Getter for the outer radius of the outer most wire layer.
      double getOuterCylindricalR() const
      { return m_wireLayers.back().getOuterCylindricalR(); }

      /// Getter for the inner radius of the inner most wire layer.
      double getInnerCylindricalR() const
      { return m_wireLayers.front().getInnerCylindricalR(); }

    private:
      /// Storage for all wires in the CDC
      std::vector<Belle2::TrackFindingCDC::CDCWire> m_wires;

      /// Storage for all wire layers in the CDC
      std::vector<Belle2::TrackFindingCDC::CDCWireLayer> m_wireLayers;

      /// Storage for all wire superlayers in the CDC
      std::vector<Belle2::TrackFindingCDC::CDCWireSuperLayer> m_wireSuperLayers;

    };
  }
}
