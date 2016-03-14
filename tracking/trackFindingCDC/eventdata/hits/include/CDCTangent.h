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

#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHitPair.h>

#include <tracking/trackFindingCDC/geometry/ParameterLine2D.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class representating a linear track piece between two oriented wire hits.
    /** A tangent is an approximation of the possible trajectory between two oriented wire hits.
     *  The approximation is obtained by constructing a tangent between two drift circles of the wire hits
     *  (in the reference xy projection).\n
     *  Generally maximal four tangents are possible. So to uniquely define a tangent we have
     *  to give additional information how it passes relativ to the drift circles. This right left
     *  passage inforamtion indicates if the related wire hit should lie to the right of to the left
     *  of the tangent. The four possible combinations are then\n
     *  (ERightLeft::c_Right, ERightLeft::c_Right), (ERightLeft::c_Right, ERightLeft::c_Left),\n
     *  (ERightLeft::c_Left, ERightLeft::c_Right), (ERightLeft::c_Left,ERightLeft::c_Left).\n
     *  To represent the tangent this class uses a ParameterLine2D. The touch points to the two drift circles
     *  are located at(0) for the first and at(1) for the second.
     *  The tangent has therefor a sense of what is forward and can be reversed if necessary.
     *  Generally tangents are only a good approximation between neighboring wire hits.*/
    class CDCTangent : public CDCRLWireHitPair {
    public:
      /// Construct a tangent from a pair of oriented wire hits.
      explicit CDCTangent(const CDCRLWireHitPair& rlWireHitPair);

      /// Construct a tangent from two oriented wire hits.
      CDCTangent(const CDCRLTaggedWireHit& fromRLWireHit,
                 const CDCRLTaggedWireHit& toRLWireHit);

      /// Construct a tangent from a pair of oriented wire hits taking the given
      /// tangential line instead of a computed one.
      CDCTangent(const CDCRLWireHitPair& rlWireHitPair,
                 const ParameterLine2D& line);

      /// Construct a tangent from two oriented wire hits taking the given tangential line instead of a computed one.
      CDCTangent(const CDCRLTaggedWireHit& fromRLWireHit,
                 const CDCRLTaggedWireHit& toRLWireHit,
                 const ParameterLine2D& line);

      /// Print tangent for debuggin.
      friend std::ostream& operator<<(std::ostream& output, const CDCTangent& tangent)
      {
        output << "Tangent" << std::endl;
        output << "From : " << tangent.getFromWireHit()->getWire() << " " <<  tangent.getFromRecoDisp2D() << std::endl;
        output << "To : " << tangent.getToWireHit()->getWire() << " " <<  tangent.getToRecoDisp2D()  << std::endl;
        return output;
      }

      /// Access the object methods and methods from a pointer in the same way.
      /** In situations where the type is not known to be a pointer or a reference there is no way to tell
       *  if one should use the dot '.' or operator '->' for method look up.
       *  So this function defines the -> operator for the object.
       *  No matter you have a pointer or an object access is given with '->'.*/
      const CDCTangent* operator->() const
      { return this; }

      /// Allow automatic taking of the address.
      /** Essentially pointers to objects is a class of the object itself.
       *  This method activally exposes this inheritance to be able to write algorithms
       *  that work for objects and poiinters alike without code duplication. */
      operator const Belle2::TrackFindingCDC::CDCTangent* () const
      { return this; }

      /// Getter for the touching point of the tangent to the first drift circle.
      const Vector2D& getFromRecoPos2D() const
      { return getLine().support(); }

      /// Getter for displacement of the touching point from the first wire in the reference plane.
      Vector2D getFromRecoDisp2D() const
      { return getFromRecoPos2D() - getFromWireHit()->getRefPos2D(); }

      /// Getter for the touching point of the tangent to the second drift circle.
      Vector2D getToRecoPos2D() const
      { return getLine().at(1); }

      /// Getter for displacement of the touching point from the second wire in the reference plane.
      Vector2D getToRecoDisp2D() const
      { return getToRecoPos2D() - getToWireHit()->getRefPos2D(); }

      /// Getter for the vector from the first to the second touch point.*/
      const Vector2D& getFlightVec2D() const
      { return getLine().tangential(); }

      /// Returns the cosine of the angle between the two flight directions of the tangents.
      double getCosFlightDifference(const CDCTangent& tangent) const
      { return getFlightVec2D().cosWith(tangent.getFlightVec2D()); }

      /// Getter for the reconstructed hit on the first oriented wire hit using reconstructed touch point as position.
      CDCRecoHit2D getFromRecoHit2D() const
      { return CDCRecoHit2D::fromRecoPos2D(getFromRLWireHit(), getFromRecoPos2D()); }

      /// Getter for the reconstructed hit on the second oriented wire hit using reconstructed touch point as position.
      CDCRecoHit2D getToRecoHit2D() const
      { return CDCRecoHit2D::fromRecoPos2D(getToRLWireHit(), getToRecoPos2D()); }

      /// Adjusts the line to touch the drift circles with the correct right left passage information.
      void adjustLine();

      /// Adjusts the right left passage information according to the tangent line.
      void adjustRLInfo();

      /** Reverses the tangent inplace.
       *  Swaps the to wire hits, reverses the left right informations and
       *  exchanges the two touch points of the line
       */
      void reverse();

      /// Same as reverse but returns a copy.
      CDCTangent reversed() const;

      /** Getter for the line representation of the line.
       *  The line stretchs between the two the touch point.
       *  The first touch point is at(0) the second at(1).
       */
      const ParameterLine2D& getLine() const
      { return m_line; }

    private:
      /// Memory for the line between the two touching points. The first touch point at(0), second at(1).
      ParameterLine2D m_line;


    }; //class

  } // namespace TrackFindingCDC
} // namespace Belle2
