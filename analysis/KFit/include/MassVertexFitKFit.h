/**************************************************************************
 * Copyright(C) 1997 - J. Tanaka                                          *
 *                                                                        *
 * Author: J. Tanaka                                                      *
 * Contributor: J. Tanaka and                                             *
 *              conversion to Belle II structure by T. Higuchi            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef MASSVERTEXFITKFIT_H
#define MASSVERTEXFITKFIT_H


#include <framework/logging/Logger.h>

#include <analysis/KFit/KFitConst.h>
#include <analysis/KFit/KFitError.h>
#include <analysis/KFit/KFitBase.h>


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
      enum KFitError::ECode       setCorrelation(const CLHEP::HepMatrix& m);
      enum KFitError::ECode       setZeroCorrelation(void);


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
      double                      getCHIsq(void) const;
      /** Get a vertex error matrix of the track
       * @param id track id
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       * @return vertex error matrix
       */
      const CLHEP::HepMatrix             getTrackVertexError(const int id) const;
      double                      getTrackCHIsq(const int id) const;
      const CLHEP::HepMatrix             getCorrelation(const int id1, const int id2, const int flag = KFitConst::kAfterFit) const;


    public:
      /** Perform a mass-vertex-constraint fit.
       * @return error code (zero if success)
       */
      enum KFitError::ECode doFit(void);


    private:
      enum KFitError::ECode prepareInputMatrix(void);
      enum KFitError::ECode prepareInputSubMatrix(void);
      enum KFitError::ECode prepareCorrelation(void);
      enum KFitError::ECode prepareOutputMatrix(void);
      enum KFitError::ECode makeCoreMatrix(void);
      enum KFitError::ECode calculateNDF(void);


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

#endif /* MASSVERTEXFITKFIT_H */

