/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef BOUNDSKEWLINE_H
#define BOUNDSKEWLINE_H


#include <tracking/cdcLocalTracking/rootification/SwitchableRootificationBase.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/numerics/numerics.h>

#include <cmath>

#include <framework/logging/Logger.h>

#include "Vector2D.h"
#include "Vector3D.h"

namespace Belle2 {
  namespace CDCLocalTracking {
    /**
     * A three dimensional limited line represented by its closest approach to the z-axes (reference position ) and its skew parameter.
     * The representation is only suitable for lines out of the xy plane.
     * The endpoints are stored encoded by their z position.
     * The end point with the higher z value is called forward, the one with the smaller is called backward.
     * This class is mainly used to descripe the cdc wires with as few parameters as possible and
     * simplifies the retrival of the two dimensional track reference position, which is taken at
     * the closest approach to the beam z-axes.
     * @ingroup cdclocaltracking
     * @brief A three dimensional limited line
     *
     */
    class BoundSkewLine : public CDCLocalTracking::SwitchableRootificationBase {

    public:

      /// Default constructor for ROOT compatibility.
      BoundSkewLine() : m_forwardZ(0.0) , m_backwardZ(0.0), m_skew(0.0), m_referencePosition() {;}

      /// Constuctor for a skew line between forward and backward point
      BoundSkewLine(const Vector3D& forwardIn , const Vector3D& backwardIn);

      /// Empty deconstructor
      ~BoundSkewLine() {;}

      /// Returns a copy of the skew line moved by a three dimensional offset
      inline BoundSkewLine movedBy(const Vector3D& offset) const {
        //simple but not optional in computation steps
        return BoundSkewLine(forward3D().add(offset), backward3D().add(offset));
      }

      /// Returns a copy of the skew line moved by a two dimensional offset
      inline BoundSkewLine movedBy(const Vector2D& offset) const {
        //simple but not optional in computation steps
        return BoundSkewLine(forward3D().add(offset), backward3D().add(offset));
      }

      /// Gives the three dimensional position of the line at the given z value
      inline Vector3D pos3DAtZ(const FloatType& z) const {

        FloatType deltaZTimesSkew = (z - refZ()) * skew();

        return Vector3D(1 * refX() - deltaZTimesSkew * refY() ,
                        deltaZTimesSkew * refX() + 1 * refY() ,
                        z);

      }

      /// Gives the two dimensional position of the line at the given z value
      inline Vector2D pos2DAtZ(const FloatType& z) const {

        FloatType deltaZTimesSkew = (z - refZ()) * skew();

        return Vector2D(1 * refX() - deltaZTimesSkew * refY() ,
                        deltaZTimesSkew * refX() + 1 * refY());

      }
      /// Gives the three dimensional position of the line at the given ( z - reference z ) * skew value
      inline Vector3D pos3DAtDeltaZTimesSkew(const FloatType& deltaZTimesSkew) const {

        return Vector3D(1 * refX() - deltaZTimesSkew * refY() ,
                        deltaZTimesSkew * refX() + 1 * refY() ,
                        deltaZTimesSkew / skew() + refZ());

      }

      /// Gives the position of the forward point
      inline Vector3D forward3D() const
      { return pos3DAtZ(forwardZ()) ; }

      /// Gives the xy position of the forward point
      inline Vector2D forward2D() const
      { return pos2DAtZ(forwardZ()) ; }

      /// Gives the position of the backward point
      inline Vector3D backward3D() const
      { return pos3DAtZ(backwardZ()) ; }

      /// Gives the xy position of the backward point
      inline Vector2D backward2D() const
      { return pos2DAtZ(backwardZ()) ; }

      /// Gives the position of the point half way between forward and backward
      inline Vector3D center3D() const
      { return pos3DAtZ((forwardZ() + backwardZ()) / 2) ; }

      /// Gives the xy position of the point half way between forward and backward
      inline Vector2D center2D() const
      { return pos2DAtZ((forwardZ() + backwardZ()) / 2) ; }

      /// Gives the tangential vector to the line
      inline Vector3D tangential3D() const {
        FloatType deltaZ = forwardZ() - backwardZ();
        return Vector3D(- deltaZ * skew() * refY(),
                        deltaZ * skew() * refX(),
                        deltaZ);
      }

