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


using namespace std;
using namespace CLHEP;
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
      const enum KFitError::ECode setInitialVertex(const HepPoint3D& v);
      /** Set an invariant mass for the mass-vertex constraint fit.
       * @param m invariant mass
       * @return error code (zero if success)
       */
      const enum KFitError::ECode setInvariantMass(const double m);
      /** Tell the object to fix the last added track property at the invariant mass.
       *  Not intended for end user's use.
       * @return error code (zero if success)
       */
      const enum KFitError::ECode fixMass(void);
      /** Tell the object to unfix the last added track property at the invariant mass.
       *  Not intended for end user's use.
       * @return error code (zero if success)
       */
      const enum KFitError::ECode unfixMass(void);
      const enum KFitError::ECode setCorrelation(const HepMatrix& m);
      const enum KFitError::ECode setZeroCorrelation(void);


      /** Get a vertex position.
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       * @return vertex position
       */
      const HepPoint3D            getVertex(const int flag = KFitConst::kAfterFit) const;
      /** Get a fitted vertex error matrix.
       * @return vertex error matrix
       */
      const HepSymMatrix          getVertexError(void) const;
      /** Get an invariant mass.
       * @return invariant mass
       */
      const double                getInvariantMass(void) const;
      const double                getCHIsq(void) const;
      /** Get a vertex error matrix of the track
       * @param id track id
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       * @return vertex error matrix
       */
      const HepMatrix             getTrackVertexError(const int id) const;
      const double                getTrackCHIsq(const int id) const;
      const HepMatrix             getCorrelation(const int id1, const int id2, const int flag = KFitConst::kAfterFit) const;


    public:
      /** Perform a mass-vertex-constraint fit.
       * @return error code (zero if success)
       */
      const enum KFitError::ECode doFit(void);


    private:
      const enum KFitError::ECode prepareInputMatrix(void);
      const enum KFitError::ECode prepareInputSubMatrix(void);
      const enum KFitError::ECode prepareCorrelation(void);
      const enum KFitError::ECode prepareOutputMatrix(void);
      const enum KFitError::ECode makeCoreMatrix(void);
      const enum KFitError::ECode calculateNDF(void);


    private:
      /** Vertex position before the fit. */
      HepPoint3D             m_BeforeVertex;

      /** Vertex position after the fit. */
      HepPoint3D             m_AfterVertex;
      /** Vertex error matrix after the fit. */
      HepSymMatrix           m_AfterVertexError;
      /** array of vertex error matrices after the fit. */
      vector<HepMatrix> m_AfterTrackVertexError;

      /** Invariant mass. */
      double m_InvariantMass;
      /** Array of flags whether the track property is fixed at the mass. */
      vector<int> m_IsFixMass;
    };

  } // namespace analysis

} // namespace Belle2

#endif /* MASSVERTEXFITKFIT_H */

