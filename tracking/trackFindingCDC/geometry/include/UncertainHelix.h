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

#include <tracking/trackFindingCDC/geometry/Helix.h>
#include <tracking/trackFindingCDC/geometry/HelixParameters.h>

#include <tracking/trackFindingCDC/geometry/UncertainPerigeeCircle.h>
#include <tracking/trackFindingCDC/geometry/UncertainSZLine.h>

#include <cmath>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// A general helix class including a covariance matrix.
    class UncertainHelix : public Helix {

    public:
      /// Default constructor for ROOT compatibility.
      UncertainHelix()
      {}

      /// Composes an uncertain perigee circle from the  perigee parameters and a 3x3 covariance matrix. Covariance matrix defaults to a zero matrix
      UncertainHelix(const double curvature,
                     const double phi0,
                     const double impact,
                     const double tanLambda,
                     const double z0,
                     const HelixCovariance& helixCovariance = HelixUtil::identity(),
                     const double chi2 = 0.0,
                     const size_t& ndf = 0)
        : Helix(curvature, phi0, impact, tanLambda, z0),
          m_helixCovariance(helixCovariance),
          m_chi2(chi2),
          m_ndf(ndf)
      {}


      /// Constructor taking all stored parameters
      explicit UncertainHelix(const HelixParameters& parameters,
                              const HelixCovariance& helixCovariance = HelixUtil::identity(),
                              const double chi2 = 0.0,
                              const size_t& ndf = 0)
        : Helix(parameters),
          m_helixCovariance(helixCovariance),
          m_chi2(chi2),
          m_ndf(ndf)
      {}

      /// Composes an uncertain perigee circle from the  perigee parameters and a 3x3 covariance matrix. Covariance matrix defaults to a zero matrix
      UncertainHelix(const double curvature,
                     const Vector2D& phi0Vec,
                     const double impact,
                     const double tanLambda,
                     const double z0,
                     const HelixCovariance& helixCovariance = HelixUtil::identity(),
                     const double chi2 = 0.0,
                     const size_t& ndf = 0)
        : Helix(curvature, phi0Vec, impact, tanLambda, z0),
          m_helixCovariance(helixCovariance),
          m_chi2(chi2),
          m_ndf(ndf)
      {}

      /// Augments a plain helix with a covariance matrix. Covariance defaults to zero.
      UncertainHelix(const Helix& helix,
                     const HelixCovariance& helixCovariance = HelixUtil::identity(),
                     const double chi2 = 0.0,
                     const size_t& ndf = 0)
        : Helix(helix),
          m_helixCovariance(helixCovariance),
          m_chi2(chi2),
          m_ndf(ndf)
      {}

      /// Augments a plain helix with a covariance matrix. Covariance defaults to zero.
      UncertainHelix(const UncertainPerigeeCircle& uncertainPerigeeCircle,
                     const UncertainSZLine& uncertainSZLine)
        : Helix(uncertainPerigeeCircle, uncertainSZLine), // copies line and circle without uncertainties
          m_helixCovariance(HelixUtil::stackBlocks(uncertainPerigeeCircle.perigeeCovariance(), uncertainSZLine.szCovariance())),
          m_chi2(uncertainPerigeeCircle.chi2() + uncertainSZLine.chi2()),
          m_ndf(uncertainPerigeeCircle.ndf() + uncertainSZLine.ndf())
      {}

      /// Construct the averages of the two given helices by properly considering their covariance matrix.
      static UncertainHelix average(const UncertainHelix& helix1,
                                    const UncertainHelix& helix2);

      /**
       *  Construct the average helix including its covariance matrix from two different stereo angle projections.
       *
       *  The averaging in the higher dimensional helix parameter space from the lower dimensional projectsions
       *  is accomplished by considering the ambiguity matrix of the projections.
       *  The average only succeeds when the projections are not parallel two each other which is generally the case
       *  for two different super layers of different stereo kind.
       *
       *  Both circle and helix parameters and their covariance matrix are considered to be passed on the same origin.
       *
       *  @param perigeeCircle1    First perigee circle
       *  @param ambiguityMatrix1  Ambiguity matrix of the first perigee parameters with respect to the helix parameters
       *  @param perigeeCircle2    Second perigee circle
       *  @param ambiguityMatrix2  Ambiguity matrix of the second perigee parameters with respect to the helix parameters
       *  @param szParameters      Reference sz parameters where the perigee circles have been fitted.
       */
      static UncertainHelix average(const UncertainPerigeeCircle& perigeeCircle1,
                                    const JacobianMatrix<3, 5>& ambiguityMatrix1,
                                    const UncertainPerigeeCircle& perigeeCircle2,
                                    const JacobianMatrix<3, 5>& ambiguityMatrix2,
                                    const SZParameters& szParameters = SZParameters(0.0, 0.0));

      /**
       *  Construct the average helix including its covariance matrix incoorporating additional information from a stereo projection.
       *
       *  The averaging in the higher dimensional helix parameter space from the lower dimensional projectsions
       *  is accomplished by considering the ambiguity matrix of the projection.
       *
       *  Both circle and helix parameters and their covariance matrix are considered to be passed on the same origin.
       *  The circle is considered to be fitted in the sz space defined by the given helix.
       *
       *  @param helix            Second perigee circle
       *  @param perigeeCircle    Perigee circle to be incoorporated
       *  @param ambiguityMatrix  Ambiguity matrix of the perigee parameters with respect to the helix parameters
       */
      static UncertainHelix average(const UncertainHelix& helix,
                                    const UncertainPerigeeCircle& perigeeCircle,
                                    const JacobianMatrix<3, 5>& ambiguityMatrix)
      { return average(perigeeCircle, ambiguityMatrix, helix); }

      /**
       *  Construct the average helix including its covariance matrix incoorporating additional information from a stereo projection.
       *
       *  The averaging in the higher dimensional helix parameter space from the lower dimensional projectsions
       *  is accomplished by considering the ambiguity matrix of the projection.
       *
       *  Both circle and helix parameters and their covariance matrix are considered to be passed on the same origin.
       *  The circle is considered to be fitted in the sz space defined by the given helix.
       *
       *  @param perigeeCircle    Perigee circle to be incoorporated
       *  @param ambiguityMatrix  Ambiguity matrix of the perigee parameters with respect to the helix parameters
       *  @param helix            Second perigee circle
       */
      static UncertainHelix average(const UncertainPerigeeCircle& perigeeCircle,
                                    const JacobianMatrix<3, 5>& ambiguityMatrix,
                                    const UncertainHelix& helix);

    public:
      /// Projects the helix into the xy plain carrying over the relevant parts of the convariance matrix.
      UncertainPerigeeCircle uncertainCircleXY() const
      { return UncertainPerigeeCircle(circleXY(), HelixUtil::getPerigeeCovariance(helixCovariance()));}

      /// Reduces the helix to an sz line carrying over the relevant parts of the convariance matrix.
      UncertainSZLine uncertainSZLine() const
      { return UncertainSZLine(szLine(), HelixUtil::getSZCovariance(helixCovariance()));}

      /// Setter for the whole covariance matrix of the perigee parameters
      inline void setHelixCovariance(const HelixCovariance& helixCovariance)
      { m_helixCovariance = helixCovariance; }

      /// Getter for the whole covariance matrix of the perigee parameters
      const HelixCovariance& helixCovariance() const
      { return m_helixCovariance; }

      /// Getter for individual elements of the covariance matrix
      double covariance(const EHelixParameter& iRow, const EHelixParameter& iCol) const
      { return helixCovariance()(iRow, iCol); }

      /// Getter for individual diagonal elements of the covariance matrix
      double variance(const EHelixParameter& i) const
      { return helixCovariance()(i, i); }

      /// Getter for the chi square value of the helix fit
      double chi2() const
      { return m_chi2; }

      /// Setter for the chi square value of the helix fit
      void setChi2(const double chi2)
      { m_chi2 = chi2; }

      /// Getter for the number of degrees of freediom used in the helix fit
      const size_t& ndf() const
      { return m_ndf; }

      /// Setter for the number of degrees of freediom used in the helix fit
      void setNDF(const size_t& ndf)
      { m_ndf = ndf; }

      /// Sets all circle parameters to zero including the covariance matrix
      inline void invalidate()
      {
        Helix::invalidate();
        m_helixCovariance = HelixUtil::identity();
        m_chi2 = 0.0;
        m_ndf = 0;
      }

    public:
      /// Flips the orientation of the circle in place
      inline void reverse()
      {
        Helix::reverse();
        HelixUtil::reverse(m_helixCovariance);
      }

      /// Returns a copy of the circle with opposite orientation.
      inline UncertainHelix reversed() const
      { return UncertainHelix(Helix::reversed(), HelixUtil::reversed(m_helixCovariance), chi2(), ndf()); }

    public:
      /// Moves the coordinate system by the vector by and calculates the new perigee and its covariance matrix. Change is inplace.
      void passiveMoveBy(const Vector3D& by)
      {
        // Move the covariance matrix first to have access to the original parameters
        HelixJacobian jacobian = passiveMoveByJacobian(by);
        HelixUtil::transport(jacobian, m_helixCovariance);
        Helix::passiveMoveBy(by);
      }

    private:
      /// Memory for the 5x5 covariance matrix of the helix parameters.
      HelixCovariance m_helixCovariance = HelixUtil::identity();

      /// Memory for the chi square value of the fit of this helix.
      double m_chi2 = 0.0;

      /// Memory for the number of degrees of freedom of the fit of this helix.
      size_t m_ndf = 0.0;

    }; //class

  } // namespace TrackFindingCDC
} // namespace Belle2
