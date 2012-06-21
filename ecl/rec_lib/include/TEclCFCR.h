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
#ifndef TECLCFCR_H
#define TECLCFCR_H

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/rec_lib/TEclCFShower.h>
#include "TVector3.h"

namespace Belle2 {
  namespace ECL {
    class TEclCFCR {
    public:
      // constants, enums and typedefs
      /** typedef Identifier */
      typedef EclIdentifier Identifier;
      /** typedef MMap */
      typedef std::multimap <Identifier, Identifier, std::less <Identifier> > MMap;

      /** Constructors and destructor */
      TEclCFCR(int id = 0);
      /** Constructors */
      TEclCFCR(const TEclCFCR& rhs);
      virtual ~TEclCFCR();

      /// assignment operator(s)
      const TEclCFCR& operator=(const TEclCFCR&);

      /** comparison operators */
      bool operator==(const TEclCFCR&) const;
      /** comparison operators */
      bool operator!=(const TEclCFCR&) const;
      /** comparison operators */
      bool operator<(const TEclCFCR&) const;

      // member functions
      /** get Id */
      const Identifier Id() const {
        return fId;
      }
      /** get mass */
      const EclGeV Mass() const {
        return fMass;
      }

      /** get Energy */
      const EclGeV Energy() const {
        return
          fEnergy;
      }
      /** get Width */
      const double Width() const {
        return
          fWidth;
      }
      /** do  Accumulate */
      void Accumulate(TEclEnergyHit& ahit) {
        fEnergyHits[ahit.CellId()] = ahit;
      }

      /**find local maxima in a CR and prepare hit-relation tables.
       *local max(seed) == more energy than all near-neighbors.
       */
      int SearchLocalMaxima(EclGeV threshold);
      /**1st phase clustering,
       * only Energy(exclusive sum)/Grade will be done.
       * (*) zero suppression for a shower < 10MeV is not done...
       */
      int FormShowers(int startId);
      /**2nd phase clustering,
       * only Unfold Energy Hits(sharing).
       */
      void Unfold(void);

      /** only mass, energy for now.
       * cr mass is not naive invariant mass!  to relate mass to 2nd moment.
       * simple approx. e.g. <x/r> ~ <x>/Rcm.
       * anyway r,<r>,Rcm cannot be well-defined, need optimization.
       * ... think about staggered case, it'll bias position.
       */
      void UncAttributes(void);
      /** do Attributes */
      void Attributes(void);

      /** get Showers */
      const EclCFShowerMap& Showers(void) const {
        return
          fShowers;
      }
      /** get EnergyHits */
      const EclEnergyHitMap& EnergyHits(void) const {
        return
          fEnergyHits;
      }
      /** get Seeds */
      const EclEnergyHitMap& Seeds(void) const {
        return
          fSeeds;
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
      /** CR id */
      Identifier fId;
      /** CR mass */
      EclGeV fMass;
      /** CR Energy */
      EclGeV fEnergy;
      /** CR Width */
      double fWidth;
      /** accessed by cellId of Seed */
      EclCFShowerMap fShowers;

      /** partitions, accessed by CellId */
      EclEnergyHitMap fEnergyHits;
      /** accessed by CellId */
      EclEnergyHitMap fSeeds;

      /// not determined how to fuse
      EclEnergyHitMap fFreeHits;
      /// cell -> seed
      MMap fNext;
      /// cell -> seed
      MMap fNear;
      // static data members

    };
  }//ECL
} // end of namespace Belle2

#endif
