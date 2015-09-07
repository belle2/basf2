/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHitPair.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** Class representing a triple of neighboring wire hits.
     *  The first is taken as a single, while the second and the third are group into a pair.
     *  In this way we can save some object creations during lookups of a triple following this one,
     *  since the projection to the last pair can be done by reference.*/
    class CDCRLWireHitTriple {

    public:
      /** Type for the different shapes of three neighboring hits
       *  (up to mirror and rotational symmetries)
       *  Note: The names are inspired by the xylene molecules. */
      typedef signed int Shape;

      /// Constant for an ill shaped triple
      static const Shape ILLSHAPED = -999;

      /// Constant for ortho arrangement with clockwise twist
      static const Shape ORTHO_CW = -4;

      /// Constant for meta arrangement with clockwise twist
      static const Shape META_CW = -2;

      /// Constant for para arrangement, no twist here
      static const Shape PARA = 0;

      /// Constant for meta arrangement with counterclockwise twist
      static const Shape META_CCW = 2;

      /// Constant for ortho arrangement with counterclockwise twist
      static const Shape ORTHO_CCW = 4;

    public:

      /// Default constructor for ROOT compatibility.
      CDCRLWireHitTriple();

      /// Constructor taking three oriented wire hits
      CDCRLWireHitTriple(const CDCRLWireHit* startRLWireHit,
                         const CDCRLWireHit* middleRLWireHit,
                         const CDCRLWireHit* endRLWireHit);

    public:

      /// Constructs the reverse tiple from this one
      CDCRLWireHitTriple reversed() const;

      /// Reverses the triple inplace.
      void reverse();


    public:

      /// Equality comparision considering all three oriented wire hits
      bool operator==(const CDCRLWireHitTriple& other) const
      { return getStartRLWireHit() == other.getStartRLWireHit() and getRearRLWireHitPair() == other.getRearRLWireHitPair(); }


      /// Establish a total ordering  based on the three oriented wire hits
      bool operator< (const CDCRLWireHitTriple& other) const
      {
        return getStartRLWireHit() <  other.getStartRLWireHit() or
               (getStartRLWireHit() == other.getStartRLWireHit() and getRearRLWireHitPair() < other.getRearRLWireHitPair());
      }


      /// Define oriented wire hit pairs to be coaligned with orient wire hit triples on the first two oriented wire hits
      friend bool operator< (const CDCRLWireHitTriple& rlWireHitTriple,
                             const CDCRLWireHitPair& rlWireHitPair)
      {
        return rlWireHitTriple.getStartRLWireHit() < rlWireHitPair.getFromRLWireHit() or
               (rlWireHitTriple.getStartRLWireHit() == rlWireHitPair.getFromRLWireHit() and
                rlWireHitTriple.getMiddleRLWireHit() < rlWireHitPair.getToRLWireHit());
      }

      /// Define oriented wire hit pairs to be coaligned with orient wire hit triples on the first two oriented wire hits
      friend bool operator< (const CDCRLWireHitPair& rlWireHitPair,
                             const CDCRLWireHitTriple& rlWireHitTriple)
      {
        return rlWireHitPair.getFromRLWireHit() < rlWireHitTriple.getStartRLWireHit() or
               (rlWireHitPair.getFromRLWireHit() == rlWireHitTriple.getStartRLWireHit() and
                rlWireHitPair.getToRLWireHit() < rlWireHitTriple.getMiddleRLWireHit());
      }


      /// Standard output operator for debugging purposes
      friend std::ostream& operator<<(std::ostream& output, const CDCRLWireHitTriple& rlWireHitTriple)
      {
        return output << "Start : "  << rlWireHitTriple.getStartWireHit() << " with rl " << rlWireHitTriple.getStartRLInfo() <<
               " Middle : " << rlWireHitTriple.getMiddleWireHit() << " with rl " << rlWireHitTriple.getMiddleRLInfo() <<
               " End : "    << rlWireHitTriple.getEndWireHit()  << " with rl " << rlWireHitTriple.getEndRLInfo() ;
      }




      /// Access the object methods and methods from a pointer in the same way.
      /** In situations where the type is not known to be a pointer or a reference there is no way to tell \n
       *  if one should use the dot '.' or operator '->' for method look up. \n
       *  So this function defines the -> operator for the object. \n
       *  No matter you have a pointer or an object access is given with '->'*/
      const CDCRLWireHitTriple* operator->() const
      { return this; }


      /// Getter for the shape of this tiple if all three oriented wire hits are neighbors. Else ILLSHAPE
      Shape getShape() const;

      /// Getter for the common superlayer id of the pair
      ILayerType getISuperLayer() const
      {
        ILayerType result = getStartRLWireHit().getISuperLayer();
        return result == getRearRLWireHitPair().getISuperLayer() ? result : INVALID_ISUPERLAYER;
      }

      /// Getter for the common stereo type of the superlayer of the pair
      StereoType getStereoType() const
      { return isValidISuperLayer(getISuperLayer()) ? getStartRLWireHit().getStereoType() : StereoType::c_Invalid; }

      /// Indicator if the underlying wires are axial.
      bool isAxial() const
      { return getStereoType() == StereoType::c_Axial; }

      /// Getter for the wire the first oriented wire hit is based on
      const CDCWire& getStartWire() const
      { return getStartRLWireHit().getWire(); }

      /// Getter for the wire the second oriented wire hit is based on
      const CDCWire& getMiddleWire() const
      { return getMiddleRLWireHit().getWire(); }

      /// Getter for the wire the third oriented wire hit is based on
      const CDCWire& getEndWire() const
      { return getEndRLWireHit().getWire(); }

      /// Indicator if any of the three oriented wire hits is based on the given wire
      bool hasWire(const CDCWire& wire) const
      { return getStartRLWireHit()->isOnWire(wire) or getRearRLWireHitPair()->hasWire(wire); }



      /// Getter for the right left passage information of the first oriented wire hit
      RightLeftInfo getStartRLInfo() const
      { return getStartRLWireHit().getRLInfo(); }

      /// Getter for the right left passage information of the second oriented wire hit
      RightLeftInfo getMiddleRLInfo() const
      { return getMiddleRLWireHit().getRLInfo(); }

      /// Getter for the right left passage information of the third oriented wire hit
      RightLeftInfo getEndRLInfo() const
      { return getEndRLWireHit().getRLInfo(); }



      /// Getter for the hit wire of the first oriented wire hit
      const CDCWireHit& getStartWireHit() const
      { return getStartRLWireHit().getWireHit(); }

      /// Getter for the hit wire of the second oriented wire hit
      const CDCWireHit& getMiddleWireHit() const
      { return getMiddleRLWireHit().getWireHit(); }

      /// Getter for the hit wire of the third oriented wire hit
      const CDCWireHit& getEndWireHit() const
      { return getEndRLWireHit().getWireHit(); }

      /// Indicator if any of the three oriented wire hits is based on the given wire hit
      bool hasWireHit(const CDCWireHit& wirehit) const
      { return getStartWireHit() == wirehit or getRearRLWireHitPair()->hasWireHit(wirehit); }



      /// Getter for the first oriented wire hit
      const CDCRLWireHit& getStartRLWireHit() const
      { return *m_startRLWireHit; }

      /// Getter for the second oriented wire hit
      const CDCRLWireHit& getMiddleRLWireHit() const
      { return getRearRLWireHitPair().getFromRLWireHit(); }

      /// Getter for the third oriented wire hit
      const CDCRLWireHit& getEndRLWireHit() const
      { return getRearRLWireHitPair().getToRLWireHit(); }

      /// Constant getter for the pair of second and third oriented wire hit
      const CDCRLWireHitPair& getRearRLWireHitPair() const
      { return m_rearRLWireHitPair; }

      /// Getter for the pair of second and third oriented wire hit
      CDCRLWireHitPair& getRearRLWireHitPair()
      { return m_rearRLWireHitPair; }



      /// Setter for the first oriented wire hit
      void setStartRLWireHit(const CDCRLWireHit* startRLWireHit)
      { m_startRLWireHit = startRLWireHit; }

      /// Setter for the second oriented wire hit
      void setMiddleRLWireHit(const CDCRLWireHit* middleRLWireHit)
      { m_rearRLWireHitPair.setFromRLWireHit(middleRLWireHit); }

      /// Setter for the third oriented wire hit
      void setEndRLWireHit(const CDCRLWireHit* endRLWireHit)
      { m_rearRLWireHitPair.setToRLWireHit(endRLWireHit); }



    public:
      /// Setter for the right left passage information of the first oriented wire hit
      void setStartRLInfo(const RightLeftInfo startRLInfo);

      /// Setter for the right left passage information of the second oriented wire hit
      void setMiddleRLInfo(const RightLeftInfo middleRLInfo);

      /// Setter for the right left passage information of the third oriented wire hit
      void setEndRLInfo(const RightLeftInfo endRLInfo);



    protected:
      /// Memory for the reference to the start oriented wire hit.
      const CDCRLWireHit* m_startRLWireHit;

      /// Memory for the reference to the second and third wire hits.
      CDCRLWireHitPair m_rearRLWireHitPair;

    }; //end class CDCRLWireHitTriple
  } // end namespace TrackFindingCDC
} // end namespace Belle2
