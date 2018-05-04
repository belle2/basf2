/**************************************************************************
 * Copyright(C) 1997 - J. Tanaka                                          *
 *                                                                        *
 * Author: J. Tanaka                                                      *
 * Contributor: J. Tanaka and                                             *
 *              conversion to Belle II structure by T. Higuchi            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef MAKEMOTHERKFIT_H
#define MAKEMOTHERKFIT_H


#include <iostream>
#include <vector>

#include <CLHEP/Matrix/Matrix.h>
#include <CLHEP/Matrix/SymMatrix.h>
#include <CLHEP/Vector/ThreeVector.h>
#include <CLHEP/Vector/LorentzVector.h>
#include <CLHEP/Geometry/Point3D.h>

#include <framework/logging/Logger.h>

#include <analysis/KFit/KFitConst.h>
#include <analysis/KFit/KFitError.h>
#include <analysis/KFit/KFitTrack.h>

#ifndef ENABLE_BACKWARDS_COMPATIBILITY
typedef HepGeom::Point3D<double> HepPoint3D;
#endif


namespace Belle2 {

  namespace analysis {

    /**
     * MakeMotherKFit is a class to build mother particle from kinematically fitted daughters.
     */
    class MakeMotherKFit {
    public:
      /** Construct an object with no argument. */
      MakeMotherKFit(void);
      /** Destruct the object. */
      ~MakeMotherKFit(void);

      /** Add a track to the make-mother object.
       * @param kp object of the track
       * @return error code (zero if success)
       */
      enum KFitError::ECode       addTrack(const KFitTrack& kp);
      /** Add a track to the make-mother object with specifying its momentum, position, error matrix, and charge.
       *  This function internally calls addTrack(const KFitTrack &kp).
       * @param p Lorentz vector of the track
       * @param x position of the track
       * @param e (7x7) error matrix of the track
       * @param q charge of the track
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       * @return error code (zero if success)
       */
      enum KFitError::ECode       addTrack(const CLHEP::HepLorentzVector& p, const HepPoint3D& x, const CLHEP::HepSymMatrix& e,
                                           const double q, const int flag = KFitConst::kAfterFit);
      /** Change a magnetic field from the default value KFitConst::kDefaultMagneticField.
       * @param mf magnetic field to set
       * @return error code (zero if success)
       */
      enum KFitError::ECode       setMagneticField(const double mf);
      /** Set a vertex position of the mother particle.
       * @param v vertex position
       * @return error code (zero if success)
       */
      enum KFitError::ECode       setVertex(const HepPoint3D& v);
      /** Set a vertex error matrix of the mother particle.
       * @param e (3x3) vertex error matrix
       * @return error code (zero if success)
       */
      enum KFitError::ECode       setVertexError(const CLHEP::HepSymMatrix& e);
      /** Set a vertex error matrix of the child particle in the addTrack'ed order.
       * @param e (3x7) vertex error matrix
       * @return error code (zero if success)
       */
      enum KFitError::ECode       setTrackVertexError(const CLHEP::HepMatrix& e);
      /** Indicate no vertex uncertainty in the child particle in the addTrack'ed order.
       * @return error code (zero if success)
       */
      enum KFitError::ECode       setTrackZeroVertexError(void);
      /** Set a correlation matrix.
       * @param e (7x7) correlation matrix
       * @return error code (zero if success)
       */
      enum KFitError::ECode       setCorrelation(const CLHEP::HepMatrix& e);
      /** Indicate no correlation between tracks.
       * @return error code (zero if success)
       */
      enum KFitError::ECode       setZeroCorrelation(void);
      /** Set a flag to control computational point for the mother particle property at the decay point or not
       * @param flag true for yes, false for no
       * @return error code (zero if success)
       */
      enum KFitError::ECode       setFlagAtDecayPoint(const bool flag);
      /** Set a flag to control computational parameters for the mother particle property before or after the fit
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       * @return error code (zero if success)
       */
      enum KFitError::ECode       setFlagBeforeAfter(const int flag);

      /** Get a code of the last error.
       * @return error code
       */
      enum KFitError::ECode       getErrorCode(void) const;
      /** Get a track object of the mother particle.
       * @return track object of the mother particle
       */
      const KFitTrack             getMother(void) const;
      /** Get a Lorentz vector of the mother particle.
       * @return Lorentz vector of the mother particle
       */
      const CLHEP::HepLorentzVector      getMotherMomentum(void) const;
      /** Get a position of the mother particle.
       * @return position of the mother particle
       */
      const HepPoint3D            getMotherPosition(void) const;
      /** Get an error matrix of the mother particle.
       * @return error matrix of the mother particle
       */
      const CLHEP::HepSymMatrix          getMotherError(void) const;

      /** Perform a reconstruction of mother particle
       * @return error code (zero if success)
       */
      enum KFitError::ECode doMake(void);

    private:
      /** Make a matrix to calculate error matrix of the mother particle.
       * @param e error matrix container
       */
      void calculateError(CLHEP::HepSymMatrix* Ec) const;
      /** Make delMdelC to calculate error matrix of the mother particle.
       * @param e delMdelC container
       */
      void calculateDELMDELC(CLHEP::HepMatrix* e) const;


    private:
      /** Error code. */
      enum KFitError::ECode m_ErrorCode;

      /** Flag controlled by setFlagAtDecayPoint(). */
      bool     m_FlagAtDecayPoint;
      /** Flag controlled by setFlagBeforeAfter(). */
      int      m_FlagBeforeAfter;
      /** Flag to indicate if the vertex error matrix of the mother particle is preset. */
      bool     m_FlagVertexError;
      /** Flag to indicate if the correlation matrix among the child particles is preset. */
      bool     m_FlagCorrelation;
      /** Flag to indicate if the vertex error matrix of the child particle is preset. */
      bool     m_FlagTrackVertexError;

      /** Magnetic field. */
      double   m_MagneticField;
      /** Number of tracks. */
      int      m_TrackCount;


      /** Array of track objects of the child particles. */
      std::vector<KFitTrack> m_Tracks;
      /** Array of vertex error matrices of the child particles. */
      std::vector<CLHEP::HepMatrix> m_TrackVertexError;
      /** Array of correlation matrices among the child particles. */
      std::vector<CLHEP::HepMatrix> m_Correlation;


      /** Vertex position of the mother particle. */
      CLHEP::Hep3Vector   m_Vertex;
      /** Vertex error matrix of the mother particle. */
      CLHEP::HepSymMatrix m_VertexError;


      /** Charge of the mother particle. */
      double    m_Charge;
      /** Track object of the mother particle. */
      KFitTrack m_Mother;
    };

  } // namespace analysis

} // namespace Belle2

#endif /* MAKEMOTHERKFIT_H */