      /// Gives the tangential xy vector to the line
      inline Vector2D tangential2D() const {
        FloatType deltaZ = forwardZ() - backwardZ();
        return Vector2D(- deltaZ * skew() * refY(),
                        deltaZ * skew() * refX());
      }

      /// Gives the forward z coodinate
      inline FloatType forwardZ() const
      { return m_forwardZ; }

      /// Gives the backward z coodinate
      inline FloatType backwardZ() const
      { return m_backwardZ; }

      /// Gives the forward polar angle
      inline FloatType forwardPhi() const
      { return  forward2D().phi(); }

      /// Gives the backward polar angle
      inline FloatType backwardPhi() const
      { return backward2D().phi(); }

      /// Gives the polar radius of the forward position
      inline FloatType forwardPolarR() const
      { return  forward2D().polarR(); }

      /// Gives the polar radius of the backward position
      inline FloatType backwardPolarR() const
      { return backward2D().polarR(); }

      /// Gives the polar angle of the forward position relative to the reference position
      inline FloatType forwardPhiToRef() const
      { return  forward2D().angleWith(refPos2D()); }

      /// Gives the polar angle of the backward position relative to the reference position
      inline FloatType backwardPhiToRef() const
      { return backward2D().angleWith(refPos2D()); }

      /// Gives the polar angle difference from backward to forward position
      /* backwardToForwardAngle means how far the backward position has to be rotated in the xy projection
         in the mathematical positiv sense that it seems to be coaligned with the forward position. */
      inline FloatType backwardToForwardAngle() const
      { return backward2D().angleWith(forward2D()) ; }

      /// Returns the closest approach to the give point
      /** This calculates the point of closest approach on the line. It does not care about the boundaries of the line. */
      inline Vector3D closest3D(const Vector3D& point) const {
        Vector3D unitLineVector = tangential3D();
        unitLineVector.normalize();

        FloatType parallelCompPoint = point.unnormalizedParallelComp(unitLineVector);
        FloatType parallelCompRef = refPos3D().unnormalizedParallelComp(unitLineVector);

        //same memory , only different name
        Vector3D& parallelVec = unitLineVector.scale(parallelCompPoint - parallelCompRef);

        return refPos3D() - parallelVec;
      }

      /// Returns the point of closest approach to the origin on the line
      inline Vector3D closestToOrigin3D() const {
        Vector3D unitLineVector = tangential3D();
        unitLineVector.normalize();

        FloatType parallelCompRef = refPos3D().unnormalizedParallelComp(unitLineVector);

        //same memory , only different name
        Vector3D& parallelVecOfRef = unitLineVector.scale(parallelCompRef);

        return refPos3D() - parallelVecOfRef;
      }
      /// Return the skew parameter
      inline const FloatType& skew() const
      { return m_skew; }

      /// Returns the the x coordinate of the reference point.
      inline const FloatType& refX() const
      { return m_referencePosition.x(); }

      /// Returns the the y coordinate of the reference point.
      inline const FloatType& refY() const
      { return m_referencePosition.y(); }

      /// Returns the the z coordinate of the reference point.
      inline const FloatType& refZ() const
      { return m_referencePosition.z(); }

      /// Returns the polar radius of the reference position
      inline FloatType refPolarRSquared() const
      { return m_referencePosition.polarRSquared(); }

      /// Returns the xy vector of the reference position
      inline const Vector2D& refPos2D() const
      { return m_referencePosition.xy(); }

      /// Returns the reference position
      inline const Vector3D& refPos3D() const
      { return m_referencePosition; }

    private:

      FloatType m_forwardZ; ///< Memory for the forward end z coordinate.
      FloatType m_backwardZ; ///< Memory for the backward end z coordinate.

      FloatType m_skew; ///< Memory for the skew parameter
      Vector3D m_referencePosition; ///< Memory for the reference postion

    private:

      /// ROOT Macro to make BoundSkewLine a ROOT class.
      CDCLOCALTRACKING_SwitchableClassDef(BoundSkewLine, 1);


    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // BOUNDSKEWLINE_H
