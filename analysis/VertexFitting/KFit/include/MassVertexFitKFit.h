/**************************************************************************
 * Copyright(C) 1997 - J. Tanaka                                          *
 *                                                                        *
 * Author: J. Tanaka                                                      *
 * Contributor: J. Tanaka and                                             *
 *              conversion to Belle II structure by T. Higuchi            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
     * MassVertexFitKFit is a derived class from KFitBase to perform mass-vertex-constraint kinematical fit.
     */
    class MassVertexFitKFit : public KFitBase {
    public:
      /** Construct an object with no argument. */
      MassVertexFitKFit(void);
      /** Destruct the object. */
      ~MassVertexFitKFit(void);


    public:
      /** Set an initial vertex point for the mass-vertex constraint fit.
       * @param v initial vertex point
       * @return error code (zero if success)
       */
      enum KFitError::ECode       setInitialVertex(const HepPoint3D& v);
      /** Set an initial vertex point for the mass-vertex constraint fit.
       * @param v initial vertex point
       * @return error code (zero if success)
       */
      enum KFitError::ECode       setInitialVertex(const TVector3& v);
      /** Set an invariant mass for the mass-vertex constraint fit.
       * @param m invariant mass
       * @return error code (zero if success)
       */
      enum KFitError::ECode       setInvariantMass(const double m);
      /** Tell the object to fix the last added track property at the invariant mass.
       *  Not intended for end user's use.
       * @return error code (zero if success)
       */
      enum KFitError::ECode       fixMass(void);
      /** Tell the object to unfix the last added track property at the invariant mass.
       *  Not intended for end user's use.
       * @return error code (zero if success)
       */
      enum KFitError::ECode       unfixMass(void);
      enum KFitError::ECode       setCorrelation(const CLHEP::HepMatrix& m) override;
      enum KFitError::ECode       setZeroCorrelation(void) override;


      /** Get a vertex position.
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       * @return vertex position
       */
      const HepPoint3D            getVertex(const int flag = KFitConst::kAfterFit) const;
      /** Get a fitted vertex error matrix.
       * @return vertex error matrix
       */
      const CLHEP::HepSymMatrix          getVertexError(void) const;
      /** Get an invariant mass.
       * @return invariant mass
       */
      double                      getInvariantMass(void) const;
      double                      getCHIsq(void) const override;
      /** Get a vertex error matrix of the track
       * @param id track id
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       * @return vertex error matrix
       */
      const CLHEP::HepMatrix             getTrackVertexError(const int id) const;
      double                      getTrackCHIsq(const int id) const override;
      const CLHEP::HepMatrix             getCorrelation(const int id1, const int id2,
                                                        const int flag = KFitConst::kAfterFit) const override;


    public:
      /** Perform a mass-vertex-constraint fit.
       * @return error code (zero if success)
       */
      enum KFitError::ECode doFit(void);

      /**
       * Update mother particle.
       * @param[in] mother Mother particle.
       */
      enum KFitError::ECode updateMother(Particle* mother);

    private:
      enum KFitError::ECode prepareInputMatrix(void) override;
      enum KFitError::ECode prepareInputSubMatrix(void) override;
      enum KFitError::ECode prepareCorrelation(void) override;
      enum KFitError::ECode prepareOutputMatrix(void) override;
      enum KFitError::ECode makeCoreMatrix(void) override;
      enum KFitError::ECode calculateNDF(void) override;


    private:
      /** Vertex position before the fit. */
      HepPoint3D             m_BeforeVertex;

      /** Vertex position after the fit. */
      HepPoint3D             m_AfterVertex;
      /** Vertex error matrix after the fit. */
      CLHEP::HepSymMatrix           m_AfterVertexError;
      /** array of vertex error matrices after the fit. */
      std::vector<CLHEP::HepMatrix> m_AfterTrackVertexError;

      /** Invariant mass. */
      double m_InvariantMass;
      /** Array of flags whether the track property is fixed at the mass. */
      std::vector<int> m_IsFixMass;
    };

  } // namespace analysis

} // namespace Belle2