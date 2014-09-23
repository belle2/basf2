/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCTRAJECTORYSZ_H
#define CDCTRAJECTORYSZ_H

#include <TVector3.h>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/geometry/Vector2D.h>
#include <tracking/cdcLocalTracking/geometry/Vector3D.h>
#include <tracking/cdcLocalTracking/geometry/UncertainSZLine.h>

namespace Belle2 {
  namespace CDCLocalTracking {


    /// Linear trajectory in sz space
    /** Particle trajectories have a linear dependence between the travel distance s and \n
     *  the z coordinate as the magnetic fields along z does not disturb the free z movement. \n
     *  This class presents this trajectory projection by a line representation in sz space.
     *  The s coordinate is interpreted as the transvers travel distance, hence  \n
     *  the part of the travel distance see in the xy projection. */
    class CDCTrajectorySZ : public CDCLocalTracking::UsedTObject {

    public:

      /// Default constructor for ROOT compatibility.
      CDCTrajectorySZ() {;}


      /// Constructs the trajectory from the given two dimensional sz line.
      CDCTrajectorySZ(const UncertainSZLine& szLine) :
        m_szLine(szLine)
      {;}


      /// Constructs the trajectory from the given two dimensional sz line.
      CDCTrajectorySZ(const Line2D& szLine) :
        m_szLine(szLine)
      {;}


      /// Constructs the trajectory with the given slope and start z.
      CDCTrajectorySZ(const FloatType& szSlope,
                      const FloatType& startZ) :
        m_szLine(Line2D::fromSlopeIntercept(szSlope, startZ))
      {;}

      /// Construct the sz line by its normal parameters ( used in least square fits )
      CDCTrajectorySZ(const FloatType& n0,
                      const FloatType& n1,
                      const FloatType& n2) :
        m_szLine(Line2D(n0, n1, n2))
      { if (not getSZLine().alignedWithFirst()) m_szLine.reverse();}   // make the orientation to be forward with s ( important for the right sign of the distances)

      /// Empty destructor
      ~CDCTrajectorySZ() {;}

      /// Translates the travel distance to the z coordinate
      inline FloatType mapSToZ(const FloatType& s = 0) const
      { return getSZLine().map(s); }

      /// Translates the absolute value of the transvers momentum to the momentum in z direction.
      /** Scales the absolute value of  the transvers momentum to the z component of the momentum
       *  vector. The z component comes with the correct sign as reflected by increasing or
      *   decreasing z component. Note : This assumes s to be the transvers travel distance.*/
      inline FloatType mapPtToPz(const FloatType& pt) const
      { return getSZLine().slope() * pt; }

      /// Gets the vertical distance of the line
      /** Gives the signed distance in the z direction from the point to the line. \n
       *  The plus sign mean the point is below the line. For minus it is above.
       *  The later is naturally choosen as the direction of positiv advance is \n
       *  coaligned with the s axes */
      inline FloatType getZDist(const FloatType& s, const  FloatType& z) const
      { return mapSToZ(s) - z; }

      /// Gets the signed distance of the sz point to the line in the sz plane
      /** This gives the signed distance of a given point in the sy plane \n
       *  Errors in s and z are equaly weighted in this distance \n
       *  ( which might be an incorrect choice to fit to ).
       *  The plus sign mean the point is below the line. For minus it is above.
       *  The later is naturally choosen as the direction of positiv advance is \n
       *  coaligned with the s axes */
      inline FloatType getSZDist(const FloatType& s, const FloatType& z) const
      { return getSZLine().distance(Vector2D(s, z)) ; }

      /// Passively Shift the s coordinate by the amount given.
      /** Adjust all transvers travel distance measurements by deltaS. \n
       *  The old values of s before the shift corresponds the new scale \n
       *  like old_s - deltaS = new_s. This corresponds to an passive transformation \n
       *  of s by deltaS. This can be used in conjunction with the return value of \n
       *  CDCTrajectory2D::setStartPos2D() in order to move the transvers travel distance \n
       *  by the same amount in both trajectories.*/
      void passiveMoveS(const FloatType& deltaS)
      { m_szLine.passiveMoveAlongFirst(deltaS); }



      /// Indicates if the line has been fitted
      bool isFitted() const
      { return m_szLine.isNull(); }

      /// Clears all information from this trajectory line
      void clear()
      { m_szLine.setNull(); }



      /// Gets the slope over the travel distance coordinate
      inline  FloatType getSZSlope() const
      { return getSZLine().slope(); }

      /// Gets the z coordinate at zero travel distance
      /** This gives the z position at the start point ( zero travel distance ) */
      inline  FloatType getStartZ() const
      { return getSZLine().intercept(); }



      /// Getter for the chi square value of the line fit
      FloatType getChi2() const
      { return getSZLine().chi2(); }

      /// Setter for the chi square value of the line fit
      void setChi2(const FloatType& chi2)
      { return m_szLine.setChi2(chi2); }

      /// Getter for the number of degrees of freedom of the line fit.
      size_t getNDF() const
      { return getSZLine().ndf(); }

      /// Setter for the number of degrees of freedom of the line fit.
      void setNDF(const size_t& ndf)
      { return m_szLine.setNDF(ndf); }



      /// Getter for the line in sz space
      const UncertainSZLine& getSZLine() const
      { return m_szLine; }

      /// Setter for the line in sz space
      void setSZLine(const Line2D& szLine)
      { m_szLine = szLine; }

      /// Setter for the line in sz space
      void setSZLine(const UncertainSZLine& szLine)
      { m_szLine = szLine; }


      /// Setter for slope and intercept of the line
      void setSlopeIntercept(const FloatType& szSlope, const FloatType& startZ)
      { m_szLine.setSlopeIntercept(szSlope, startZ); }

      /// Setter for the normal parameters of the line.
      void setN(const FloatType& n0,
                const FloatType& n1,
                const FloatType& n2) {
        m_szLine.setN(n0, n1, n2);
        if (not getSZLine().alignedWithFirst()) m_szLine.reverse();
      }

    private:
      UncertainSZLine m_szLine; ///< Memory for the line representation

      /// ROOT Macro to make CDCTrajectorySZ a ROOT class.
      ClassDefInCDCLocalTracking(CDCTrajectorySZ, 1);

    }; //class
  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCTRAJECTORYSZ_H
