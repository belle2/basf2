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
#ifndef TRECECLCF_H
#define TRECECLCF_H

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <ecl/geoecl/ECLGeometryPar.h>
#include <ecl/rec_lib/TEclCFShower.h>
#include <ecl/rec_lib/TRecEclCFParameters.h>
#include "TVector3.h"


namespace Belle2 {

  class TRecEclCF {
    // friend classes and functions

  public:
    // constants, enums and typedefs
    ///
    typedef EclIdentifier Identifier;

    // Constructors and destructor
    virtual ~TRecEclCF();

    // member functions
    ///
    void Clear(void);
    /// with zero suppression
    void Accumulate(const int hid, const float energy, const int cid);
    ///
    void Accumulate(const TEclEnergyHit& ahit) ;
    // helper
    /// cellId -> HitId, for panther mainly
    const Identifier HitId(Identifier cId) const;
    ///
    int SearchCRs(void);

    ///
    void Attributes(void);
    ///
//      void EACorr(TEclCFShower& s);

    /* for new correction on 2k.sep.22nd */
    void NewEACorr(TEclCFShower& s);
    /* end of for new correction */

    // const member functions
//      const vector<TEclEnergyHit>& EnergyHits(void) const;
    ///
    const EclEnergyHitMap& EnergyHits(void) const;
    ///
    const std::vector<TEclCFCR>& CRs(void) const;


    // static member functions
    ///
    static TRecEclCF& Instance(void);
    ///
    static bool IsInstance(void);
    ///
    static void Kill(void);


  public:
    /// backdoor, compromise with basf define_param
    TRecEclCFParameters fParameters;
  protected:
    /// protected member functions
    TRecEclCF();

    // protected const member functions

  private:
    // Constructors and destructor
//      TRecEclCF();
    TRecEclCF(const TRecEclCF&);

    // assignment operator(s)
    const TRecEclCF& operator=(const TRecEclCF&);

    // comparison operators
    bool operator==(const TRecEclCF&) const;
    bool operator!=(const TRecEclCF&) const;

    // private member functions

    // private const member functions

    // data members
//      vector <TEclEnergyHit> fEnergyHits;
    EclEnergyHitMap fEnergyHits;  // can be acessed by CellId
    std::vector <TEclCFCR> fCRs;

    // static data members
    static TRecEclCF* fgInstance;

  private:
    /* for new correction
       will not be used any longer
       actually moved to TComEclGQ class
    */
  };

// inline function definitions



} // end of namespace Belle2

#endif
