/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCRIEMANNFITTER_H
#define CDCRIEMANNFITTER_H

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/eventdata/CDCEventData.h>

namespace Belle2 {
  namespace CDCLocalTracking {


    /// Class implementing the Riemann fit for two dimensional trajectory circle
    class CDCRiemannFitter : public CDCLocalTracking::UsedTObject {
    public:

      /** Constructor. */
      /** This is as well the parameter free I/O constructor.
       */

      CDCRiemannFitter();

      /** Destructor. */
      ~CDCRiemannFitter();

      CDCTrajectory2D fit(const CDCRecoHit2DVector& recohits) const {
        CDCTrajectory2D result;
        update(result, recohits);
        return result;
      }
      CDCTrajectory2D fit(const CDCSegmentTriple& recohits) const {
        CDCTrajectory2D result;
        update(result, recohits);
        return result;
      }

      void update(CDCTrajectory2D& fit, const CDCRecoHit2DVector& recohits) const {
        std::vector<FloatType> observations;
        bool useRightLeftInfo = fillObservations(recohits, observations) != 0;
        update(fit, observations, useRightLeftInfo);

        //set transverse s reference
        //fit.setStartPos2D( recohits.getCenterOfMass2D() ) ;
        fit.setStartPos2D(recohits.front().getRecoPos2D()) ;

        //check if fit is forward
        if (not recohits.isForwardTrajectory(fit)) fit.reverse();
        if (not recohits.isForwardTrajectory(fit)) B2WARNING("Fit cannot be oriented correctly");

      }

      void update(CDCTrajectory2D& fit,
                  const CDCRecoHit2DVector& firstRecohits,
                  const CDCRecoHit2DVector& secondRecohits) const {

        std::vector<FloatType> observations;
        bool useRightLeftInfo = (fillObservations(firstRecohits, observations)  +
                                 fillObservations(secondRecohits, observations)) != 0;

        update(fit, observations, useRightLeftInfo);

        //set transverse s reference
        //fit.setStartPos2D( recohits.getCenterOfMass2D() ) ;
        fit.setStartPos2D(firstRecohits.front().getRecoPos2D()) ;

        //check if fit is forward
        if (not firstRecohits.isForwardTrajectory(fit)) fit.reverse();

        if (not(firstRecohits.isForwardTrajectory(fit) and secondRecohits.isForwardTrajectory(fit)))
          B2WARNING("Fit cannot be oriented correctly");

      }

      void update(CDCTrajectory2D& fit, const CDCSegmentTriple& recohits) const {
        update(fit, *(recohits.getStart()), *(recohits.getEnd()));

        //std::vector<FloatType> observations;
        //bool useRightLeftInfo = fillObservations(recohits,observations) != 0;
        //update(fit,observations,useRightLeftInfo);
      }

      //size_t fillObservations(const CDCRecoHit2DSet& recohits, std::vector<FloatType>& observations) const;
      size_t fillObservations(const CDCRecoHit2DVector& recohits, std::vector<FloatType>& observations) const;
      //size_t fillObservations(const CDCSegmentTriple& segmentTriple, std::vector<FloatType>& observations) const __attribute__((deprecated));

      size_t fillObservation(const Belle2::CDCLocalTracking::CDCRecoHit2D& recohit, std::vector<FloatType>& observations) const;

      //void update(CDCTrajectory2D & fit,const CDCWireHitCollection & wirehits) const;
      //void update(CDCTrajectory2D & fit,const CDCRecoHitCollection & recohits) const;
      //void update(CDCTrajectory2D & fit,const CDCRecoSegment & recosegments) const;
      //void update(CDCTrajectory2D & fit, const CDCRecoSegment & fromRecoSegment, const CDCRecoSegment & toRecoSegment ) const;

    private:

      //template<class RecoHitInputIterator>
      //void updateGeneric(CDCTrajectory2D & fit,RecoHitInputIterator recohitsBegin , RecoHitInputIterator recohitsEnd,size_t nRecoHits) const;

    public:

      void update(CDCTrajectory2D& fit, std::vector<FloatType>& observations, bool usesRightLeftInfo) const;

      void updateWithOutRightLeft(CDCTrajectory2D& fit,
                                  FloatType* observations,
                                  size_t nObservations) const;
      void updateWithRightLeft(CDCTrajectory2D& fit,
                               FloatType* observations,
                               size_t nObservations) const;

      bool isLineConstrained() const { return m_lineConstrained; }
      bool isOriginConstrained() const { return m_originConstrained; }

      void setLineConstrained(bool constrained = true) { m_lineConstrained = constrained; }
      void setOriginConstrained(bool constrained = true) { m_originConstrained = constrained; }

      //set which information should be used from the recohits
      //usePositionAndOrientation is standard
      void useOnlyPosition() { m_usePosition = true; m_useOrientation = false;}
      void useOnlyOrientation() { m_usePosition = false; m_useOrientation = true;}
      void usePositionAndOrientation() { m_usePosition = true; m_useOrientation = true;}

      bool  m_usePosition;
      bool  m_useOrientation;

      bool m_lineConstrained;
      bool m_originConstrained;

      /** ROOT Macro to make CDCRiemannFitter a ROOT class.*/
      ClassDefInCDCLocalTracking(CDCRiemannFitter, 1);

    }; //class

  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCRIEMANNFITTER
