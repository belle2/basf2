/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/topology/CDCWireLayer.h>

namespace Belle2 {
  namespace TrackFindingCDC {


    /// Class representating a sense wire superlayer in the central drift chamber.
    /** This class represents the a CDC superlayer as a range of wire layers. \n
     *  The range is sorted with increasing layer ids for maximal fast look up. \n
     *  It provides methods for checking the validity of layer and wire ids as well as \n
     *  a getter for the layers and wires in this layer. \n
     *  Additionally it is iterable as a range of CDCWireLayers. \n
     *  The superlayer also defines the closest and the secondary neighborhood. \n
     *  The class does not provide the memory for the wire layers by itself. \n
     *  It uses iterators into a vector of CDCWireLayers. \n
     *  The vector of CDCWireLayers is normally provided by the CDCWireTopology object instance. \n
     * Note : All possible superlayer object are stored in the CDCWireTopology \n
     * which you can get with the static getInstance() functions. \n
     * There is rarely a need for constructing a superlayer object and \n
     * it should be avoided for speed reasons. */
    class CDCWireSuperLayer  {


    private:
      static ERotation getShiftDelta(ERotation rhs, ERotation lhs)
      { return static_cast<ERotation>(rhs - lhs); }

    public:
      /// The underlying container type storing range of layers referenced by this class
      typedef std::vector<Belle2::TrackFindingCDC::CDCWireLayer> Container;

      /// The value type of the iterator range
      typedef Belle2::TrackFindingCDC::CDCWireLayer value_type;

      /// The type of the iterator for the layer range
      typedef Container::iterator iterator;

      /// The type of the const_iterator for the layer range
      typedef Container::const_iterator const_iterator;

      /// A wire pointer pair as returned from getNeighborsOutward(), getNeighborsInward()
      typedef std::pair<const Belle2::TrackFindingCDC::CDCWire*, const Belle2::TrackFindingCDC::CDCWire*> NeighborPair;

      /// Constructor taking the range of layers the superlayer shall contain. Use rather getInstance() to avoid instance constructions.
      CDCWireSuperLayer(const const_iterator& begin, const const_iterator& end);

    public:
      /// Intializes the superlayer variables of according the layer range. Set the numbering shift of contained layers.
      void initialize();

      /** @name Superlayer index
        */
      /**@{*/
      /// Getter for the super layer id
      ISuperLayer getISuperLayer() const { return first().getISuperLayer(); }
      /**@}*/

      /** @name Layer range
       */
      /**@{*/
      /// Getter for the begin iterator of the layer range
      const_iterator begin() const { return m_begin; }
      /// Getter for the end iterator of the layer range
      const_iterator end() const { return m_end; }

      /// Gives the number of wire in this superlayer
      size_t size() const { return m_end - m_begin; }

      /// Getter for the first layer of this superlayer
      const CDCWireLayer& first() const { return *(begin()); }

      /// Getter for the last layer of this superlayer
      const CDCWireLayer& last() const { return *(--end()); }
      /**@}*/

      /** @name Layer getters
       */
      /**@{*/
      /// Checks if the given layer id belongs to a valid layer in this superlayer
      inline bool isValidILayer(const ILayerType& iLayer) const
      { return 0 <= iLayer and iLayer < int(size()); }

      /// Gives the layer by its layer id within the superlayer
      const CDCWireLayer& getWireLayer(const ILayerType& iLayer) const
      { return *(begin() + iLayer); }
      /**@}*/

      /** @name Wire getters
       */
      /**@{*/
      /// Checks if the given wire id belongs to a valid wire in this superlayer
      inline bool isValidIWire(const ILayerType& iLayer, const IWireType& iWire) const
      { return isValidILayer(iLayer) and getWireLayer(iLayer).isValidIWire(iWire); }

      /// Gives the wire by its layer id within the superlayer and the wire id in the layer.
      const CDCWire& getWire(const ILayerType& iLayer, const IWireType& iWire) const
      { return getWireLayer(iLayer).getWire(iWire); }
      /**@}*/


      /** @name Geometry properties
        */
      /**@{*/
      /// Indicates if the wire is axial or stereo
      bool isAxial() const { return getStereoKind() == EStereoKind::c_Axial; }

      /// Getter for the stereo type of the wire layer
      /** Gives the stereo type of the wire. \n
       *  Result is one of EStereoKind::c_Axial, EStereoKind::c_StereoU and EStereoKind::c_StereoV. \n
       *  The stereo type is shared by all wires in the same superlayer. \n
       *  The superlayer pattern for Belle II is AUAVAUAVA according the TDR
       */
      EStereoKind getStereoKind() const { return first().getStereoKind(); }

