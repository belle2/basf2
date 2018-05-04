/**************************************************************************
 * Copyright(C) 1997 - J. Tanaka                                          *
 *                                                                        *
 * Author: J. Tanaka                                                      *
 * Contributor: J. Tanaka and                                             *
 *              conversion to Belle II structure by T. Higuchi            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef KFITBASE_H
#define KFITBASE_H


#include <iostream>
#include <vector>

#include <CLHEP/Matrix/Matrix.h>
#include <CLHEP/Matrix/SymMatrix.h>
#include <CLHEP/Vector/ThreeVector.h>
#include <CLHEP/Vector/LorentzVector.h>
#include <CLHEP/Geometry/Point3D.h>

#include <framework/logging/Logger.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/KFit/KFitConst.h>
#include <analysis/KFit/KFitError.h>
#include <analysis/KFit/KFitTrack.h>


#ifndef ENABLE_BACKWARDS_COMPATIBILITY
typedef HepGeom::Point3D<double> HepPoint3D;
#endif


namespace Belle2 {

  namespace analysis {

    /**
     * KFitBase is a base class for kinematical fitters.
     */
    class KFitBase {
    public:
      /** Construct an object with no argument. */
      KFitBase(void);
      /** Destruct the object. */
      virtual ~KFitBase(void);


    public:
      /** Add a track to the fitter object.
       * @param kp object of the track
       * @return error code (zero if success)
       */
      enum KFitError::ECode         addTrack(const KFitTrack& kp);
      /** Add a track to the fitter object with specifying its momentum, position, error matrix, and charge.
       *  This function internally calls addTrack(const KFitTrack &kp).
       * @param p Lorentz vector of the track
       * @param x position of the track
       * @param e (7x7) error matrix of the track
       * @param q charge of the track
       * @return error code (zero if success)
       */
      enum KFitError::ECode               addTrack(const CLHEP::HepLorentzVector& p, const HepPoint3D& x, const CLHEP::HepSymMatrix& e,
                                                   const double q);
      /** Add a particle to the fitter.
       * The function gets track parameters from the Particle dataobject and
       * calls addTrack().
       * @param[in] particle Particle.
       * @return error code (zero if success)
       */
      enum KFitError::ECode addParticle(const Particle* particle);
      /** Set a correlation matrix.  Not intended for end user's use.
       * @param c (7x7) correlation matrix
       * @return error code (zero if success)
       */
      virtual enum KFitError::ECode       setCorrelation(const CLHEP::HepMatrix& c);
      /** Indicate no correlation between tracks.  Not intended for end user's use.
       * @return error code (zero if success)
       */
      virtual enum KFitError::ECode       setZeroCorrelation(void);
      /** Change a magnetic field from the default value KFitConst::kDefaultMagneticField.
       * @param mf magnetic field to set
       * @return error code (zero if success)
       */
      enum KFitError::ECode               setMagneticField(const double mf);


      /** Get a code of the last error.
       * @return the last error code
       */
      enum KFitError::ECode               getErrorCode(void) const;
      /** Get the number of added tracks.
       * @return the number of added tracks
       */
      int                                 getTrackCount(void) const;
      /** Get an NDF of the fit.
       * @return NDF of the fit
       */
      virtual int                         getNDF(void) const;
      /** Get a chi-square of the fit.
       * @return chi-square of the fit
       */
      virtual double                      getCHIsq(void) const;
      /** Get a magnetic field.
       * @return magnetic field
       */
      double                              getMagneticField(void) const;
      /** Get a chi-square of the track.
       * @param id track id
       * @return chi-square of the track
       */
      virtual double                      getTrackCHIsq(const int id) const;
      /** Get a Lorentz vector of the track.
       * @param id track id
       * @return Lorentz vector of the track
       */
      const CLHEP::HepLorentzVector              getTrackMomentum(const int id) const;
      /** Get a position of the track.
       * @param id track id
       * @return position of the track
       */
      const HepPoint3D                    getTrackPosition(const int id) const;
      /** Get an error matrix of the track.
       * @param id track id
       * @return error matrix of the track
       */
      const CLHEP::HepSymMatrix                  getTrackError(const int id) const;
      /** Get a specified track object.
       * @param id track id
       * @return specified track object
       */
      const KFitTrack                     getTrack(const int id) const;
      /** Get a correlation matrix between two tracks.
       * @param id1 first track id
       * @param id2 second track id
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       * @return (7x7) correlation matrix
       */
      virtual const CLHEP::HepMatrix             getCorrelation(const int id1, const int id2,
                                                                const int flag = KFitConst::kAfterFit) const;


    public:
      /** Return false if fit is not performed yet or performed fit is failed; otherwise true.
       * @return see description
       */
      bool isFitted(void) const;


    protected:
      /** Rebuild an error matrix from a Lorentz vector and an error matrix.
       * @param p Lorentz vector
       * @param e (6x6) error matrix
       * @return (7x7) error matrix
       */
      const CLHEP::HepSymMatrix makeError1(const CLHEP::HepLorentzVector& p, const CLHEP::HepMatrix& e) const;
      /** Rebuild an error matrix from a pair of Lorentz vectors and an error matrix.
       * @param p1 first Lorentz vector
       * @param p2 second Lorentz vector
       * @param e (6x6) error matrix
       * @return (7x7) error matrix
       */
      const CLHEP::HepMatrix    makeError1(const CLHEP::HepLorentzVector& p1, const CLHEP::HepLorentzVector& p2,
                                           const CLHEP::HepMatrix& e) const;
      /** Rebuild an error matrix from a Lorentz vector and an error matrix.
       * @param p Lorentz vector
       * @param e (3x6) error matrix
       * @return (3x7) error matrix
       */
      const CLHEP::HepMatrix    makeError2(const CLHEP::HepLorentzVector& p, const CLHEP::HepMatrix& e) const;
      /** Rebuild an error matrix from a Lorentz vector and an error matrix.
       * @param p Lorentz vector
       * @param e (7x7) error matrix
       * @param is_fix_mass true to recalculate energy term from other parameters, false to do nothing
       * @return (7x7) error matrix
       */
      const CLHEP::HepSymMatrix makeError3(const CLHEP::HepLorentzVector& p, const CLHEP::HepMatrix& e, const bool is_fix_mass) const;
      /** Rebuild an error matrix from a pair of Lorentz vectors and an error matrix.
       * @param p1 first Lorentz vector
       * @param p2 second Lorentz vector
       * @param e (7x7) error matrix
       * @param is_fix_mass true to recalculate energy term from other parameters, false to do nothing
       * @return (7x7) error matrix
       */
      const CLHEP::HepMatrix    makeError3(const CLHEP::HepLorentzVector& p1, const CLHEP::HepLorentzVector& p2,
                                           const CLHEP::HepMatrix& e, const bool is_fix_mass1, const bool is_fix_mass2) const;
      /** Rebuild an error matrix from a Lorentz vector and an error matrix.
       * @param p Lorentz vector
       * @param e (3x7) error matrix
       * @return (7x7) error matrix
       */
      const CLHEP::HepMatrix    makeError4(const CLHEP::HepLorentzVector& p, const CLHEP::HepMatrix& e) const;


      /** Build grand matrices for minimum search from input-track properties.
       * @return error code (zero if success)
       */
      virtual enum KFitError::ECode prepareInputMatrix(void)    = 0;
      /** Build sub-matrices for minimum search from input-track properties.
       * @return error code (zero if success)
       */
      virtual enum KFitError::ECode prepareInputSubMatrix(void) = 0;
      /** Build a grand correlation matrix from input-track properties.
       * @return error code (zero if success)
       */
      virtual enum KFitError::ECode prepareCorrelation(void);
      /** Build an output error matrix.
       * @return error code (zero if success)
       */
      virtual enum KFitError::ECode prepareOutputMatrix(void)   = 0;
      /** Build matrices using the kinematical constraint.
       * @return error code (zero if success)
       */
      virtual enum KFitError::ECode makeCoreMatrix(void)        = 0;
      /** Calculate an NDF of the fit.
       * @return error code (zero if success)
       */
      virtual enum KFitError::ECode calculateNDF(void)          = 0;


    protected:
      /** Perform a fit (used in MassFitKFit::doFit()).
       * @return error code (zero if success)
       */
      enum KFitError::ECode doFit1(void);
      /** Perform a fit (used in VertexFitKFit::doFit() and MassVertexFitKFit::doFit()).
       * @return error code (zero if success)
       */
      enum KFitError::ECode doFit2(void);


    protected:
      /** Check if the id is in the range.
       * @param id track id
       * @return true if the id is in the range, false otherwise
       */
      bool isTrackIDInRange(const int id) const;
      /** Check if the energy is non-zero.
       * @param p Lorentz vector
       * @return true for non-zero energy, false for otherwise
       */
      bool isNonZeroEnergy(const CLHEP::HepLorentzVector& p) const;


      /** Error code. */
      enum KFitError::ECode m_ErrorCode;
      /** Flag to indicate if the fit is performed and succeeded. */
      bool                  m_FlagFitted;


      /** Container of input tracks. */
      std::vector<KFitTrack> m_Tracks;
      /** Container of input correlation matrices. */
      std::vector<CLHEP::HepMatrix> m_BeforeCorrelation;


      /** See J.Tanaka Ph.D (2001) p137 for definition. */
      CLHEP::HepSymMatrix m_V_al_0;
      /** See J.Tanaka Ph.D (2001) p136 for definition. */
      CLHEP::HepMatrix    m_al_0;
      /** See J.Tanaka Ph.D (2001) p136 for definition. */
      CLHEP::HepMatrix    m_al_1;
      /** See J.Tanaka Ph.D (2001) p137 for definition. */
      CLHEP::HepMatrix    m_al_a;
      /** Container of charges and masses. */
      CLHEP::HepMatrix    m_property;

      /** See J.Tanaka Ph.D (2001) p137 for definition. */
      CLHEP::HepMatrix    m_D;
      /** See J.Tanaka Ph.D (2001) p137 for definition. */
      CLHEP::HepMatrix    m_d;

      /** See J.Tanaka Ph.D (2001) p138 for definition. */
      CLHEP::HepMatrix    m_V_D;

      /** See J.Tanaka Ph.D (2001) p138 for definition. */
      CLHEP::HepMatrix    m_V_al_1;
      /** See J.Tanaka Ph.D (2001) p137 for definition. */
      CLHEP::HepMatrix    m_lam;

      /** See J.Tanaka Ph.D (2001) p137 for definition. */
      CLHEP::HepMatrix    m_E;
      /** See J.Tanaka Ph.D (2001) p138 for definition. */
      CLHEP::HepMatrix    m_V_E;
      /** See J.Tanaka Ph.D (2001) p138 for definition. */
      CLHEP::HepMatrix    m_lam0;
      /** See J.Tanaka Ph.D (2001) p137 for definition. */
      CLHEP::HepMatrix    m_v;
      /** See J.Tanaka Ph.D (2001) p137 for definition. */
      CLHEP::HepMatrix    m_v_a;
      /** See J.Tanaka Ph.D (2001) p138 for definition. */
      CLHEP::HepMatrix    m_V_Dt;
      /** See J.Tanaka Ph.D (2001) p137 for definition. */
      CLHEP::HepMatrix    m_Cov_v_al_1;


      /** NDF of the fit. */
      int      m_NDF;
      /** chi-square of the fit. */
      double   m_CHIsq;


      /** Number of tracks. */
      int      m_TrackCount;
      /** Number needed tracks to perform fit. */
      int      m_NecessaryTrackCount;

      /** Flag whether a correlation among tracks exists. */
      bool     m_FlagCorrelation;
      /** Flag whether the iteration count exceeds the limit. */
      bool     m_FlagOverIteration;

      /** Magnetic field. */
      double   m_MagneticField;
    };

  } // namespace analysis

} // namespace Belle2

#endif /* B2KFIT_BASE_H */

