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

#include <tracking/trackFindingCDC/geometry/UncertainPerigeeCircle.h>
#include <tracking/trackFindingCDC/geometry/UncertainSZLine.h>
#include <tracking/trackFindingCDC/geometry/SZLine.h>

#include <tracking/trackFindingCDC/geometry/HelixParameters.h>
#include <tracking/trackFindingCDC/geometry/PerigeeParameters.h>
#include <tracking/trackFindingCDC/geometry/SZParameters.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <cstddef>
#include <iosfwd>

namespace Belle2 {

  namespace TrackFindingCDC {
    class Vector3D;

    /// A general helix class including a covariance matrix.
    class UncertainHelix {

    public:
      /// Default constructor for ROOT compatibility.
      UncertainHelix()
      {
      }

      /// Composes an uncertain perigee circle from the  perigee parameters and a 3x3 covariance
      /// matrix.
      UncertainHelix(const double curvature,
                     const double phi0,
                     const double impact,
                     const double tanLambda,
                     const double z0,
                     const HelixCovariance& helixCovariance = HelixUtil::identity(),
                     const double chi2 = 0.0,
                     std::size_t ndf = 0)
        : m_helix(curvature, phi0, impact, tanLambda, z0)
        , m_helixCovariance(helixCovariance)
        , m_chi2(chi2)
        , m_ndf(ndf)
      {
      }

      /// Constructor taking all stored parameters
      explicit UncertainHelix(const HelixParameters& parameters,
                              const HelixCovariance& helixCovariance = HelixUtil::identity(),
                              const double chi2 = 0.0,
                              std::size_t ndf = 0)
        : m_helix(parameters)
        , m_helixCovariance(helixCovariance)
        , m_chi2(chi2)
        , m_ndf(ndf)
      {
      }

      /// Composes an uncertain perigee circle from the  perigee parameters and a 3x3 covariance
      /// matrix.
      UncertainHelix(const double curvature,
                     const Vector2D& phi0Vec,
                     const double impact,
                     const double tanLambda,
                     const double z0,
                     const HelixCovariance& helixCovariance = HelixUtil::identity(),
                     const double chi2 = 0.0,
                     std::size_t ndf = 0)
        : m_helix(curvature, phi0Vec, impact, tanLambda, z0)
        , m_helixCovariance(helixCovariance)
        , m_chi2(chi2)
        , m_ndf(ndf)
      {
      }

      /// Augments a plain helix with a covariance matrix. Covariance defaults to zero.
      UncertainHelix(const Helix& helix,
                     const HelixCovariance& helixCovariance = HelixUtil::identity(),
                     const double chi2 = 0.0,
                     std::size_t ndf = 0)
        : m_helix(helix)
        , m_helixCovariance(helixCovariance)
        , m_chi2(chi2)
        , m_ndf(ndf)
      {
      }

      /// Augments a plain helix with a covariance matrix. Covariance defaults to zero.
      UncertainHelix(const UncertainPerigeeCircle& uncertainPerigeeCircle,
                     const UncertainSZLine& uncertainSZLine)
        : m_helix(uncertainPerigeeCircle, uncertainSZLine)
        , m_helixCovariance(HelixUtil::stackBlocks(uncertainPerigeeCircle.perigeeCovariance(),
                                                   uncertainSZLine.szCovariance()))
        , m_chi2(uncertainPerigeeCircle.chi2() + uncertainSZLine.chi2())
        , m_ndf(uncertainPerigeeCircle.ndf() + uncertainSZLine.ndf())
      {
      }

      /// Construct the averages of the two given helices by properly considering their covariance
      /// matrix.
      static UncertainHelix average(const UncertainHelix& fromHelix, const UncertainHelix& toHelix);