      /// Getter for the inner radius of the layer as retrived from the CDCGeometryPar by the inner most layer
      double getInnerCylindricalR() const { return first().getInnerCylindricalR(); }

      /// Getter for the outer radius of the layer as retrived from the CDCGeometryPar by the outer most layer
      double getOuterCylindricalR() const { return last().getOuterCylindricalR(); }

      /// Getter for the radius in the middle of the layer.
      double getMiddleCylindricalR() const
      { return (getOuterCylindricalR() + getInnerCylindricalR()) / 2.0; }


      /// Getter for the (fitted) z of the reference wire reference points at the inner cylindricalR of this super layer.
      double getInnerRefZ() const { return m_innerRefZ; }

      /// Getter for the (fitted) z of the reference wire reference points at the outer cylindricalR of this super layer.
      double getOuterRefZ() const { return m_outerRefZ; }

      /// Getter for (fitted) proporitionality factor between the increasing cylindrical radius and the refernce z coordinate in this superlayer.
      double getRefTanLambda() const { return m_refTanLambda; }
      /**@}*/

      /** @name Closest neighborhood
       *
       *  Note : Not all neighbors might be present at the boundaries of the superlayer.
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
       *  EWireNeighborKind::c_None for the wires being no neighbors \n
       *  EWireNeighborKind::c_CWOut = 1  for clockwise outwards \n
       *  EWireNeighborKind::c_CW = 3 for clockwise \n
       *  EWireNeighborKind::c_CWIn = 5 for clockwise inwards \n
       *  EWireNeighborKind::c_CCWIn = 7 for counterclockwise inwards \n
       *  EWireNeighborKind::c_CCW = 9 for counterclockwise \n
       *  EWireNeighborKind::c_CCWOut = 11 for counterclockwise outwards \n
       *  The values are choosen to have an assoziation with the numbers on a regular clock.
       *  ( compare secondary neighborhood ) */
      EWireNeighborKind getNeighborKind(const ILayerType& iLayer,
                                        const IWireType& iWire,
                                        const ILayerType& iOtherLayer,
                                        const IWireType& iOtherWire) const;

      /// Getter for the two closest neighbors in the layer outwards of the given on
      NeighborPair getNeighborsOutwards(const ILayerType& iLayer, const IWireType& iWire) const;
      /// Getter for the two closest neighbors in the layer inwards of the given on
      NeighborPair getNeighborsInwards(const ILayerType& iLayer, const IWireType& iWire) const;

      /// Getter for the nearest clockwise neighbor in the next layer outwards by wire id and layer id with in this superlayer.
      const CDCWire* getNeighborCCWOutwards(const ILayerType& iLayer, const IWireType& iWire) const
      { return getNeighborsOutwards(iLayer, iWire).first; }  // not optimal since a second wire gets fetched additionally

      /// Getter for the nearest clockwise neighbor in the next layer outwards by wire id and layer id with in this superlayer.
      const CDCWire* getNeighborCWOutwards(const ILayerType& iLayer, const IWireType& iWire) const
      { return getNeighborsOutwards(iLayer, iWire).second; }  // not optimal since a second wire gets fetched additionally

      /// Getter for the nearest clockwise neighbor by wire id and layer id with in this superlayer.
      const CDCWire* getNeighborCW(const ILayerType& iLayer, const IWireType& iWire) const
      { return &(getWireLayer(iLayer).getNeighborCW(iWire)); }

      /// Getter for the nearest counterclockwise neighbor by wire id and layer id with in this superlayer.
      const CDCWire* getNeighborCCW(const ILayerType& iLayer, const IWireType& iWire) const
      { return &(getWireLayer(iLayer).getNeighborCCW(iWire)); }

      /// Getter for the nearest clockwise neighbor in the next layer outwards by wire id and layer id with in this superlayer.
      const CDCWire* getNeighborCCWInwards(const ILayerType& iLayer, const IWireType& iWire) const
      { return getNeighborsInwards(iLayer, iWire).first; }  // not optimal since a second wire gets fetched additionally

      /// Getter for the nearest clockwise neighbor in the next layer outwards by wire id and layer id with in this superlayer.
      const CDCWire* getNeighborCWInwards(const ILayerType& iLayer, const IWireType& iWire) const
      { return getNeighborsInwards(iLayer, iWire).second; }  // not optimal since a second wire gets fetched additionally
      /**@}*/

