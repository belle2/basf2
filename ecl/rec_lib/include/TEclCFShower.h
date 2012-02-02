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
#include <ecl/geoecl/ECLGeometryPar.h>

#include "TVector3.h"
#include "TMatrix.h"
#include "CLHEP/Matrix/SymMatrix.h"

namespace Belle2 {


  class MEclCFShowerHA {
    friend class TRecEclCF;
    friend class TEclCFCR;
    friend class TEclCFShower;
  public:
    ///
    typedef EclIdentifier Identifier;

    ///
    MEclCFShowerHA() {}
    ///
    virtual ~MEclCFShowerHA() {}
    ///
    MEclCFShowerHA(const MEclCFShowerHA& a)
      : fId(a.fId), fFraction(a.fFraction) {}
    ///
    const MEclCFShowerHA& operator=(const MEclCFShowerHA& rhs) {
      if (this != &rhs) {
        fId = rhs.fId;
        fFraction = rhs.fFraction;
      }
      return *this;
    }
    ///
    MEclCFShowerHA(int id, double w): fId(id), fFraction(w) {}
    ///
    const Identifier Id(void) const {
      return fId;
    }
    ///
    const double Fraction(void) const {
      return fFraction;
    }
  private:
    // not HitId but cellId!
    Identifier fId;
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
    ///
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
    ///
    TEclCFShower(int id = 0) : fId(id) {
      fEnergy = 0.0;  //gcc does not initialize...
      fE3x3 = 0.0;
      fE5x5 = 0.0;
      fE3x3unf = 0.0;
      fE5x5unf = 0.0;

      fStatus = 0;
      fGrade = UNKNOWN;
//   fError = TMatrix(3,1);
    }
    ///
    virtual ~TEclCFShower() {
    }

    ///
    TEclCFShower(const TEclCFShower& aShower)
      : fId(aShower.fId)
      , fEnergy(aShower.fEnergy)
      , fTheta(aShower.fTheta)
      , fPhi(aShower.fPhi)
      , fDistance(aShower.fDistance)
//   , fError(aShower.fError)
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
    const TEclCFShower& operator=(const TEclCFShower& rhs) {
      if (this != &rhs) {
        fId = rhs.fId;
        fEnergy = rhs.fEnergy;
        fTheta = rhs.fTheta;
        fPhi = rhs.fPhi;
        fDistance = rhs.fDistance;
//poyuan    fError = rhs.fError;
        fMass = rhs.fMass;
        fWidth = rhs.fWidth;
//      fE9oE25 = rhs.fE9oE25;
//      fE9oE25unf = rhs.fE9oE25unf;
        fE3x3 = rhs.fE3x3;
        fE5x5 = rhs.fE5x5;
        fE3x3unf = rhs.fE3x3unf;
        fE5x5unf = rhs.fE5x5unf;
//      fNHits = rhs.fNHits;
//    double fWNHits;  // weighted
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
    ///
    bool operator!=(const TEclCFShower& rhs) const {
      return
        fId != rhs.fId;
    }
    ///
    bool operator<(const TEclCFShower& rhs) const {
      return
        fId < rhs.fId;
    }


    // member functions
    ///
    void Accumulate(Identifier cId, EclGeV energy, double w = 1.0) {
      fEnergy += (energy * w);
      Assign(cId, w);
    }
    ///
    void Assign(Identifier cId, double w = 1.0) {
      fHA.push_back(MEclCFShowerHA(cId, w));
    }

//      void UncAttributes(void);

    /// currently only getters are implemented...
    const Identifier Id(void) const {
      return
        fId;
    }
    ///
    const EclGeV Energy(void) const {
      return
        fEnergy;
    }
    /// correct runtime with vertex info?
    const EclRad Theta(void) const {
      return
        fTheta;
    }
    ///
    const EclRad Phi(void) const {
      return
        fPhi;
    }
    ///
    const EclCM Distance(void) const {
      return
        fDistance;
    }
    ///
//      const TMatrix& Error(void) const {
//   return
//      fError;
//      }
    ///
    const EclGeV Mass(void) const {
      return
        fMass;
    }
    ///
    const EclCM  Width(void) const {
      return
        fWidth;
    }
    ///
    const double E9oE25(void) const {
      return
        fE3x3 / fE5x5;
//      fE9oE25;
    }
    ///
    const double E9oE25unf(void) const {
      return
        fE3x3unf / fE5x5unf;
//      fE9oE25unf;
    }
    ///
    const int NHits(void) const {
      return
        fHA.size();
//      fNHits;
    }
    ///weighted #of hits
    const double WNHits(void) const {
      double n = 0.0;
      for (std::vector<MEclCFShowerHA>::const_iterator i = fHA.begin();
           i != fHA.end(); ++i)
        n += i->fFraction;
      return n;
    }
    ///
    const double NHitsUsed(void) const {
      return
        NHits();  // not implemented yet
//      fNHits;
    }
    ///
    const int Status(void) const {
      return
        fStatus;
    }
    /// backward compatibility
    const EGrade Grade(void) const {
      return
        fGrade;
    }
    ///
    void Grade(EGrade g) {
      if (g > fGrade)
        fGrade = g;
    }

    /// don't use these, if you don't know...
    const EclGeV UncEnergy(void) const {
      return
        fE5x5unf;
    }
    ///
    const EclGeV TotEnergy(void) const {
      return
        fE5x5;
    }
    ///
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

    // data members
    ///
    Identifier fId;
    /// corrected energy of shower
    EclGeV fEnergy;
    /// correct runtime with vertex info?
    EclRad fTheta;
    EclRad fPhi;

    /// from origin to shower(how to correct?), by center or front face?
    EclCM  fDistance;

    /// energy,theta,phi
//      TMatrix fError;
    /// shower mass
    EclGeV fMass;
    /// rms shower width
    EclCM  fWidth;

//      double fE9oE25;
//      double fE9oE25unf;  // unfolded

    /// not true for the endcaps
    EclGeV fE3x3;
    ///
    EclGeV fE5x5;
    ///
    EclGeV fE3x3unf;
    ///
    EclGeV fE5x5unf;
//      int    fNHits;      // #crystals in shower
//    double fWNHits;  // weighted
    /// for highest N method, not impl. yet
    double fNHitsUsed;

    /// shower status flag;  0 == O.K.
    int    fStatus;

//      EclGeV fUncEnergy;  // E before final correc
    // if we move to highestN, it will be different
//      EclGeV fTotEnergy;  // E, no corr, incl All xtals(for highestN?)

    /// for backward compatibility
    EGrade fGrade;

    ///
    std::vector<MEclCFShowerHA> fHA;

    // static data members

  };

  typedef std::map<EclIdentifier, TEclCFShower, std::less <EclIdentifier> > EclCFShowerMap;
} // end of namespace Belle2

#endif

