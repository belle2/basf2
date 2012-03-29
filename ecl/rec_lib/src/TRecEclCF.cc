/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
// Implementation:
//     *  using cellId internally, hitId is only for PANTHER i/o
//
//     1. init parameters(cuts, eatables, sw)
//     2. accumulate hits
//     3. search CRs
//     4. loop over CRs
//        4-1  seeds search
//        4-2  hit assignments(shower recon)
//        4-3  initial shower attributes(e, position)
//     [] if corr then E/Angle corr
//     5. CR/Shower Attrs

#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/rec_lib/TEclCFCR.h>
#include <ecl/rec_lib/TRecEclCF.h>
#include <ecl/rec_lib/TRecEclCFParameters.h>
#include <ecl/rec_lib/TEclCFShower.h>
#include <ecl/rec_lib/TRecEclCF.h>

#include <cmath>
#include <boost/format.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <functional>
#include <utility>
//#include <stack>
#include <vector>
#include <map>

#include "TVector3.h"

using namespace std;
using namespace boost;
using namespace Belle2;


//
// constants, enums and typedefs
//

//
// static data member definitions
//
TRecEclCF* TRecEclCF::fgInstance = 0;

//
// constructors and destructor
//
TRecEclCF::TRecEclCF()
{
}

// TRecEclCF::TRecEclCF( const TRecEclCF& )
// {
// }

TRecEclCF::~TRecEclCF()
{
}

//
// assignment operators
//
// const TRecEclCF& TRecEclCF::operator=( const TRecEclCF& )
// {
// }

//
// comparison operators
//
// bool TRecEclCF::operator==( const TRecEclCF& ) const
// {
// }

// bool TRecEclCF::operator!=( const TRecEclCF& ) const
// {
// }

//
// member functions
//
void
TRecEclCF::Clear(void)
{
  fEnergyHits.erase(fEnergyHits.begin(), fEnergyHits.end());
  fCRs.erase(fCRs.begin(), fCRs.end());

}

void
TRecEclCF::Accumulate(const int hid, const float energy, const int cid)
{
  Accumulate(TEclEnergyHit(hid, energy, cid));
}

void
TRecEclCF::Accumulate(const TEclEnergyHit& ahit)
{

  if (fParameters.fEthX3 == 0) {  // old threshold method
    if (ahit.Energy() > fParameters.fEthHit)
      fEnergyHits[ahit.CellId()] = ahit;
  } else {                        // new theta dependent threshold
    int cid = ahit.CellId();
//        ECLGeometryPar geom;
    ECLGeometryPar* geom = ECLGeometryPar::Instance();
    geom->Mapping(cid);
    int ith = geom->GetThetaID();
    if (ahit.Energy() > fParameters.fgEthX3[ith] / 1000.)
      fEnergyHits[ahit.CellId()] = ahit;
  }
}
// helper

int
TRecEclCF::SearchCRs(void)
{
  int id = 0;
  int showers = 1;
//   const EclCalorimeterGeometry& cal (EclCalorimeterGeometry::instance());
  EclNbr tmpNbr;
  EclEnergyHitMap hitmap(fEnergyHits);  // destructive hit storeage

  while (!hitmap.empty()) { // hitmap.empty() != 0
    TEclEnergyHit aSeed = (*hitmap.begin()).second;
    hitmap.erase(hitmap.begin());

    std::vector <TEclEnergyHit> aStack;
    aStack.push_back(aSeed);
    fCRs.push_back(TEclCFCR(++id));

    while (!aStack.empty()) {
      TEclEnergyHit aHit = aStack.back();
      fCRs.back().Accumulate(aHit);
      aStack.pop_back();

      const EclNbr& aNbr(tmpNbr.getNbr(aHit.CellId()));
      for (std::vector<EclNbr::Identifier>::const_iterator
           iNbr = aNbr.nearBegin(); iNbr != aNbr.nearEnd(); ++iNbr) {
        EclEnergyHitMap::iterator found = hitmap.find(*iNbr);
        if (found != hitmap.end()) {
          aStack.push_back(hitmap[*iNbr]);
          hitmap.erase(found);
        }
      }
    }
    /// a CR found
    fCRs.back().SearchLocalMaxima(fParameters.fEthSeed);
    showers += fCRs.back().FormShowers(showers);
    fCRs.back().Unfold();
  }
  return fCRs.size();  /// return #Connected Regions
}


///___________________ SHOULD BE ReWRITTEN ____________________________________
void
TRecEclCF::Attributes(void)
/// need to take args for calib or track match ... for 2nd phase recon
{
  for (std::vector<TEclCFCR>::iterator iCR = fCRs.begin(); iCR != fCRs.end(); ++iCR) {
// all showers in one cr will be assigned uncorrected values
    iCR->UncAttributes();  // don't know how to correct yet

    /// Energy/Angle[r] corrections
    if (fParameters.fEnergyAngleCorrection != 0) {
      EclCFShowerMap tmpShower = iCR->Showers();
//   for (EclCFShowerMap::iterator iShower = iCR->fShowers.begin();
//        iShower != iCR->fShowers.end(); ++iShower) {
      for (EclCFShowerMap::iterator iShower = tmpShower.begin();
           iShower != tmpShower.end(); ++iShower) {
        TEclCFShower& s = iShower->second;
///for new correction 2k.sep.22nd
        //EACorr(s);
        //NewEACorr(s);
      }
    }
// this is dummy member function 2k.feb.22
    iCR->Attributes();     // final attributes(even in uncorrected case)
  }
}


/* for new correction 2k.sep.22nd */
/* comment for Basf2 unused warning
void
TRecEclCF::
NewEACorr(TEclCFShower& s)
{
}
*/
//
// const member functions
//
const EclIdentifier
TRecEclCF::HitId(Identifier cId) const
{
  const EclEnergyHitMap::const_iterator i = fEnergyHits.find(cId);
  return
    (i != fEnergyHits.end())
    ?
    (*i).second.Id()
    :
    0;  // something is wrong
}

const EclEnergyHitMap&
TRecEclCF::EnergyHits(void) const
{
  return
    fEnergyHits;
}

const std::vector<TEclCFCR>&
TRecEclCF::CRs(void) const
{
  return
    fCRs;
}

//
// static member functions
//
TRecEclCF&
TRecEclCF::Instance(void)
{
  if (!IsInstance()) {
    fgInstance = new TRecEclCF;
  }
  return *fgInstance;
}

bool
TRecEclCF::IsInstance(void)
{
  return fgInstance != 0;
}

void
TRecEclCF::Kill(void)
{
  if (IsInstance()) {
    delete fgInstance;
    fgInstance = 0;
  }
}

//__________________________________________________________________________
// for E/A corr.

