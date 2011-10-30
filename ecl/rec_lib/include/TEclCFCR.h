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
#include <ecl/geoecl/ECLGeometryPar.h>
#include <ecl/rec_lib/TEclCFShower.h>
#include "TVector3.h"


namespace Belle2 {
  class TEclCFCR {
  public:
    // constants, enums and typedefs
    ///
    typedef EclIdentifier Identifier;
    ///
    typedef std::multimap <Identifier, Identifier, std::less <Identifier> > MMap;

    /// Constructors and destructor
    TEclCFCR(int id = 0);
    ///
    TEclCFCR(const TEclCFCR& rhs);
    virtual ~TEclCFCR();

    /// assignment operator(s)
    const TEclCFCR& operator=(const TEclCFCR&);

    /// comparison operators
    bool operator==(const TEclCFCR&) const;
    ///
    bool operator!=(const TEclCFCR&) const;
    ///
    bool operator<(const TEclCFCR&) const;

    // member functions
    ///
    const Identifier Id() const {
      return fId;
    }
    ///
    const EclGeV Mass() const {
      return fMass;
    }

    ///
    const EclGeV Energy() const {
      return
        fEnergy;
    }
    ///
    const double Width() const {
      return
        fWidth;
    }
    ///
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
    ///
    void Attributes(void);

    ///
    const EclCFShowerMap& Showers(void) const {
      return
        fShowers;
    }
    ///
    const EclEnergyHitMap& EnergyHits(void) const {
      return
        fEnergyHits;
    }
    ///
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
    /// CR id
    Identifier fId;
    ///
    EclGeV fMass;
    ///
    EclGeV fEnergy;
    ///
    double fWidth;
    /// accessed by cellId of Seed
    EclCFShowerMap fShowers;

    /// partitions, accessed by CellId
    EclEnergyHitMap fEnergyHits;
    /// accessed by CellId
    EclEnergyHitMap fSeeds;

    /// not determined how to fuse
    EclEnergyHitMap fFreeHits;
    /// cell -> seed
    MMap fNext;
    ///
    MMap fNear;
    // static data members

  };

} // end of namespace Belle2

#endif
