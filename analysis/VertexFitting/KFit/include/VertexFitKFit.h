/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <analysis/VertexFitting/KFit/KFitConst.h>
#include <analysis/VertexFitting/KFit/KFitError.h>
#include <analysis/VertexFitting/KFit/KFitBase.h>

#ifndef ENABLE_BACKWARDS_COMPATIBILITY
typedef HepGeom::Point3D<double> HepPoint3D;
#endif


namespace Belle2 {

  namespace analysis {

    /**
     * VertexFitKFit is a derived class from KFitBase to perform vertex-constraint kinematical fit.
     */
    class VertexFitKFit : public KFitBase {
    public:
      /** Construct an object with no argument. */
      VertexFitKFit(void);
      /** Destruct the object. */
      ~VertexFitKFit(void);


    public:
      /** Set an initial vertex point for the vertex-vertex constraint fit.
       * @param v initial vertex point
       * @return error code (zero if success)
       */
      enum KFitError::ECode       setInitialVertex(const HepPoint3D& v);
      /** Set an initial vertex point for the mass-vertex constraint fit.
       * @param v initial vertex point
       * @return error code (zero if success)
       */
      enum KFitError::ECode       setInitialVertex(const TVector3& v);
      /** Set an IP-ellipsoid shape for the vertex constraint fit.
       * @param ip IP position
       * @param ipe error matrix of the IP
       * @return error code (zero if success)
       */
      enum KFitError::ECode       setIpProfile(const HepPoint3D& ip, const CLHEP::HepSymMatrix& ipe);
      /** Set a virtual IP-tube track for the vertex constraint fit.
       * @param p Lorentz vector of the virtual IP-tube track
       * @param x IP position
       * @param e error matrix of IP-tube track and IP position
       * @param q charge of the virtual IP-tube track
       * @return error code (zero if success)
       */
      enum KFitError::ECode       setIpTubeProfile(const CLHEP::HepLorentzVector& p, const HepPoint3D& x, const CLHEP::HepSymMatrix& e,
                                                   const double q);
      /** Tell the object to perform a fit with vertex position fixed.
       * @param flag true for fixed vertex, false for otherwise
       * @return error code (zero if success)
       */
      enum KFitError::ECode       setKnownVertex(const bool flag = true);
      /** Tell the object to perform a fit with track correlations.
       * @param m true for with correlation, false for otherwise
       * @return error code (zero if success)
       */
      enum KFitError::ECode       setCorrelationMode(const bool m);


    public:
      /** Get a vertex position.
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       * @return vertex position
       */
      const HepPoint3D            getVertex(const int flag = KFitConst::kAfterFit) const;
      /** Get a fitted vertex error matrix.
       * @return vertex error matrix
       */
      const CLHEP::HepSymMatrix          getVertexError(void) const;
      double                      getCHIsq(void) const override;
      /** Get a chi-square of the fit excluding IP-constraint part.
       * @return chi-square of the fit excluding IP-constraint part.
       */
      double                      getCHIsqVertex(void) const;
      /** Get a vertex error matrix of the track
       * @param id track id
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       * @return vertex error matrix
       */
      const CLHEP::HepMatrix             getTrackVertexError(const int id) const;
      double                      getTrackCHIsq(const int id) const override;
      /** Get a sum of the chi-square associated to the input tracks.
       *  The return value should be the same as the one from getCHIsqVertex().
       * @return sum of the chi-square associated to the input tracks
       */
      double                      getTrackPartCHIsq(void) const;
      /** Get an NDF relevant to the getTrackPartCHIsq().
       * @return NDF relevant to the getTrackPartCHIsq()
       */
      int                         getTrackPartNDF(void) const;


    public:
      /** Perform a vertex-constraint fit.
       * @return error code (zero if success)
       */
      enum KFitError::ECode doFit(void);

      /**
       * Update mother particle.
       * @param[in] mother Mother particle.
       */
      enum KFitError::ECode updateMother(Particle* mother);

    private:
      /** Perform a standard vertex-constraint fit including IP-tube constraint.
       * @return error code (zero if success)
       */
      enum KFitError::ECode doFit3(void);
      /** Perform a IP-ellipsoid and vertex-constraint fit.
       * @return error code (zero if success)
       */
      enum KFitError::ECode doFit4(void);
      /** Perform a fixed-vertex-position fit mainly for slow pion.
       * @return error code (zero if success)
       */
      enum KFitError::ECode doFit5(void);


    private:
      enum KFitError::ECode prepareInputMatrix(void) override;
      enum KFitError::ECode prepareInputSubMatrix(void) override;
      enum KFitError::ECode prepareOutputMatrix(void) override;
      enum KFitError::ECode makeCoreMatrix(void) override;
      enum KFitError::ECode calculateNDF(void) override;


    private:
      /** Add the virtual tube track to m_Tracks just before the internal minimization call.
       * @return error code (zero if success)
       */
      enum KFitError::ECode appendTube(void);
      /** Delete the virtual tube track to m_Tracks just after the internal minimization call.
       * @return error code (zero if success)
       */
      enum KFitError::ECode deleteTube(void);


    private:
      /** Flag controlled by setCorrelationMode(). */
      bool   m_CorrelationMode;
      /** Container of chi-square's of the input tracks. */
      double m_EachCHIsq[KFitConst::kMaxTrackCount2];
      /** chi-square of the fit excluding IP-constraint part. */
      double m_CHIsqVertex;

      /** Vertex position before the fit. */
      HepPoint3D             m_BeforeVertex;

      /** Vertex position after the fit. */
      HepPoint3D             m_AfterVertex;
      /** Vertex error matrix after the fit. */
      CLHEP::HepSymMatrix           m_AfterVertexError;
      /** Array of vertex error matrices after the fit. */
      std::vector<CLHEP::HepMatrix> m_AfterTrackVertexError;

      /** Flag if to perform IP-ellipsoid constraint fit. */
      bool         m_FlagBeam;
      /** Error matrix modeling the IP ellipsoid. */
      CLHEP::HepSymMatrix m_BeamError;

      /** Flag controlled by setKnownVertex(). */
      bool m_FlagKnownVertex;

      /** Flag if to perform IP-tube constraint fit. */
      bool      m_FlagTube;
      /** ID of the virtual tube track in the m_Tracks. */
      int       m_iTrackTube;
      /** Entity of the virtual IP-tube track. */
      KFitTrack m_TubeTrack;
    };

  } // namespace analysis

} // namespace Belle2
