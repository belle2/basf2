/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCWIRETOPOLOGY_H
#define CDCWIRETOPOLOGY_H

#include <algorithm>
#include <set>

#include <TVector3.h>

#include <cdc/dataobjects/WireID.h>

#include <tracking/cdcLocalTracking/rootification/SwitchableRootificationBase.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/topology/CDCWire.h>
#include <tracking/cdcLocalTracking/topology/CDCWireLayer.h>
#include <tracking/cdcLocalTracking/topology/CDCWireSuperLayer.h>

namespace Belle2 {
  namespace CDCLocalTracking {


    /// Class representating the sense wire arrangement in the whole of the central drift chamber.
    /** This class represents the the whole CDC layer geometry and the neighborhood relations in it. \n
     *  Since their is only one CDC at a time it is implemented as a singletone object, with getInstance(). \n
     *  Their should rarely be the need to construct  additional instances. \n
     *  It provides getters for all wires, layers and superlayers and can check the validity of wire id combinations. \n
     *  It also presents an iterable range of all wires in the CDC. \n
     *  The range is sorted with increasing wire ids for maximal fast look up. \n
     *  Also there are methods for getting to the closest and secondary of each wire. \n
     *  The class does provide the memory for all instances of wires, layers and superlayers by its self.
     */
    class CDCWireTopology : public CDCLocalTracking::SwitchableRootificationBase {

    public:
      /// The underlying container type storing range of wires represented by this class
      typedef std::vector<Belle2::CDCLocalTracking::CDCWire> Container;
      /// The value type of the iterator range
      typedef Belle2::CDCLocalTracking::CDCWire value_type;

      /// The type of the iterator for the wire range
      typedef Container::iterator iterator;

      /// The type of the const_iterator for the wire range
      typedef Container::const_iterator const_iterator;

      /// A wire pointer pair as returned from getNeighborsOutward(), getNeighborsInward()
      typedef CDCWireSuperLayer::NeighborPair NeighborPair;

    public:

      /// Default constructor for ROOT compatibility. Use rather getInstance to get the shared singleton instance.
      CDCWireTopology() { initialize(); }

      /// Empty destructor
      ~CDCWireTopology() {;}

    public:
      /// Total number of wires
      static const IWireType N_WIRES = 14336;
      ///Total number of layers
      static const ILayerType N_LAYERS = 56;
      ///Total number of superlayers
      static const ILayerType N_SUPERLAYERS = 9;

      /// Initializes all wires, wire layers and wire superlayers their interdependences from the CDCGeometryPar.
      void initialize();

      /** @name Wire range
       */
      /**@{*/
      /// Getter for the begin iterator of the whole wire range
      const_iterator begin() const { return m_wires.begin(); }

      /// Getter for the end iterator of the whole wire range
      const_iterator end() const { return m_wires.end(); }

      /// Iteration helper for python only
      /** In order to retrieve wires from a layer from python the stl iterators are not usable, \n
       *  so we give an other mechanism for iteration. \n
       *  To get the first wire call the function with no argument. \n
       *  Call the function with the last wire yielded to get the next one. \n
       *  If there is no next wire anymore at the end of the wire range return nullptr */
      //const CDCWire * nextWire(const CDCWire * wire = nullptr) const;

      /// Getter for the total number of wires
      IWireType size() const { return m_wires.size(); }

      /// Getter for the first wire in the whole cdc
      const CDCWire& first() const { return *(begin()); }
      /// Getter for the last wire in the whole cdc
      const CDCWire& last() const { return *(--end()); }

      /**@}*/

      /** @name Wire getters
       *  Gets the wires from a WireID. */
      ///@{

      /// Checks the validity of a wireID convinience object.
      inline bool isValidIWire(const WireID& wireID) const
      { return isValidIWire(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire()); }

      /// Checks the validity of the continuous layer id, wire id combination.
      inline bool isValidIWire(const ILayerType& iCLayer, const IWireType& iWire) const
      { return isValidICLayer(iCLayer) and getWireLayer(iCLayer).isValidIWire(iWire); }

      /// Checks the validity of the superlayer id, layer id, wire id combination.
      inline
      bool
      isValidIWire(
        const ILayerType& iSuperLayer,
        const ILayerType& iLayer,
        const IWireType& iWire
      ) const
      { return isValidISuperLayer(iSuperLayer) and getWireSuperLayer(iSuperLayer).isValidIWire(iLayer, iWire); }

