/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 *                                                                        *
 * 7/31: Poyuan                                                           *
 *                                                                        *
 *                                                                        *
 **************************************************************************/
#ifndef TECLCFSHOWER_H
#define TECLCFSHOWER_H

#include <map>
#include <vector>
#include <fstream>
#include <functional>
#include <ecl/geometry/ECLGeometryPar.h>

#include "TVector3.h"
#include "TMatrix.h"
#include  "CLHEP/Matrix/Matrix.h"
#include  "CLHEP/Matrix/SymMatrix.h"
namespace Belle2 {
  namespace ECL {

    /** MEclCFShower Hit Assignment  class  */
    class MEclCFShowerHA {
      friend class TRecEclCF;
      friend class TEclCFCR;
      friend class TEclCFShower;
    public:
      /** define EclIdentifier is Identifier(int) */
      typedef EclIdentifier Identifier;

      /** construct MEclCFShowerHA */
      MEclCFShowerHA() {
        fId = 0;
        fFraction = 0;
      }
      ///
      virtual ~MEclCFShowerHA() {}
      /** construct MEclCFShowerHA with MEclCFShowerHA& a */
      MEclCFShowerHA(const MEclCFShowerHA& a)
        : fId(a.fId), fFraction(a.fFraction) {}
      /** define operator= for  MEclCFShowerHA */
      MEclCFShowerHA& operator=(const MEclCFShowerHA& rhs) {
        if (this != &rhs) {
          fId = rhs.fId;
          fFraction = rhs.fFraction;
        }
        return *this;
      }
      /** construct MEclCFShowerHA with  fId(id), fFraction(w) */
      MEclCFShowerHA(int id, double w): fId(id), fFraction(w) {}
      /** Get fId */
      Identifier Id(void) const {
        return fId;
      }
      /** Get fFraction */
      double Fraction(void) const {
        return fFraction;
      }
    private:
      // not HitId but cellId!
      /** fid     */
      Identifier fId;
      /** fFraction     */
      double fFraction;
    };



    /**Calorimeter cluster
     */
    class TEclCFShower {
      /// friend classes and functions
      friend class TRecEclCF;
      ///
      friend class TEclCFCR;


    public:
      // constants, enums and typedefs
      /** define EclIdentifier is Identifier(int) */
      typedef EclIdentifier Identifier;
      /// backward compat.
      // no GARBAGE anymore
      // in fact, the proper meanings are
      // 0 = unknown,
      // 10 = isolated,
      // 20 = halo shared,
      // 30 = halo-core overlap,
      // 40 = core shared
      enum EGrade {
        UNKNOWN = 0, LONE = 10, ISOLATED = 20, SEPARATED = 30, MERGED = 40
      };


      // Constructors and destructor
      /** construct TEclCFShower with  fId(id) */
      TEclCFShower(int id = 0) : fId(id) {
        fEnergy = 0.0;  //gcc does not initialize...
        fE3x3 = 0.0;
        fE5x5 = 0.0;
        fE3x3unf = 0.0;
        fE5x5unf = 0.0;

        fStatus = 0;
        fGrade = UNKNOWN;
//   fError = TMatrix(3,1);
        fError = CLHEP::HepSymMatrix(3, 1);
      }
      ///
      virtual ~TEclCFShower() {
      }

      /** construct TEclCFShower with aShower */
      TEclCFShower(const TEclCFShower& aShower)
        : fId(aShower.fId)
        , fEnergy(aShower.fEnergy)
        , fTheta(aShower.fTheta)
        , fPhi(aShower.fPhi)
        , fDistance(aShower.fDistance)
        , fError(aShower.fError)
        , fMass(aShower.fMass)
        , fWidth(aShower.fWidth)

//   , fE9oE25(aShower.fE9oE25)
//   , fE9oE25unf(aShower.fE9oE25unf)
        , fE3x3(aShower.fE3x3)
        , fE5x5(aShower.fE5x5)
        , fE3x3unf(aShower.fE3x3unf)
        , fE5x5unf(aShower.fE5x5unf)
//       , fNHits(aShower.fNHits)
        , fNHitsUsed(aShower.fNHitsUsed)

        , fStatus(aShower.fStatus)
        , fGrade(aShower.fGrade)
        , fHA(aShower.fHA) {
      }

      /// assignment operator(s)
      TEclCFShower& operator=(const TEclCFShower& rhs) {
        if (this != &rhs) {
          fId = rhs.fId;
          fEnergy = rhs.fEnergy;
          fTheta = rhs.fTheta;
          fPhi = rhs.fPhi;
          fDistance = rhs.fDistance;
          fError = rhs.fError;
          fMass = rhs.fMass;
          fWidth = rhs.fWidth;
          fE3x3 = rhs.fE3x3;
          fE5x5 = rhs.fE5x5;
          fE3x3unf = rhs.fE3x3unf;
          fE5x5unf = rhs.fE5x5unf;
          fNHitsUsed = rhs.fNHitsUsed;

          fStatus = rhs.fStatus;
          fGrade = rhs.fGrade;
          fHA = rhs.fHA;
        }
        return *this;
      }

