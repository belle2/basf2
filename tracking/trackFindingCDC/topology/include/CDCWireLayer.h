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

#include <tracking/trackFindingCDC/topology/CDCWire.h>
#include <tracking/trackFindingCDC/topology/ISuperLayer.h>
#include <tracking/trackFindingCDC/topology/ILayer.h>
#include <tracking/trackFindingCDC/topology/IWire.h>
#include <tracking/trackFindingCDC/topology/EStereoKind.h>

#include <tracking/trackFindingCDC/numerics/ERotation.h>
#include <tracking/trackFindingCDC/utilities/VectorRange.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Class representating a sense wire layer in the central drift chamber.
     *  This class represents a CDC layer as a range of wires. \n
     *  The range is sorted with increasing wire ids for maximal fast look up. \n
     *  It offers methods for checking the validity of wire ids, a getter for the wires in this layer \n
     *  additionally to be iterable as a range. \n
     *  Furthermore it stores averaged geometrical quantities combined from the wires it contains. \n
     *  The class does not provide the memory for the wires by itself. \n
     *  It uses iterators into a vector of CDCWires. \n
     *  The vector of CDCWires is normally provided by the CDCWireTopology object instance. \n
     *  Note : All possible layer object are stored in the CDCWireTopology, \n
     *  which you can get with the static getInstance() functions. \n
     *  There is rarely a need for constructing a layer object and \n
     *  it should be avoided.
     */
    class CDCWireLayer
      : public ConstVectorRange<CDCWire> {

    private:
      /// Type of the base class
      using Super = ConstVectorRange<CDCWire>;

    public:
      /**
       *  @name Static instance getters
       *  Getter for the already constructed instances from the CDCWireTopology::getInstance() object.
       */
      /**@{*/

      /// Getter from the the continuous layer id. Does not construct a new object.
      static const CDCWireLayer* getInstance(ILayer iCLayer);

      /// Getter from the superlayer id, the layer id. Does not construct a new object.
      static const CDCWireLayer* getInstance(ISuperLayer iSuperLayer, ILayer iLayer);
      /**@}*/

      /**
       *  Constructor taking the range of wires the layer shall contain.
       *  Use rather getInstance() to avoid instance constructions.
       */
      explicit CDCWireLayer(const ConstVectorRange<CDCWire>& wireRange);


      /// Disallow copy construction of wire layers.
      CDCWireLayer(const CDCWireLayer& wireLayer) = delete;

      /// Allow move construction of wire layers for use in std::vector.
      CDCWireLayer(CDCWireLayer&& wireLayer) = default;

      /// Disallow copy assignment of wire layers.
      void operator=(const CDCWireLayer& wireLayer) = delete;

    public:
      /// Intializes the wire layer variables to the average of according variables in the wire range
      void initialize();

      /**
       *  @name Layer index
       */
      /**@{*/

      /// Getter for the continuous layer id unique over all layers
      ILayer getICLayer() const
      { return front().getICLayer(); }

      /// Getter for the layer id unique within the superlayer
      ILayer getILayer() const
      { return front().getILayer(); }

      /// Getter for the super layer id
      ISuperLayer getISuperLayer() const
      { return front().getISuperLayer(); }
      /**@}*/


      /**
       *  @name Shift property
       */
      /**@{*/

      /**
       *  Getter for the numbering shift. See details.
       *  The shift indicates if the wire with index zero in this layer \n
       *  is more clockwise or counterclockwise than \n
       *  the zero wire in the zero layer within this superlayer. \n
       *  This can be used to speed up the lookup inward and outward neighbors \n
       *  of the wires in this layer. For even layers the shift should be ESign::c_Zero. \n
       */
      ERotation getShift() const
      { return m_shift; }

      /**
       *  Getter for the relative difference in wire numbering shift.
       *  In the current numbering scheme the shift can be of magnitude 1 in
       *  clockwise or counter clockwise direction or zero
       *  @param baseLayer  Wire layer that should be used as reference to the numbering shift
       *  @return           Shift in numbering from the given layer to this layer.
       */
      ERotation getShiftDelta(const CDCWireLayer& baseLayer) const
      { return static_cast<ERotation>(getShift() - baseLayer.getShift()); }
      /**@}*/

      /**
       *  @name Wire range
       */
      /**@{*/

      /// Checks if the given wire id belongs to a valid wire in this layer
      bool isValidIWire(IWire iWire) const
      { return 0 <= iWire and iWire < IWire(size()); }

      /// Gives the wire by its id in the layer.
      const CDCWire& getWire(IWire iWire) const
      {
        // cppcheck-suppress returnTempReference
        return at(iWire);
      }

      /// Gets the wire in this layer that is closest to the given position
      const CDCWire& getClosestWire(const Vector3D& pos3D) const;
      /**@}*/


      /**
       *  @name Neighborhood of wires in this layer
       */
      /**@{*/

      /**
       *  Returns the wire by its id in the layer.
       *  Gives the wire by its id, but transforms it to valid range first, \n
       *  so even if the id is out of range you will get a valid wire object. \n
       *  This makes the retrival of neighbors much easier if the neighborhood \n
       *  is around the discontinuity near the zero wires.
       */
      const CDCWire& getWireWrappedAround(IWire iWire) const
      { return getWire(IWireUtil::wrappedAround(iWire, size())); }

      /// Getter for the first counterclockwise neighbor by wire id in the layer.
      const CDCWire& getNeighborCCW(IWire iWire) const
      { return getWireWrappedAround(iWire + 1); }

      /// Getter for the first clockwise neighbor by wire id in the layer.
      const CDCWire& getNeighborCW(IWire iWire) const
      { return getWireWrappedAround(iWire - 1); }

      /// Getter for the first counterclockwise neighbor by wire id in the layer.
      const CDCWire& getSecondNeighborCCW(IWire iWire) const
      { return getWireWrappedAround(iWire + 2); }

      /// Getter for the first clockwise neighbor by wire id in the layer.
      const CDCWire& getSecondNeighborCW(IWire iWire) const
      { return getWireWrappedAround(iWire - 2); }
      /**@}*/

      /**
       *  @name Averaged geometry properties
       *  The following getters return variables calculated as average from the contained CDCWire.
       */
      /**@{*/

      /// Indicates if the wire is axial or stereo
      bool isAxial() const
      { return front().isAxial(); }

      /**
       *  Getter for the stereo type of the wire layer
       *  Gives the stereo type of the wire. \n
       *  Result is one of EStereoKind::c_Axial, EStereoKind::c_StereoU and EStereoKind::c_StereoV. \n
       *  The stereo type is shared by all wires in the same superlayer. \n
       *  The superlayer pattern for Belle II is AUAVAUAVA according the TDR
       */
      EStereoKind getStereoKind() const
      { return front().getStereoKind(); }

      /// Getter for the averaged tan stereo angle of all wires in this layer
      double getTanStereoAngle() const
      { return m_tanStereoAngle; }

      /// Getter for the average stereo angle of all wires in this layer
      double getStereoAngle() const
      { return std::atan(getTanStereoAngle()); }

      /// Getter for the closest distance to the beamline ( z-axes ) of all wires in this layer
      double getMinCylindricalR() const
      { return m_minCylindricalR; }

      /// Getter for the  common (averaged) cylindrical radius at the wire reference point
      double getRefCylindricalR() const
      { return m_refCylindricalR; }

      /// Getter for the common (averaged) z component of the wire reference point
      double getRefZ() const
      { return m_refZ; }

      /// Getter for the average distance to the beamline ( z-axes ) at the forward joint points of all wires in this layer
      double getForwardCylindricalR() const
      { return m_forwardCylindricalR; };

      /// Getter for the average distance to the beamline ( z-axes ) at the backward joint points of all wires in this layer
      double getBackwardCylindricalR() const
      { return m_backwardCylindricalR; };

      /// Getter for the average z coordinate at the forward joint points of all wires in this layer
      double getForwardZ() const
      { return m_forwardZ; }

      /// Getter for the average z coordinate at the backward joint points of all wires in this layer
      double getBackwardZ() const
      { return m_backwardZ; }

      /**@}*/

      /**
       *  @name Geometry properties copied from the CDCGeometryPar
       */
      /**@{*/

      /// Getter for inner radius of the layer as taken from the CDCGeometryPar
      double getInnerCylindricalR() const
      { return m_innerCylindricalR; }

      /// Getter for outer radius of the layer as taken from the CDCGeometryPar
      double getOuterCylindricalR() const
      { return m_outerCylindricalR; }

      /// Getter for the width of the wire layer in the radial direction
      double getRadialCellWidth() const
      { return getOuterCylindricalR() - getInnerCylindricalR(); }

      /**
       *  Getter for the width of the wire layer in the lateral direction.
       *
       *  Lateral means the perpendicular to the radial outwards direction
       *  The measure is taken at the outer bound, where it is largest for the wire layer
       */
      double getLateralCellWidth() const
      { return getOuterCylindricalR() * 2 * M_PI / size(); }
      /**@}*/

    private:
      /**
       *  Indicates if this layer is shifted clockwise or counterclockwise. \n
       *  The shift indicates if the wire with index zero in this layer \n
       *  is more clockwise or counterclockwise than  \n
       *  the zero wire in the zero layer within this superlayer. \n
       *  This can be used to speed up the lookup inward and outward \n
       *  neighbors of the wires in this layer. For even layers the shift should be 0.
       */
      ERotation m_shift = ERotation::c_Invalid;

      /**
       *  @name Property averages from the contained CDCWires
       *  The following variables are calculated from the contained CDCWire
       *  on initialization of the CDCWireLayer.
       */
      //@{

      /// Storage for average tan stereo angle
      double m_tanStereoAngle = 0.0;

      /// Storage for minimal distance from beamline
      double m_minCylindricalR = 0.0;

      /// Storage for common (averaged) cylindrical radius of all wire reference positions.
      double m_refCylindricalR = 0.0;

      /// Storage for common (averaged) z coordinate of all wire reference positions.
      double m_refZ = 0.0;

      /// Storage for average distance from beamline of forward wire ends
      double m_forwardCylindricalR = 0.0;

      /// Storage for average distance from beamline of backward wire ends
      double m_backwardCylindricalR = 0.0;

      /// Storage for z position of forward wire ends
      double m_forwardZ = 0.0;

      /// Storage for z position of backward wire ends
      double m_backwardZ = 0.0;
      //@}

      /**
       *  @name Property averages CDCGeometryPar
       *  The following variables are taken directly from the CDCGeometryPar
       *  on initialization of the CDCWireLayer.
       */
      //@{

      /// Storage of the inner radius of the wire layer as taken from the CDCGeometryPar instance
      double m_innerCylindricalR = 0.0;

      /// Storage of the outer radius of the wire layer as taken from the CDCGeometryPar instance
      double m_outerCylindricalR = 0.0;
      //@}


    };

  }
}
