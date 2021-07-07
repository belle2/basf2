/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/topology/CDCWireLayer.h>
#include <tracking/trackFindingCDC/topology/WireNeighborPair.h>
#include <tracking/trackFindingCDC/topology/ISuperLayer.h>
#include <tracking/trackFindingCDC/topology/ILayer.h>
#include <tracking/trackFindingCDC/topology/IWire.h>
#include <tracking/trackFindingCDC/topology/WireNeighborKind.h>
#include <tracking/trackFindingCDC/topology/EStereoKind.h>
#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/trackFindingCDC/utilities/MayBePtr.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Class representating a sense wire superlayer in the central drift chamber.
     *  This class represents the a CDC superlayer as a range of wire layers. \n
     *  The range is sorted with increasing layer ids for maximal fast look up. \n
     *  It provides methods for checking the validity of layer and wire ids as well as \n
     *  a getter for the layers and wires in this layer. \n
     *  Additionally it is iterable as a range of CDCWireLayers. \n
     *  The superlayer also defines the closest and the secondary neighborhood. \n
     *  The class does not provide the memory for the wire layers by itself. \n
     *  It uses iterators into a vector of CDCWireLayers. \n
     *  The vector of CDCWireLayers is normally provided by the CDCWireTopology object instance. \n
     *  Note : All possible superlayer object are stored in the CDCWireTopology \n
     *  which you can get with the static getInstance() functions. \n
     *  There is rarely a need for constructing a superlayer object and \n
     *  it should be avoided.
     */
    class CDCWireSuperLayer
      : public ConstVectorRange<CDCWireLayer> {

    public:
      /// Type of the base class
      using Super = ConstVectorRange<CDCWireLayer>;

      /// Constructor taking the range of layers the superlayer shall contain. Use rather getInstance() to avoid instance constructions.
      explicit CDCWireSuperLayer(const ConstVectorRange<CDCWireLayer>& wireLayers);

      /// Disallow copy construction of wire super layers.
      CDCWireSuperLayer(const CDCWireSuperLayer& wireSuperLayer) = delete;

      /// Allow move construction of wire super layers for use in std::vector.
      CDCWireSuperLayer(CDCWireSuperLayer&& wireSuperLayer) = default;

      /// Disallow copy assignment of wire super layers.
      void operator=(const CDCWireSuperLayer& wireSuperLayer) = delete;

    public:
      /// Intializes the superlayer variables of according the layer range. Set the numbering shift of contained layers.
      void initialize();

      /**
       *  @name Superlayer index
       */
      /**@{*/

      /// Getter for the super layer id
      ISuperLayer getISuperLayer() const
      { return front().getISuperLayer(); }
      /**@}*/

      /**
       *  @name Layer getters
       */
      /**@{*/

      /// Checks if the given layer id belongs to a valid layer in this superlayer
      bool isValidILayer(ILayer iLayer) const
      { return 0 <= iLayer and iLayer < int(size()); }

      /// Gives the layer by its layer id within the superlayer
      const CDCWireLayer& getWireLayer(ILayer iLayer) const
      {
        // cppcheck-suppress returnTempReference
        return at(iLayer);
      }
      /**@}*/

      /**
       *  @name Wire getters
       */
      /**@{*/

      /// Checks if the given wire id belongs to a valid wire in this superlayer
      bool isValidIWire(ILayer iLayer, IWire iWire) const
      { return isValidILayer(iLayer) and getWireLayer(iLayer).isValidIWire(iWire); }

      /// Gives the wire by its layer id within the superlayer and the wire id in the layer.
      const CDCWire& getWire(ILayer iLayer, IWire iWire) const
      { return getWireLayer(iLayer).getWire(iWire); }
      /**@}*/


      /**
       *  @name Geometry properties
       */
      /**@{*/

      /// Indicates if the wire is axial or stereo
      bool isAxial() const
      { return getStereoKind() == EStereoKind::c_Axial; }

      /**
       *  Getter for the stereo type of the wire layer
       *  Gives the stereo type of the wire. \n
       *  Result is one of EStereoKind::c_Axial, EStereoKind::c_StereoU and EStereoKind::c_StereoV. \n
       *  The stereo type is shared by all wires in the same superlayer. \n
       *  The superlayer pattern for Belle II is AUAVAUAVA according the TDR
       */
      EStereoKind getStereoKind() const
      { return front().getStereoKind(); }

      /// Getter for the inner radius of the layer as retrived from the CDCGeometryPar by the inner most layer
      double getInnerCylindricalR() const
      { return front().getInnerCylindricalR(); }

      /// Getter for the outer radius of the layer as retrived from the CDCGeometryPar by the outer most layer
      double getOuterCylindricalR() const
      { return back().getOuterCylindricalR(); }

      /// Getter for the radius in the middle of the layer.
      double getMiddleCylindricalR() const
      { return (getOuterCylindricalR() + getInnerCylindricalR()) / 2.0; }


      /// Getter for the (fitted) z of the reference wire reference points at the inner cylindricalR of this super layer.
      double getInnerRefZ() const
      { return m_innerRefZ; }

      /// Getter for the (fitted) z of the reference wire reference points at the outer cylindricalR of this super layer.
      double getOuterRefZ() const
      { return m_outerRefZ; }

      /// Getter for (fitted) proporitionality factor between the increasing cylindrical radius and the refernce z coordinate in this superlayer.
      double getRefTanLambda() const
      { return m_refTanLambda; }
      /**@}*/

      /**
       *  @name Closest neighborhood
       *
       *  Note : Not all neighbors might be present at the boundaries of the superlayer.
       *  In case the neighbor asked for does not exist the function return nullptr instead.
       */
      /**@{*/

      /**
       *  Checks if two wires are closest neighbors. see details.
       *  Returns the relation of the first wire to the second wire give by their
       *  layer id within the superlayer and the wire id.
       */
      WireNeighborKind getNeighborKind(ILayer iLayer,
                                       IWire iWire,
                                       ILayer iOtherLayer,
                                       IWire iOtherWire) const;

      /**
       *  Getter for the two closest neighbors in the given o'clock direction.
       *  Valid values for the o clock direction are 1, 3, 5, 7, 9, 11.
       */
      MayBePtr<const CDCWire> getPrimaryNeighbor(short oClockDirection,
                                                 ILayer iLayer,
                                                 IWire iWire) const
      {
        oClockDirection = modulo(oClockDirection, 12);
        switch (oClockDirection) {
          case 1:
            return getNeighborCWOutwards(iLayer, iWire);
          case 3:
            return getNeighborCW(iLayer, iWire);
          case 5:
            return getNeighborCWInwards(iLayer, iWire);
          case 7:
            return getNeighborCCWInwards(iLayer, iWire);
          case 9:
            return getNeighborCCW(iLayer, iWire);
          case 11:
            return getNeighborCCWOutwards(iLayer, iWire);
          default:
            return nullptr;
        }
      }

      /// Getter for the two closest neighbors in the layer outwards of the given on
      WireNeighborPair getNeighborsOutwards(ILayer iLayer, IWire iWire) const;

      /// Getter for the two closest neighbors in the layer inwards of the given on
      WireNeighborPair getNeighborsInwards(ILayer iLayer, IWire iWire) const;

      /// Getter for the nearest clockwise neighbor in the next layer outwards by wire id and layer id with in this superlayer.
      MayBePtr<const CDCWire> getNeighborCCWOutwards(ILayer iLayer, IWire iWire) const
      { return getNeighborsOutwards(iLayer, iWire).first; }  // not optimal since a second wire gets fetched additionally

      /// Getter for the nearest clockwise neighbor in the next layer outwards by wire id and layer id with in this superlayer.
      MayBePtr<const CDCWire> getNeighborCWOutwards(ILayer iLayer, IWire iWire) const
      { return getNeighborsOutwards(iLayer, iWire).second; }  // not optimal since a second wire gets fetched additionally

      /// Getter for the nearest clockwise neighbor by wire id and layer id with in this superlayer.
      MayBePtr<const CDCWire> getNeighborCW(ILayer iLayer, IWire iWire) const
      { return &(getWireLayer(iLayer).getNeighborCW(iWire)); }

      /// Getter for the nearest counterclockwise neighbor by wire id and layer id with in this superlayer.
      MayBePtr<const CDCWire> getNeighborCCW(ILayer iLayer, IWire iWire) const
      { return &(getWireLayer(iLayer).getNeighborCCW(iWire)); }

      /// Getter for the nearest clockwise neighbor in the next layer outwards by wire id and layer id with in this superlayer.
      MayBePtr<const CDCWire> getNeighborCCWInwards(ILayer iLayer, IWire iWire) const
      { return getNeighborsInwards(iLayer, iWire).first; }  // not optimal since a second wire gets fetched additionally

      /// Getter for the nearest clockwise neighbor in the next layer outwards by wire id and layer id with in this superlayer.
      MayBePtr<const CDCWire> getNeighborCWInwards(ILayer iLayer, IWire iWire) const
      { return getNeighborsInwards(iLayer, iWire).second; }  // not optimal since a second wire gets fetched additionally
      /**@}*/

      /**
       *  @getSecondaryNeighbor Secondary neighborhood
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
      MayBePtr<const CDCWire> getSecondaryNeighbor(short oClockDirection,
                                                   ILayer iLayer,
                                                   IWire iWire) const
      {
        oClockDirection = modulo(oClockDirection, 12);
        switch (oClockDirection) {
          case 0:
            return getSecondNeighborTwelveOClock(iLayer, iWire);
          case 1:
            return getSecondNeighborOneOClock(iLayer, iWire);
          case 2:
            return getSecondNeighborTwoOClock(iLayer, iWire);
          case 3:
            return getSecondNeighborThreeOClock(iLayer, iWire);
          case 4:
            return getSecondNeighborFourOClock(iLayer, iWire);
          case 5:
            return getSecondNeighborFiveOClock(iLayer, iWire);
          case 6:
            return getSecondNeighborSixOClock(iLayer, iWire);
          case 7:
            return getSecondNeighborSevenOClock(iLayer, iWire);
          case 8:
            return getSecondNeighborEightOClock(iLayer, iWire);
          case 9:
            return getSecondNeighborNineOClock(iLayer, iWire);
          case 10:
            return getSecondNeighborTenOClock(iLayer, iWire);
          case 11:
            return getSecondNeighborElevenOClock(iLayer, iWire);
          default:
            return nullptr;
        }
      }

      /// Getter for secondary neighbor at the one o'clock position
      MayBePtr<const CDCWire> getSecondNeighborOneOClock(ILayer iLayer, IWire iWire) const
      { return isValidILayer(iLayer + 2) ? &(getWireLayer(iLayer + 2).getWireWrappedAround(iWire - 1)) : nullptr;  }


      /// Getter for secondary neighbor at the two o'clock position
      MayBePtr<const CDCWire> getSecondNeighborTwoOClock(ILayer iLayer, IWire iWire) const
      { return getNeighborCWOutwards(iLayer, iWire - 1); }

      /// Getter for secondary neighbor at the three o'clock position
      MayBePtr<const CDCWire> getSecondNeighborThreeOClock(ILayer iLayer, IWire iWire) const
      { return getNeighborCW(iLayer, iWire - 1); }

      /// Getter for secondary neighbor at the four o'clock position
      MayBePtr<const CDCWire> getSecondNeighborFourOClock(ILayer iLayer, IWire iWire) const
      { return getNeighborCWInwards(iLayer, iWire - 1); }


      /// Getter for secondary neighbor at the five o'clock position
      MayBePtr<const CDCWire> getSecondNeighborFiveOClock(ILayer iLayer, IWire iWire) const
      { return isValidILayer(iLayer - 2) ? &(getWireLayer(iLayer - 2).getWireWrappedAround(iWire - 1)) : nullptr;  }

      /// Getter for secondary neighbor at the six o'clock position
      MayBePtr<const CDCWire> getSecondNeighborSixOClock(ILayer iLayer, IWire iWire) const
      { return isValidILayer(iLayer - 2) ? &(getWireLayer(iLayer - 2).getWireWrappedAround(iWire)) : nullptr;  }

      /// Getter for secondary neighbor at the seven o'clock position
      MayBePtr<const CDCWire> getSecondNeighborSevenOClock(ILayer iLayer, IWire iWire) const
      { return isValidILayer(iLayer - 2) ? &(getWireLayer(iLayer - 2).getWireWrappedAround(iWire + 1)) : nullptr;  }


      /// Getter for secondary neighbor at the eight o'clock position
      MayBePtr<const CDCWire> getSecondNeighborEightOClock(ILayer iLayer, IWire iWire) const
      { return getNeighborCCWInwards(iLayer, iWire + 1); }

      /// Getter for secondary neighbor at the nine o'clock position
      MayBePtr<const CDCWire> getSecondNeighborNineOClock(ILayer iLayer, IWire iWire) const
      { return getNeighborCCW(iLayer, iWire + 1); }

      /// Getter for secondary neighbor at the ten o'clock position
      MayBePtr<const CDCWire> getSecondNeighborTenOClock(ILayer iLayer, IWire iWire) const
      { return getNeighborCCWOutwards(iLayer, iWire + 1); }


      /// Getter for secondary neighbor at the elven o'clock position
      MayBePtr<const CDCWire> getSecondNeighborElevenOClock(ILayer iLayer, IWire iWire) const
      { return isValidILayer(iLayer + 2) ? &(getWireLayer(iLayer + 2).getWireWrappedAround(iWire + 1)) : nullptr;  }

      /// Getter for secondary neighbor at the twelve o'clock position
      MayBePtr<const CDCWire> getSecondNeighborTwelveOClock(ILayer iLayer, IWire iWire) const
      { return isValidILayer(iLayer + 2) ? &(getWireLayer(iLayer + 2).getWireWrappedAround(iWire)) : nullptr;  }
      /**@}*/

    private:
      /// Memory for the (fitted) z of the reference wire reference points at the inner cylindricalR of this super layer.
      double m_innerRefZ;

      /// Memory for the (fitted) z of the reference wire reference points at the outer cylindricalR of this super layer.
      double m_outerRefZ;

      /// Memory for (fitted) proporitionality factor between the increasing cylindrical radius and the refernce z coordinate in this superlayer.
      double m_refTanLambda;

    };

  }
}