      /**
       *  Construct the average helix including its covariance matrix from two different stereo
       *  angle projections.
       *
       *  The averaging in the higher dimensional helix parameter space from the lower dimensional
       *  projections
       *  is accomplished by considering the ambiguity matrix of the projections.
       *  The average only succeeds when the projections are not parallel two each other which is
       *  generally the case
       *  for two different super layers of different stereo kind.
       *
       *  Both circle and helix parameters and their covariance matrix are considered to be passed
       *  on the same origin.
       *
       *  @param perigeeCircle1    First perigee circle
       *  @param ambiguityMatrix1  Ambiguity matrix of the first perigee parameters with respect to
       * the helix parameters
       *  @param perigeeCircle2    Second perigee circle
       *  @param ambiguityMatrix2  Ambiguity matrix of the second perigee parameters with respect to
       * the helix parameters
       *  @param szParameters      Reference sz parameters where the perigee circles have been
       * fitted.
       */
      static UncertainHelix average(const UncertainPerigeeCircle& fromPerigeeCircle,
                                    const PerigeeHelixAmbiguity& fromAmbiguity,
                                    const UncertainPerigeeCircle& toPerigeeCircle,
                                    const PerigeeHelixAmbiguity& toAmbiguity,
                                    const SZParameters& szParameters);

      /**
       *  Construct the average helix including its covariance matrix incoorporating additional
       * information from a stereo projection.
       *
       *  The averaging in the higher dimensional helix parameter space from the lower dimensional
       * projectsions
       *  is accomplished by considering the ambiguity matrix of the projection.
       *
       *  Both circle and helix parameters and their covariance matrix are considered to be passed
       * on the same origin.
       *  The circle is considered to be fitted in the sz space defined by the given helix.
       *
       *  @param helix            Second perigee circle
       *  @param perigeeCircle    Perigee circle to be incoorporated
       *  @param ambiguityMatrix  Ambiguity matrix of the perigee parameters with respect to the
       * helix parameters
       */
      static UncertainHelix average(const UncertainHelix& helix,
                                    const UncertainPerigeeCircle& perigeeCircle,
                                    const PerigeeHelixAmbiguity& ambiguityMatrix)
      {
        return average(perigeeCircle, ambiguityMatrix, helix);
      }

      /**
       *  Construct the average helix including its covariance matrix incoorporating additional
       * information from a stereo projection.
       *
       *  The averaging in the higher dimensional helix parameter space from the lower dimensional
       * projectsions
       *  is accomplished by considering the ambiguity matrix of the projection.
       *
       *  Both circle and helix parameters and their covariance matrix are considered to be passed
       * on the same origin.
       *  The circle is considered to be fitted in the sz space defined by the given helix.
       *
       *  @param perigeeCircle    Perigee circle to be incoorporated
       *  @param ambiguityMatrix  Ambiguity matrix of the perigee parameters with respect to the
       * helix parameters
       *  @param helix            Second perigee circle
       */
      static UncertainHelix average(const UncertainPerigeeCircle& fromPerigeeCircle,
                                    const PerigeeHelixAmbiguity& fromAmbiguity,
                                    const UncertainHelix& toHelix);

    public:
      /**
       *  Access to the constant interface of the underlying parameter line
       *  Allows the user of this "super" class to use the getters and
       *  other methods to get inforamation about the line but disallows mutation
       *  of internal state.
       *  This ensures that the parameters are not changed without proper adjustment to
       *  the covariance matrix in this class, which can be achieved by the more limited
       *  set of methods in this class accessable by normal . method calls
       */
      const Helix* operator->() const
      {
        return &m_helix;
      }

      /// Downcast to the "super" class
      operator const Helix& () const
      {
        return m_helix;
      }

      /// Getter for the underlying helix
      const Helix& helix() const
      {
        return m_helix;
      }

      /// Getter for the perigee parameters in the order defined by EPerigeeParameter.h
      HelixParameters helixParameters() const
      {
        return m_helix.helixParameters();
      }

    public:
      /// Projects the helix into the xy plain carrying over the relevant parts of the convariance
      /// matrix.
      UncertainPerigeeCircle uncertainCircleXY() const
      {
        return UncertainPerigeeCircle(helix().circleXY(),
                                      HelixUtil::getPerigeeCovariance(helixCovariance()));
      }