      /// Getter for wire getter by wireID object.
      const CDCWire& getWire(const WireID& wireId) const
      { return getWireLayer(wireId.getICLayer()).getWire(wireId.getIWire()) ; }

      /// Getter for wire getter by continuous layer id and wire id.
      const CDCWire& getWire(const ILayerType& iCLayer , const IWireType& iWire) const
      { return getWireLayer(iCLayer).getWire(iWire) ; }

      /// Getter for wire getter by superlayer id, layer id and wire id.
      const CDCWire& getWire(const ILayerType& iSuperLayer, const ILayerType& iLayer , const IWireType& iWire) const
      { return getWireSuperLayer(iSuperLayer).getWireLayer(iLayer).getWire(iWire) ; }

      /// Getter for the underlying storing wire vector.
      const std::vector<Belle2::CDCLocalTracking::CDCWire>& getWires() const
      { return m_wires; }
      ///@}

      /** @name Wire layer getters
       *  Getters for the wire layer*/
      ///@{
      /// Getter for the total number of wire layers
      inline ILayerType getNLayers() const
      { return getWireLayers().size(); }

      /// Checks the validity of the continuous layer id.
      inline bool isValidICLayer(const ILayerType& iCLayer) const
      { return 0 <= iCLayer and iCLayer < int(getWireLayers().size()); }
      /// Checks the validity of the superlayer id, layer id combination.
      inline bool isValidILayer(const ILayerType& iSuperLayer, const ILayerType& iLayer) const
      { return isValidISuperLayer(iSuperLayer) and getWireSuperLayer(iSuperLayer).isValidILayer(iLayer); }

      /// Getter for wire layer getter by wireID object.
      const CDCWireLayer& getWireLayer(const WireID& wireId) const
      { return getWireLayer(wireId.getICLayer()); }

      /// Getter for wire layer getter by continuous layer id.
      const CDCWireLayer& getWireLayer(const ILayerType& iCLayer) const
      { return m_wireLayers[ iCLayer ]; }

      /// Getter for wire layer getter by superlayer id and layer id.
      const CDCWireLayer& getWireLayer(const ILayerType& iSuperLayer, const ILayerType& iLayer) const
      { return getWireSuperLayer(iSuperLayer).getWireLayer(iLayer); }

      /// Getter for the underlying storing layer vector
      const std::vector<Belle2::CDCLocalTracking::CDCWireLayer>& getWireLayers() const
      { return m_wireLayers; }
      ///@}

      /** @name Wire superlayer getters
       *  Getters for the wire superlayers */
      ///@{
      /// Getter for the total number of superlayers
      inline ILayerType getNSuperLayers() const
      { return getWireSuperLayers().size(); }

      /// Checks the validity of the superlayer id.
      inline bool isValidISuperLayer(const ILayerType& iSuperLayer) const
      { return 0 <= iSuperLayer and iSuperLayer < int(getWireSuperLayers().size()); }

      /// Getter for wire superlayer getter by wireID object.
      const CDCWireSuperLayer& getWireSuperLayer(const WireID& wireId) const
      { return getWireSuperLayer(wireId.getISuperLayer()); }

      /// Getter for wire superlayer getter by superlayer id.
      const CDCWireSuperLayer& getWireSuperLayer(const ILayerType& iSuperLayer) const
      { return m_wireSuperLayers[ iSuperLayer ]; }

      /// Getter for the underlying storing superlayer vector
      const std::vector<Belle2::CDCLocalTracking::CDCWireSuperLayer>& getWireSuperLayers() const
      { return m_wireSuperLayers; }
      ///@}


      /** @name Getters for the closest neighbors of a wire by wireID
       *  They do not cross superlayer boundaries. \n
       *  Note : Not all neighbors might be present at the boundaries of the superlayer. \n
       *  In case the neighbor asked for does not exist the function return nullptr instead.
       */
      /**@{*/

