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
#include <Math/Vector4D.h>

#ifndef ENABLE_BACKWARDS_COMPATIBILITY
typedef HepGeom::Point3D<double> HepPoint3D;
#endif

namespace Belle2 {

  namespace analysis {

    /**
     * MassFourCFitKFit is a derived class from KFitBase to perform mass and 4 momentum-constraint kinematical fit.
     */
    class MassFourCFitKFit : public KFitBase {
    public:
      /** Construct an object with no argument. */
      MassFourCFitKFit();
      /** Destruct the object. */
      ~MassFourCFitKFit(void);


    public:


      /** Set an invariant mass of daughter particle for the mass-four-momentum-constraint fit.
       * @param m invariant mass
       * @param childTrackId vector of daughter track ids
       * @return error code (zero if success)
       */
      enum KFitError::ECode       addMassConstraint(const double m, std::vector<unsigned>& childTrackId);

      /** Set an initial vertex position for the mass-four-momentum-constraint fit.
       * @param v vertex position
       * @return error code (zero if success)
       */
      enum KFitError::ECode       setVertex(const HepPoint3D& v);
      /** Set an initial vertex error matrix for the mass-four-momentum-constraint fit.
       * @param e vertex error matrix
       * @return error code (zero if success)
       */
      enum KFitError::ECode       setVertexError(const CLHEP::HepSymMatrix& e);
      /** Set an invariant mass for the mass-four-momentum-constraint fit.
       * @param m invariant mass
       * @return error code (zero if success)
       */
      enum KFitError::ECode       setInvariantMass(const double m);
      /** Set an 4 Momentum for the mass-four-constraint fit.
       * @param m four momentum
       * @return error code (zero if success)
       */
      enum KFitError::ECode       setFourMomentum(const ROOT::Math::PxPyPzEVector& m);
      /** Set a flag if to constraint at the decay point in the mass-four-momentum-constraint fit.
       * @param flag true for constraint, false for not
       * @return error code (zero if success)
       */
      enum KFitError::ECode       setFlagAtDecayPoint(const bool flag);
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
      /** Set a vertex error matrix of the child particle in the addTrack'ed order.
       * @param e (3x7) vertex error matrix
       * @return error code (zero if success)
       */
      enum KFitError::ECode       setTrackVertexError(const CLHEP::HepMatrix& e);
      /** Indicate no vertex uncertainty in the child particle in the addTrack'ed order.
       * @return error code (zero if success)
       */
      enum KFitError::ECode       setTrackZeroVertexError(void);
      enum KFitError::ECode       setCorrelation(const CLHEP::HepMatrix& m) override;
      enum KFitError::ECode       setZeroCorrelation(void) override;


      /** Get a vertex position.
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       * @return vertex position
       */
      const HepPoint3D            getVertex(const int flag = KFitConst::kAfterFit) const;
      /** Get a vertex error matrix.
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       * @return vertex error matrix
       */
      const CLHEP::HepSymMatrix          getVertexError(const int flag = KFitConst::kAfterFit) const;
      /** Get an invariant mass.
       * @return invariant mass
      */
      double                      getInvariantMass(void) const;
      /** Get a flag if to constraint at the decay point in the mass-four-momentum-constraint fit.
       * @return flag value
       */
      bool                        getFlagAtDecayPoint(void) const;
      /** Get a flag if the fit is allowed with moving the vertex position.
       * @return flag value
       */
      bool                        getFlagFitWithVertex(void) const;
      double                      getCHIsq(void) const override;
      /** Get a vertex error matrix of the track
       * @param id track id
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       * @return vertex error matrix
       */
      const CLHEP::HepMatrix             getTrackVertexError(const int id, const int flag = KFitConst::kAfterFit) const;
      double                      getTrackCHIsq(const int id) const override;
      const CLHEP::HepMatrix             getCorrelation(const int id1, const int id2,
                                                        const int flag = KFitConst::kAfterFit) const override;


    public:
      /** Perform a mass-four-momentum-constraint fit.
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
      /** Vertex error matrix before the fit. */
      CLHEP::HepSymMatrix           m_BeforeVertexError;
      /** array of vertex error matrices before the fit. */
      std::vector<CLHEP::HepMatrix> m_BeforeTrackVertexError;

      /** Vertex position after the fit. */
      HepPoint3D             m_AfterVertex;
      /** Vertex error matrix after the fit. */
      CLHEP::HepSymMatrix           m_AfterVertexError;
      /** array of vertex error matrices after the fit. */
      std::vector<CLHEP::HepMatrix> m_AfterTrackVertexError;

      /** Flag to indicate if the vertex error matrix of the child particle is preset. */
      bool m_FlagTrackVertexError;
      /** Flag to indicate if the fit is allowed with moving the vertex position. */
      bool m_FlagFitIncludingVertex;
      /** Flag controlled by setFlagAtDecayPoint(). */
      bool m_FlagAtDecayPoint;

      /** Invariant mass. */
      double m_InvariantMass;

      /** Four Momentum. */
      ROOT::Math::PxPyPzEVector m_FourMomentum;

      /** Array of flags whether the track property is fixed at the mass. */
      std::vector<int> m_IsFixMass;

      /** # of constrained mass . */
      int m_ConstraintMassCount;

      /** constrained mass */
      std::vector<double> m_ConstraintMassList;

      /** Daughter track id of constrained particle*/
      std::vector<std::pair<int, int>> m_ConstraintMassChildLists;
    };

  } // namespace analysis

} // namespace Belle2