      /** @name Secondary neighborhood
       *  The secondary neighbors are numbered like positions on the normal twelve hour clock.
       *
       *  Due to the hexogonal arrangement of the wires, the secondary neighbors are positioned
       *  just like the numbers on the clock if you took the center of the clock to the wire position
       *  and the twelve o'clock poing in the outwards direction.
       *
       *  Note : Not all neighbors might be present at the boundaries of the superlayer.
       *  In case the neighbor asked for does not exist, the functions return nullptr instead.
       */
      /**@{*/

      ///Getter for secondary neighbor at the one o'clock position
      const CDCWire* getSecondNeighborOneOClock(const ILayerType& iLayer, const IWireType& iWire) const
      { return isValidILayer(iLayer + 2) ? &(getWireLayer(iLayer + 2).getWireSafe(iWire - 1)) : nullptr;  }


      ///Getter for secondary neighbor at the two o'clock position
      const CDCWire* getSecondNeighborTwoOClock(const ILayerType& iLayer, const IWireType& iWire) const
      { return getNeighborCWOutwards(iLayer, iWire - 1); }

      ///Getter for secondary neighbor at the three o'clock position
      const CDCWire* getSecondNeighborThreeOClock(const ILayerType& iLayer, const IWireType& iWire) const
      { return getNeighborCW(iLayer, iWire - 1); }

      ///Getter for secondary neighbor at the four o'clock position
      const CDCWire* getSecondNeighborFourOClock(const ILayerType& iLayer, const IWireType& iWire) const
      { return getNeighborCWInwards(iLayer, iWire - 1); }


      ///Getter for secondary neighbor at the five o'clock position
      const CDCWire* getSecondNeighborFiveOClock(const ILayerType& iLayer, const IWireType& iWire) const
      { return isValidILayer(iLayer - 2) ? &(getWireLayer(iLayer - 2).getWireSafe(iWire - 1)) : nullptr;  }

      ///Getter for secondary neighbor at the six o'clock position
      const CDCWire* getSecondNeighborSixOClock(const ILayerType& iLayer, const IWireType& iWire) const
      { return isValidILayer(iLayer - 2) ? &(getWireLayer(iLayer - 2).getWireSafe(iWire)) : nullptr;  }

      ///Getter for secondary neighbor at the seven o'clock position
      const CDCWire* getSecondNeighborSevenOClock(const ILayerType& iLayer, const IWireType& iWire) const
      { return isValidILayer(iLayer - 2) ? &(getWireLayer(iLayer - 2).getWireSafe(iWire + 1)) : nullptr;  }


      ///Getter for secondary neighbor at the eight o'clock position
      const CDCWire* getSecondNeighborEightOClock(const ILayerType& iLayer, const IWireType& iWire) const
      { return getNeighborCCWInwards(iLayer , iWire + 1); }

      ///Getter for secondary neighbor at the nine o'clock position
      const CDCWire* getSecondNeighborNineOClock(const ILayerType& iLayer, const IWireType& iWire) const
      { return getNeighborCCW(iLayer, iWire + 1); }

      ///Getter for secondary neighbor at the ten o'clock position
      const CDCWire* getSecondNeighborTenOClock(const ILayerType& iLayer, const IWireType& iWire) const
      { return getNeighborCCWOutwards(iLayer , iWire + 1); }


      ///Getter for secondary neighbor at the elven o'clock position
      const CDCWire* getSecondNeighborElevenOClock(const ILayerType& iLayer, const IWireType& iWire) const
      { return isValidILayer(iLayer + 2) ? &(getWireLayer(iLayer + 2).getWireSafe(iWire + 1)) : nullptr;  }

      ///Getter for secondary neighbor at the twelve o'clock position
      const CDCWire* getSecondNeighborTwelveOClock(const ILayerType& iLayer, const IWireType& iWire) const
      { return isValidILayer(iLayer + 2) ? &(getWireLayer(iLayer + 2).getWireSafe(iWire)) : nullptr;  }
      /**@}*/



    private:
      /// Begin of the layer range.
      /** Stores iterator pointing to the stored vector<CDCWireLayer> in the related CDCWireTopology object. */
      const_iterator m_begin;

      /// End of the layer range.
      /** Stores iterator pointing to the stored vector<CDCWireLayer> in the related CDCWireTopology object. */
      const_iterator m_end;

      /// Memory for the (fitted) z of the reference wire reference points at the inner cylindricalR of this super layer.
      double m_innerRefZ;

      /// Memory for the (fitted) z of the reference wire reference points at the outer cylindricalR of this super layer.
      double m_outerRefZ;

      /// Memory for (fitted) proporitionality factor between the increasing cylindrical radius and the refernce z coordinate in this superlayer.
      double m_refTanLambda;


    }; //class

  } // namespace TrackFindingCDC
} // namespace Belle2
