/**************************************************************************
 * Copyright(C) 1997 - J. Tanaka                                          *
 *                                                                        *
 * Author: J. Tanaka                                                      *
 * Contributor: J. Tanaka and                                             *
 *              conversion to Belle II structure by T. Higuchi            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef KFITTRACK_H
#define KFITTRACK_H


#include <iostream>

#include <CLHEP/Matrix/Matrix.h>
#include <CLHEP/Matrix/SymMatrix.h>
#include <CLHEP/Vector/ThreeVector.h>
#include <CLHEP/Vector/LorentzVector.h>
#include <CLHEP/Geometry/Point3D.h>

#include <framework/logging/Logger.h>

#include <analysis/KFit/KFitConst.h>


using namespace std;
using namespace CLHEP;
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
        HepLorentzVector   m_P;
        /** Position of the track */
        HepPoint3D         m_X;
        /** (7x7) error matrix of the track */
        HepSymMatrix       m_E;
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
        const HepLorentzVector& p,
        const HepPoint3D&       x,
        const HepSymMatrix&     e,
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
      void setMomentum(const HepLorentzVector& p, const int flag = KFitConst::kBeforeFit);
      /** Set a position of the track.
       * @param x position of the track
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       */
      void setPosition(const HepPoint3D& x,       const int flag = KFitConst::kBeforeFit);
      /** Set an error matrix of the track.
       * @param e error matrix of the track
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       */
      void setError(const HepSymMatrix& e,     const int flag = KFitConst::kBeforeFit);
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
      void setVertexError(const HepSymMatrix& ve);

      /** Get a Lorentz vector of the track.
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       * @return Lorentz vector of the track
       */
      const HepLorentzVector getMomentum(const int flag = KFitConst::kAfterFit) const;
      /** Get a position of the track.
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       * @return position of the track
       */
      const HepPoint3D       getPosition(const int flag = KFitConst::kAfterFit) const;
      /** Get an error matrix of the track.
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       * @return error matrix of the track
       */
      const HepSymMatrix     getError(const int flag = KFitConst::kAfterFit) const;
      /** Get a charge of the track.
       */
      const double           getCharge(void) const;
      /** Get a mass of the track.
       */
      const double           getMass(void) const;
      /** Get a vertex position associated to the track.
       */
      const HepPoint3D       getVertex(void) const;
      /** Get a vertex error matrix associated to the track.
       */
      const HepSymMatrix     getVertexError(void) const;

      /** Get a parameter of the track.  Not intended for end user's use.
       * @param which (0,1,2,3,4,5) = (Px,Py,Pz,Xx,Xy,Xz)
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       * @return corresponding parameter
       */
      const double           getFitParameter(const int which, const int flag) const;
      /** Get a parameter set of the track.  Not intended for end user's use.
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       * @return array of getFitParameter(0,flag) ... getFitParameter(5,flag)
       */
      const HepMatrix        getFitParameter(const int flag) const;
      /** Get an error matrix of the track.  Not intended for end user's use.
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       * @return fitted error matrix
       */
      const HepSymMatrix     getFitError(const int flag) const;
      /** Get a combination of Lorentz vector and position of the track.  Not intended for end user's use.
       * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
       * @return combination of Lorentz vector and position
       */
      const HepMatrix        getMomPos(const int flag) const;


    private:
      /** Check if the flag is one of KFitConst::kBeforeFit or KFitConst::kAfterFit.  If check fails, abort the program.
       * @param flag
       */
      inline void checkFlag(const int flag) const {
        if (flag != KFitConst::kBeforeFit && flag != KFitConst::kAfterFit) B2FATAL("checkFlag");
      }

      /** Check if the matrix size is intended one.  If check fails, abort the program.
       * @param m matrix to be tested
       * @param dim size of the matrix
       */
      inline void checkMatrixDimension(const HepSymMatrix& m, const int dim) const {
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
      HepSymMatrix   m_VertexError;
    };

  } // namespace analysis

} // namespace Belle2

#endif /* KFITTRACK_H */

