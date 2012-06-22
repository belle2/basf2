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
#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/rec_lib/TEclCFShower.h>
#include <ecl/rec_lib/TRecEclCFParameters.h>
#include "TVector3.h"


namespace Belle2 {
  namespace ECL {
    /** TRecEclCF  class  */
    class TRecEclCF {
      // friend classes and functions

    public:
      // constants, enums and typedefs
      /** typedef Identifier */
      typedef EclIdentifier Identifier;

      // Constructors and destructor
      virtual ~TRecEclCF();

      // member functions
      /** Clear */
      void Clear(void);
      /** with zero suppression */
      void Accumulate(const int hid, const float energy, const int cid);
      /** with zero suppression */
      void Accumulate(const TEclEnergyHit& ahit) ;
      // helper
      /// cellId -> HitId, for panther mainly
      Identifier HitId(Identifier cId) const;
      /** SearchCRs */
      int SearchCRs(void);

      /** do Attributes */
      void Attributes(void);
      ///
//      void EACorr(TEclCFShower& s);

      /* for new correction on 2k.sep.22nd */
      /* comment for Basf2 unused warning
          void NewEACorr(TEclCFShower& s);
      */
      /* end of for new correction */

      // const member functions
//      const vector<TEclEnergyHit>& EnergyHits(void) const;
      /** get EnergyHits */
      const EclEnergyHitMap& EnergyHits(void) const;
      /** get CRs */
      const std::vector<TEclCFCR>& CRs(void) const;


      // static member functions
      /**  TRecEclCF& Instance */
      static TRecEclCF& Instance(void);
      /** Check Instance */
      static bool IsInstance(void);
      /** Kill */
      static void Kill(void);


    public:
      /// backdoor, compromise with basf define_param
      TRecEclCFParameters fParameters;
    protected:
      /// protected member functions
      TRecEclCF();

      // protected const member functions

    private:
      /** Constructors and destructor
         TRecEclCF(); */
      TRecEclCF(const TRecEclCF&);

      /** assignment operator(s) */
      const TRecEclCF& operator=(const TRecEclCF&);

      /** comparison operators */
      bool operator==(const TRecEclCF&) const;
      /** comparison operators */
      bool operator!=(const TRecEclCF&) const;

      // private member functions

      // private const member functions

      // data members
//      vector <TEclEnergyHit> fEnergyHits;
      /** fEnergyHits  */
      EclEnergyHitMap fEnergyHits;  // can be acessed by CellId
      /** CRs  */
      std::vector <TEclCFCR> fCRs;

      /** static data members */
      static TRecEclCF* fgInstance;

    private:
      /* for new correction
         will not be used any longer
         actually moved to TComEclGQ class
      */
    };

// inline function definitions


  }//ECL
} // end of namespace Belle2

#endif