      /// Checks if two wires are closest neighbors. see details.
      /** Returns the relation of the first wire to the second wire give by their
       *  layer id within the superlayer and the wire id.
       *  If they are not neighbors zero is returned.
       *  If they are neighbors the return value indicates
       *  the direction to go from the first wire to the second. \n
       *  The return value is one of \n
       *  0 for the wires being no neighbors \n
       *  CW_OUT_NEIGHBOR = 1  for clockwise outwards \n
       *  CW_NEIGHBOR = 3 for clockwise \n
       *  CW_IN_NEIGHBOR = 5 for clockwise inwards \n
       *  CCW_IN_NEIGHBOR = 7 for counterclockwise inwards \n
       *  CCW_NEIGHBOR = 9 for counterclockwise \n
       *  CCW_OUT_NEIGHBOR = 11 for counterclockwise outwards \n
       *  The values are choosen to have an assoziation with the numbers on a regular clock.
       *  ( compare secondary neighborhood ) */
      WireNeighborType areNeighbors(const WireID& wire, const WireID& other) const;

      /// Getter for the two closest neighbors in the layer outwards.
      NeighborPair
      getNeighborsOutwards(
        const WireID& wireID
      ) const
      { return getNeighborsOutwards(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      /// Getter for the two closest neighbors in the layer inwards.
      NeighborPair
      getNeighborsInwards(
        const WireID& wireID
      ) const
      { return getNeighborsInwards(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      /// Getter for the nearest counterclockwise neighbor in the next layer outwards.
      const CDCWire*
      getNeighborCCWOutwards(
        const WireID& wireID
      ) const
      { return getNeighborCCWOutwards(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      /// Getter for the nearest clockwise neighbor in the next layer outwards.
      const CDCWire*
      getNeighborCWOutwards(
        const WireID& wireID
      ) const
      { return getNeighborCWOutwards(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      /// Getter for the nearest counterclockwise neighbor.
      const CDCWire*
      getNeighborCCW(
        const WireID& wireID
      ) const
      { return getNeighborCCW(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      /// Getter for the nearest clockwise neighbor.
      const CDCWire*
      getNeighborCW(
        const WireID& wireID
      ) const
      { return getNeighborCW(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      /// Getter for the nearest counterclockwise neighbor in the next layer outwards.
      const CDCWire*
      getNeighborCCWInwards(
        const WireID& wireID
      ) const
      { return getNeighborCCWInwards(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      /// Getter for the nearest clockwise neighbor in the next layer outwards.
      const CDCWire*
      getNeighborCWInwards(
        const WireID& wireID
      ) const
      { return getNeighborCWInwards(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }
      /**@}*/


      /** @name Getters for the closest neighbors of a wire by superlayer id, layer id and wire id
       *  They do not cross superlayer boundaries. \n
       *  Note : Not all neighbors might be present at the boundaries of the superlayer. \n
       *  In case the neighbor asked for does not exist the function return nullptr instead. \n
       */
      /**@{*/
      /// Getter for the two closest neighbors in the layer outwards.
      NeighborPair
      getNeighborsOutwards(
        const ILayerType& iSuperLayer,
        const ILayerType& iLayer,
        const IWireType& iWire
      ) const
      { return getWireSuperLayer(iSuperLayer).getNeighborsOutwards(iLayer, iWire); }

      /// Getter for the two closest neighbors in the layer inwards.
      NeighborPair
      getNeighborsInwards(
        const ILayerType& iSuperLayer,
        const ILayerType& iLayer,
        const IWireType& iWire
      ) const
      { return getWireSuperLayer(iSuperLayer).getNeighborsInwards(iLayer, iWire); }

      /// Getter for the nearest counterclockwise neighbor in the next layer outwards.
      const CDCWire*
      getNeighborCCWOutwards(
        const ILayerType& iSuperLayer,
        const ILayerType& iLayer,
        const IWireType& iWire
      ) const
      { return getWireSuperLayer(iSuperLayer).getNeighborCCWOutwards(iLayer, iWire); }

      /// Getter for the nearest clockwise neighbor in the next layer outwards.
      const CDCWire*
      getNeighborCWOutwards(
        const ILayerType& iSuperLayer,
        const ILayerType& iLayer,
        const IWireType& iWire
      ) const
      { return getWireSuperLayer(iSuperLayer).getNeighborCWOutwards(iLayer, iWire); }

      /// Getter for the nearest counterclockwise neighbor.
      const CDCWire*
      getNeighborCCW(
        const ILayerType& iSuperLayer,
        const ILayerType& iLayer,
        const IWireType& iWire
      ) const
      { return getWireSuperLayer(iSuperLayer).getNeighborCCW(iLayer, iWire); }

      /// Getter for the nearest clockwise neighbor.
      const CDCWire*
      getNeighborCW(
        const ILayerType& iSuperLayer,
        const ILayerType& iLayer,
        const IWireType& iWire
      ) const
      { return getWireSuperLayer(iSuperLayer).getNeighborCW(iLayer, iWire); }

      /// Getter for the nearest counterclockwise neighbor in the next layer outwards.
      const CDCWire*
      getNeighborCCWInwards(
        const ILayerType& iSuperLayer,
        const ILayerType& iLayer,
        const IWireType& iWire
      ) const
      { return getWireSuperLayer(iSuperLayer).getNeighborCCWInwards(iLayer, iWire); }

      /// Getter for the nearest clockwise neighbor in the next layer outwards.
      const CDCWire*
      getNeighborCWInwards(
        const ILayerType& iSuperLayer,
        const ILayerType& iLayer,
        const IWireType& iWire
      ) const
      { return getWireSuperLayer(iSuperLayer).getNeighborCWInwards(iLayer, iWire); }
      /**@}*/



      /** @name Secondary neighbor getters by wireID object
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

      ///Getter for secondary neighbor at the one o'clock position
      const CDCWire*
      getSecondNeighorOneOClock(
        const WireID& wireID
      ) const
      { return getSecondNeighorOneOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      ///Getter for secondary neighbor at the two o'clock position
      const CDCWire*
      getSecondNeighorTwoOClock(
        const WireID& wireID
      ) const
      { return getSecondNeighorTwoOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      ///Getter for secondary neighbor at the three o'clock position
      const CDCWire*
      getSecondNeighorThreeOClock(
        const WireID& wireID
      ) const
      { return getSecondNeighorThreeOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      ///Getter for secondary neighbor at the four o'clock position
      const CDCWire*
      getSecondNeighorFourOClock(
        const WireID& wireID
      ) const
      { return getSecondNeighorFourOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      ///Getter for secondary neighbor at the five o'clock position
      const CDCWire*
      getSecondNeighorFiveOClock(
        const WireID& wireID
      ) const
      { return getSecondNeighorFiveOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      ///Getter for secondary neighbor at the six o'clock position
      const CDCWire*
      getSecondNeighorSixOClock(
        const WireID& wireID
      ) const
      { return getSecondNeighorSixOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      ///Getter for secondary neighbor at the seven o'clock position
      const CDCWire*
      getSecondNeighorSevenOClock(
        const WireID& wireID
      ) const
      { return getSecondNeighorSevenOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      ///Getter for secondary neighbor at the eight o'clock position
      const CDCWire*
      getSecondNeighorEightOClock(
        const WireID& wireID
      ) const
      { return getSecondNeighorEightOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      ///Getter for secondary neighbor at the nine o'clock position
      const CDCWire*
      getSecondNeighorNineOClock(
        const WireID& wireID
      ) const
      { return getSecondNeighorNineOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      ///Getter for secondary neighbor at the ten o'clock position
      const CDCWire*
      getSecondNeighorTenOClock(
        const WireID& wireID
      ) const
      { return getSecondNeighorTenOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      ///Getter for secondary neighbor at the eleven o'clock position
      const CDCWire*
      getSecondNeighorElevenOClock(
        const WireID& wireID
      ) const
      { return getSecondNeighorElevenOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }

      ///Getter for secondary neighbor at the twelve o'clock position
      const CDCWire*
      getSecondNeighorTwelveOClock(
        const WireID& wireID
      ) const
      { return getSecondNeighorTwelveOClock(wireID.getISuperLayer(), wireID.getILayer(), wireID.getIWire());  }
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

      ///Getter for secondary neighbor at the one o'clock position
      const CDCWire*
      getSecondNeighorOneOClock(
        const ILayerType& iSuperLayer,
        const ILayerType& iLayer,
        const IWireType&   iWire
      ) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighorOneOClock(iLayer, iWire);  }

      ///Getter for secondary neighbor at the two o'clock position
      const CDCWire*
      getSecondNeighorTwoOClock(
        const ILayerType& iSuperLayer,
        const ILayerType& iLayer,
        const IWireType& iWire
      ) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighorTwoOClock(iLayer, iWire); }

      ///Getter for secondary neighbor at the three o'clock position
      const CDCWire*
      getSecondNeighorThreeOClock(
        const ILayerType& iSuperLayer,
        const ILayerType& iLayer,
        const IWireType& iWire
      ) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighorThreeOClock(iLayer, iWire); }

      ///Getter for secondary neighbor at the four o'clock position
      const CDCWire*
      getSecondNeighorFourOClock(
        const ILayerType& iSuperLayer,
        const ILayerType& iLayer,
        const IWireType& iWire
      ) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighorFourOClock(iLayer, iWire); }

      ///Getter for secondary neighbor at the five o'clock position
      const CDCWire*
      getSecondNeighorFiveOClock(
        const ILayerType& iSuperLayer,
        const ILayerType& iLayer,
        const IWireType& iWire
      ) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighorFiveOClock(iLayer, iWire); }

      ///Getter for secondary neighbor at the six o'clock position
      const CDCWire*
      getSecondNeighorSixOClock(
        const ILayerType& iSuperLayer,
        const ILayerType& iLayer,
        const IWireType& iWire
      ) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighorSixOClock(iLayer, iWire); }

      ///Getter for secondary neighbor at the seven o'clock position
      const CDCWire*
      getSecondNeighorSevenOClock(
        const ILayerType& iSuperLayer,
        const ILayerType& iLayer,
        const IWireType& iWire
      ) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighorSevenOClock(iLayer, iWire); }

      ///Getter for secondary neighbor at the eight o'clock position
      const CDCWire*
      getSecondNeighorEightOClock(
        const ILayerType& iSuperLayer,
        const ILayerType& iLayer,
        const IWireType& iWire
      ) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighorEightOClock(iLayer, iWire); }

      ///Getter for secondary neighbor at the nine o'clock position
      const CDCWire*
      getSecondNeighorNineOClock(
        const ILayerType& iSuperLayer,
        const ILayerType& iLayer,
        const IWireType& iWire
      ) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighorNineOClock(iLayer, iWire); }

      ///Getter for secondary neighbor at the ten o'clock position
      const CDCWire*
      getSecondNeighorTenOClock(
        const ILayerType& iSuperLayer,
        const ILayerType& iLayer,
        const IWireType& iWire
      ) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighorTenOClock(iLayer, iWire); }

      ///Getter for secondary neighbor at the eleven o'clock position
      const CDCWire*
      getSecondNeighorElevenOClock(
        const ILayerType& iSuperLayer,
        const ILayerType& iLayer,
        const IWireType& iWire
      ) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighorElevenOClock(iLayer, iWire); }

      ///Getter for secondary neighbor at the twelve o'clock position
      const CDCWire*
      getSecondNeighorTwelveOClock(
        const ILayerType& iSuperLayer,
        const ILayerType& iLayer,
        const IWireType& iWire
      ) const
      { return getWireSuperLayer(iSuperLayer).getSecondNeighorTwelveOClock(iLayer, iWire); }
      /**@}*/





    private:
      /// Storage for all wires in the CDC
      std::vector<Belle2::CDCLocalTracking::CDCWire> m_wires;
      /// Storage for all wire layers in the CDC
      std::vector<Belle2::CDCLocalTracking::CDCWireLayer> m_wireLayers;
      /// Storage for all wire superlayers in the CDC
      std::vector<Belle2::CDCLocalTracking::CDCWireSuperLayer> m_wireSuperLayers;

      //The singleton implementation for the wire topology.
    private:
      static CDCWireTopology* s_instance;  ///< The Topology instance to be used by the CDCWires

    public:
      /// Getter for the singleton instance of the wire topology. Used by the CDCWires.
      static CDCWireTopology& getInstance() {
        if (not s_instance) s_instance = new CDCWireTopology;
        return *s_instance;
      }


    private:
      /// ROOT Macro to make CDCWireTopology a ROOT class
      CDCLOCALTRACKING_SwitchableClassDef(CDCWireTopology, 1);

    }; //class


  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCWIRETOPOLOGY
