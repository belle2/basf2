/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: J. Tanaka                                        *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <CLHEP/Matrix/Matrix.h>
#include <CLHEP/Matrix/SymMatrix.h>
#include <CLHEP/Vector/LorentzVector.h>
#include <CLHEP/Geometry/Point3D.h>

#include <framework/logging/Logger.h>

#include <analysis/VertexFitting/KFit/KFitConst.h>

#ifndef ENABLE_BACKWARDS_COMPATIBILITY
typedef HepGeom::Point3D<double> HepPoint3D;
#endif


namespace Belle2 {

  namespace analysis {

    /**
     * KFitTrack is a container of the track information (Lorentz vector, position, and error matrix),
     * and a set of macro functions to set/get the contents.
     */
    class KFitTrack {
    public:
      /**
       * KFitPXE is a container of the track information (Lorentz vector, position, and error matrix).
       */
      struct KFitPXE {
        /** Lorentz vector of the track */
        CLHEP::HepLorentzVector   m_P;
        /** Position of the track */
        HepPoint3D         m_X;
        /** (7x7) error matrix of the track */
        CLHEP::HepSymMatrix       m_E;
      };

      /** Construct an object with no argument. */
      KFitTrack(void);
      /** Construct the object as a copy constructor. */
      KFitTrack(const KFitTrack& kp);
      /** Construct the object with track properties.
       * @param p Lorentz vector of the track
       * @param x position of the track
       * @param e (7x7) error matrix of the track
       * @param q charge of the track
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       */
      KFitTrack(
        const CLHEP::HepLorentzVector& p,
        const HepPoint3D&       x,
        const CLHEP::HepSymMatrix&     e,
        const double           q,
        const int              flag = KFitConst::kBeforeFit
      );

      /** Destruct the object. */
      ~KFitTrack(void);


    public:
      /** Operator: assignment operator */
      KFitTrack& operator = (const KFitTrack&);


    public:
      /** Set a Lorentz vector of the track.
       * @param p Lorentz vector of the track
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       */
      void setMomentum(const CLHEP::HepLorentzVector& p, const int flag = KFitConst::kBeforeFit);
      /** Set a position of the track.
       * @param x position of the track
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       */
      void setPosition(const HepPoint3D& x,       const int flag = KFitConst::kBeforeFit);
      /** Set an error matrix of the track.
       * @param e error matrix of the track
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       */
      void setError(const CLHEP::HepSymMatrix& e,     const int flag = KFitConst::kBeforeFit);
      /** Set a charge of the track.
       * @param q charge of the track
       */
      void setCharge(const double q);
      /** Set a vertex position associated to the track.
       * @param v vertex position associated to the track
       */
      void setVertex(const HepPoint3D& v);
      /** Set a vertex error matrix associated to the track.
       * @param ve vertex error matrix associated to the track
       */
      void setVertexError(const CLHEP::HepSymMatrix& ve);

      /** Get a Lorentz vector of the track.
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       * @return Lorentz vector of the track
       */
      const CLHEP::HepLorentzVector getMomentum(const int flag = KFitConst::kAfterFit) const;
      /** Get a position of the track.
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       * @return position of the track
       */
      const HepPoint3D       getPosition(const int flag = KFitConst::kAfterFit) const;
      /** Get an error matrix of the track.
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       * @return error matrix of the track
       */
      const CLHEP::HepSymMatrix     getError(const int flag = KFitConst::kAfterFit) const;
      /** Get a charge of the track.
       */
      double                 getCharge(void) const;
      /** Get a mass of the track.
       */
      double                 getMass(void) const;
      /** Get a vertex position associated to the track.
       */
      const HepPoint3D       getVertex(void) const;
      /** Get a vertex error matrix associated to the track.
       */
      const CLHEP::HepSymMatrix     getVertexError(void) const;

      /** Get a parameter of the track.  Not intended for end user's use.
       * @param which (0,1,2,3,4,5) = (Px,Py,Pz,Xx,Xy,Xz)
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       * @return corresponding parameter
       */
      double                 getFitParameter(const int which, const int flag) const;
      /** Get a parameter set of the track.  Not intended for end user's use.
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       * @return array of getFitParameter(0,flag) ... getFitParameter(5,flag)
       */
      const CLHEP::HepMatrix        getFitParameter(const int flag) const;
      /** Get an error matrix of the track.  Not intended for end user's use.
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       * @return fitted error matrix
       */
      const CLHEP::HepSymMatrix     getFitError(const int flag) const;
      /** Get a combination of Lorentz vector and position of the track.  Not intended for end user's use.
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       * @return combination of Lorentz vector and position
       */
      const CLHEP::HepMatrix        getMomPos(const int flag) const;


    private:
      /** Check if the flag is one of KFitConst::kBeforeFit or KFitConst::kAfterFit.  If check fails, abort the program.
       * @param flag
       */
      inline void checkFlag(const int flag) const
      {
        if (flag != KFitConst::kBeforeFit && flag != KFitConst::kAfterFit) B2FATAL("checkFlag");
      }

      /** Check if the matrix size is intended one.  If check fails, abort the program.
       * @param m matrix to be tested
       * @param dim size of the matrix
       */
      inline void checkMatrixDimension(const CLHEP::HepSymMatrix& m, const int dim) const
      {
        if (m.num_row() != dim) B2FATAL("checkMatrixDimension");
      }


    private:
      /** Lorentz vector, position, and error matrix of the track before the fit. */
      struct KFitPXE m_PXEBefore;

      /** Lorentz vector, position, and error matrix of the track after the fit. */
      struct KFitPXE m_PXEAfter;

      /** Charge of the track. */
      double         m_Charge;
      /** Mass of the track. */
      double         m_Mass;

      /** Vertex position associated to the track. */
      HepPoint3D     m_Vertex;
      /** Vertex error matrix associated to the track. */
      CLHEP::HepSymMatrix   m_VertexError;
    };

  } // namespace analysis
} // namespace Belle2