      /// comparison operators
      bool operator==(const TEclCFShower& rhs) const {
        return
          fId == rhs.fId;
      }
      /// comparison operators
      bool operator!=(const TEclCFShower& rhs) const {
        return
          fId != rhs.fId;
      }
      /// comparison operators
      bool operator<(const TEclCFShower& rhs) const {
        return
          fId < rhs.fId;
      }


      ///member functions Accumulate  from belle Library
      void Accumulate(Identifier cId, EclGeV energy, double w = 1.0) {
        fEnergy += (energy * w);
        Assign(cId, w);
      }
      ///member functions Assign  from belle Library
      void Assign(Identifier cId, double w = 1.0) {
        fHA.push_back(MEclCFShowerHA(cId, w));
      }

//      void UncAttributes(void);

      /** currently only getters are implemented... */
      Identifier Id(void) const {
        return
          fId;
      }
      ///get fEnergy
      EclGeV Energy(void) const {
        return
          fEnergy;
      }
      /// correct runtime with vertex info?
      EclRad Theta(void) const {
        return
          fTheta;
      }
      /// get fPhi
      EclRad Phi(void) const {
        return
          fPhi;
      }
      ///get fDistance
      EclCM Distance(void) const {
        return
          fDistance;
      }
      ///get fError
      const CLHEP::HepSymMatrix& Error(void) const {
        return
          fError;
      }

      ///get Mass
      EclGeV Mass(void) const {
        return
          fMass;
      }
      ///get Width
      EclCM  Width(void) const {
        return
          fWidth;
      }
      ///get  fE3x3 / fE5x5
      double E9oE25(void) const {
        return
          fE3x3 / fE5x5;
//      fE9oE25;
      }
      /// get E9oE25unf
      double E9oE25unf(void) const {
        return
          fE3x3unf / fE5x5unf;
//      fE9oE25unf;
      }
      /// get NHits
      int NHits(void) const {
        return
          fHA.size();
//      fNHits;
      }
      ///weighted #of hits
      double WNHits(void) const {
        double n = 0.0;
        for (std::vector<MEclCFShowerHA>::const_iterator i = fHA.begin();
             i != fHA.end(); ++i)
          n += i->fFraction;
        return n;
      }
      ///get NHitsUsed
      double NHitsUsed(void) const {
        return
          NHits();  // not implemented yet
//      fNHits;
      }
      ///get Status
      int Status(void) const {
        return
          fStatus;
      }
      /// backward compatibility
      EGrade Grade(void) const {
        return
          fGrade;
      }
      ///get Grade
      void Grade(EGrade g) {
        if (g > fGrade)
          fGrade = g;
      }

      /// don't use these, if you don't know...
      EclGeV UncEnergy(void) const {
        return
          fE5x5unf;
      }
      ///get TotEnergy
      EclGeV TotEnergy(void) const {
        return
          fE5x5;
      }
      ///get HitAssignment
      const std::vector<MEclCFShowerHA>& HitAssignment(void) const {
        return
          fHA;
      }

      // const member functions

      // static member functions

    protected:
      // protected member functions

      // protected const member functions

    private:

      // private member functions

      // private const member functions

      /** data members */
      Identifier fId;
      /// corrected energy of shower
      EclGeV fEnergy;
      /// correct runtime with vertex info?
      EclRad fTheta;
      /// correct runtime with vertex info?
      EclRad fPhi;

      /// from origin to shower(how to correct?), by center or front face?
      EclCM  fDistance;

      /// energy,theta,phi
//      TMatrix fError;
      CLHEP::HepSymMatrix fError;
//    CLHEP::HepMatrix fError;
      /// shower mass
      EclGeV fMass;
      /// rms shower width
      EclCM  fWidth;

//      double fE9oE25;
//      double fE9oE25unf;  // unfolded

      /** not true for the endcaps */
      EclGeV fE3x3;
      /** not true for the endcaps */
      EclGeV fE5x5;
      /** not true for the endcaps */
      EclGeV fE3x3unf;
      /** not true for the endcaps */
      EclGeV fE5x5unf;
//      int    fNHits;      // #crystals in shower
//    double fWNHits;  // weighted
      /** for highest N method, not impl. yet */
      double fNHitsUsed;

      /** shower status flag;  0 == O.K. */
      int    fStatus;

//      EclGeV fUncEnergy;  // E before final correc
      // if we move to highestN, it will be different
//      EclGeV fTotEnergy;  // E, no corr, incl All xtals(for highestN?)

      /** for backward compatibility */
      EGrade fGrade;

      /** member MEclCFShowerHA */
      std::vector<MEclCFShowerHA> fHA;

      // static data members

    };
    /** define EclCFShowerMap */
    typedef std::map<EclIdentifier, TEclCFShower, std::less <EclIdentifier> > EclCFShowerMap;
  }//ECL
} // end of namespace Belle2

#endif