      /// Reduces the helix to an sz line carrying over the relevant parts of the convariance
      /// matrix.
      UncertainSZLine uncertainSZLine() const
      {
        return UncertainSZLine(helix().szLine(), HelixUtil::getSZCovariance(helixCovariance()));
      }

      /// Setter for the whole covariance matrix of the perigee parameters
      void setHelixCovariance(const HelixCovariance& helixCovariance)
      {
        m_helixCovariance = helixCovariance;
      }

      /// Getter for the whole covariance matrix of the perigee parameters
      const HelixCovariance& helixCovariance() const
      {
        return m_helixCovariance;
      }

      /// Getter for individual elements of the covariance matrix
      double covariance(const EHelixParameter& iRow, const EHelixParameter& iCol) const
      {
        return helixCovariance()(iRow, iCol);
      }

      /// Getter for individual diagonal elements of the covariance matrix
      double variance(const EHelixParameter& i) const
      {
        return helixCovariance()(i, i);
      }

      /// Getter for the chi square value of the helix fit
      double chi2() const
      {
        return m_chi2;
      }

      /// Setter for the chi square value of the helix fit
      void setChi2(const double chi2)
      {
        m_chi2 = chi2;
      }

      /// Getter for the number of degrees of freediom used in the helix fit
      std::size_t ndf() const
      {
        return m_ndf;
      }

      /// Setter for the number of degrees of freediom used in the helix fit
      void setNDF(std::size_t ndf)
      {
        m_ndf = ndf;
      }

      /// Sets all circle parameters to zero and the covariance matrix to something noninformative
      void invalidate()
      {
        m_helix.invalidate();
        m_helixCovariance = HelixUtil::identity();
        m_chi2 = 0.0;
        m_ndf = 0;
      }

    public:
      /// Flips the orientation of the circle in place
      void reverse()
      {
        m_helix.reverse();
        HelixUtil::reverse(m_helixCovariance);
      }

      /// Returns a copy of the circle with opposite orientation.
      UncertainHelix reversed() const
      {
        return UncertainHelix(m_helix.reversed(),
                              HelixUtil::reversed(m_helixCovariance),
                              chi2(),
                              ndf());
      }

    public:
      /// Moves the coordinate system by the vector by and calculates the new perigee and its
      /// covariance matrix. Change is inplace.
      void passiveMoveBy(const Vector3D& by)
      {
        // Move the covariance matrix first to have access to the original parameters
        HelixJacobian jacobian = m_helix.passiveMoveByJacobian(by);
        HelixUtil::transport(jacobian, m_helixCovariance);
        m_helix.passiveMoveBy(by);
      }

      /**
       *  Adjust the arclength measure to start n periods later.
       *  @return The arc length needed to travel n periods.
       */
      double shiftPeriod(int nPeriods)
      {
        double arcLength2D = m_helix.shiftPeriod(nPeriods);
        SZJacobian szJacobian = m_helix.szLine().passiveMoveByJacobian(Vector2D(arcLength2D, 0));
        PerigeeJacobian perigeeJacobian = PerigeeUtil::identity();
        HelixJacobian jacobian = HelixUtil::stackBlocks(perigeeJacobian, szJacobian);
        HelixUtil::transport(jacobian, m_helixCovariance);
        return arcLength2D;
      }

    private:
      /// Memory for the underlying helix
      Helix m_helix;

      /// Memory for the 5x5 covariance matrix of the helix parameters.
      HelixCovariance m_helixCovariance = HelixUtil::identity();

      /// Memory for the chi square value of the fit of this helix.
      double m_chi2 = 0.0;

      /// Memory for the number of degrees of freedom of the fit of this helix.
      size_t m_ndf = 0.0;

    };

    /// Debug helper
    std::ostream& operator<<(std::ostream& output, const UncertainHelix& uncertainHelix);
  }
}
