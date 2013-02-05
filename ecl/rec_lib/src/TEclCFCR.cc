/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/rec_lib/TEclCFCR.h>


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
using namespace ECL;

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
TEclCFCR::TEclCFCR(int id)
  : fId(id)
{
}

TEclCFCR::TEclCFCR(const TEclCFCR& rhs)
  : fId(rhs.fId),
    fMass(rhs.fMass), fEnergy(rhs.fEnergy), fWidth(rhs.fWidth),
    fShowers(rhs.fShowers),
    fEnergyHits(rhs.fEnergyHits),
    fSeeds(rhs.fSeeds),
    fFreeHits(rhs.fFreeHits), fNext(rhs.fNext), fNear(rhs.fNear)
{
}

TEclCFCR::~TEclCFCR()
{
}

//
// assignment operators
//
const TEclCFCR&
TEclCFCR::operator=(const TEclCFCR& rhs)
{
  if (this != &rhs) {
    fId = rhs.fId;
    fMass = rhs.fMass;
    fEnergy = rhs.fEnergy;
    fWidth = rhs.fWidth;
    fShowers = rhs.fShowers;
    fEnergyHits = rhs.fEnergyHits;  // is <map> op= standard?
    fSeeds = rhs.fSeeds;
    fFreeHits = rhs.fFreeHits;
    fNext = rhs.fNext;
    fNear = rhs.fNear;
  }
  return *this;
}

//
// comparison operators
//
bool
TEclCFCR::operator==(const TEclCFCR& rhs) const
{
  return
    (fId == rhs.fId) && (fMass == rhs.fMass) && (fEnergy == rhs.fEnergy);
}

bool
TEclCFCR::operator!=(const TEclCFCR& rhs) const
{
  return
    !(*this == rhs);
}

bool
TEclCFCR::operator<(const TEclCFCR& rhs) const
/// id order?
{
  return
    (fId < rhs.fId);
}

//
// member functions
//
int
TEclCFCR::SearchLocalMaxima(EclGeV threshold)
/// find local maxima in a CR and prepare hit-relation tables.
/// local max(seed) == more energy than all near-neighbors
{

  EclNbr tmpNbr;
  EclEnergyHitMap cands;   // self-destructing map
  /// copy
  for (EclEnergyHitMap::iterator i = fEnergyHits.begin();
       i != fEnergyHits.end(); ++i)
    if ((*i).second.Energy() > threshold)
      cands[(*i).first] = (*i).second;

  /// search
  while (!cands.empty()) {// todo: sort and search will improve speed...
    TEclEnergyHit aCand = (*cands.begin()).second;
    cands.erase(cands.begin());

    bool found = true;

    const EclNbr& aNbr(tmpNbr.getNbr(aCand.CellId()));
    for (std::vector<EclNbr::Identifier>::const_iterator
         iNbr = aNbr.nearBegin(); iNbr != aNbr.nearEnd(); ++iNbr) {
      EclEnergyHitMap::iterator near = fEnergyHits.find(*iNbr);
      if (near != fEnergyHits.end())
        if ((*near).second.Energy() > aCand.Energy()) {
          found = !found;
          break;
        }
    }

    /// hit-relationships
    if (found) {
      fSeeds[aCand.CellId()] = aCand;

// called in FormShowers
//   fFreeHits.erase(aCand.CellId());

      for (std::vector<EclNbr::Identifier>::const_iterator
           iNbr = aNbr.nearBegin(); iNbr != aNbr.nearEnd(); ++iNbr) {

        int cell = *iNbr;
        cands.erase(cell);

        if (!!fEnergyHits.count(*iNbr))
          fNear.insert(
            std::pair<const Identifier, Identifier>(*iNbr, aCand.CellId()));
      }
      for (std::vector<EclNbr::Identifier>::const_iterator
           iNbr = aNbr.nextBegin(); iNbr != aNbr.nextEnd(); ++iNbr) {
        if (!!fEnergyHits.count(*iNbr))
          fNext.insert(
            std::pair<const Identifier, Identifier>(*iNbr, aCand.CellId()));
      }
    }
  }

  return
    fSeeds.size();
}


int
TEclCFCR::FormShowers(int startId)
/// 1st phase clustering
/// only Energy(exclusive sum)/Grade will be done
/// (*) zero suppression for a shower < 10MeV is not done...
{
  Identifier id = startId;
  fFreeHits = fEnergyHits;
  EclNbr tmpNbr;
  for (EclEnergyHitMap::const_iterator iSeed = fSeeds.begin();
       iSeed != fSeeds.end(); ++iSeed) {
    const Identifier cid = (*iSeed).first;
    TEclCFShower aShower(id++);
    aShower.Accumulate(cid, (*iSeed).second.Energy());
    fFreeHits.erase(cid);
    // unf for Unfold()
    aShower.fE3x3unf = aShower.fE3x3 = (*iSeed).second.Energy();
    aShower.fGrade = TEclCFShower::LONE;

    const EclNbr& aNbr(tmpNbr.getNbr(cid));
//      const EclNbr& aNbr(cal.crystalGeometries()[cid-1].nbr());
    for (std::vector<EclNbr::Identifier>::const_iterator
         iNbr = aNbr.nearBegin(); iNbr != aNbr.nearEnd(); ++iNbr) {
      if (!!fEnergyHits.count(*iNbr)) {
        aShower.fE3x3 += fEnergyHits[*iNbr].Energy();
        // cannot be a another seed in the current algo
        if (fNear.count(*iNbr) == 1) {
          aShower.Accumulate(*iNbr, fEnergyHits[*iNbr].Energy());
          aShower.fE3x3unf += fEnergyHits[*iNbr].Energy();
          int cell = *iNbr;
          fFreeHits.erase(cell);
          if (fNext.count(*iNbr) > 0)
            aShower.Grade(TEclCFShower::SEPARATED);
        } else
          aShower.Grade(TEclCFShower::MERGED);
      }
    }
    aShower.fE5x5 = aShower.fE3x3;
    aShower.fE5x5unf = aShower.fE3x3unf;
    for (std::vector<EclNbr::Identifier>::const_iterator
         iNbr = aNbr.nextBegin(); iNbr != aNbr.nextEnd(); ++iNbr) {
      if (!!fEnergyHits.count(*iNbr)) {
        aShower.fE5x5 += fEnergyHits[*iNbr].Energy();
        if (fSeeds.count(*iNbr) || fNear.count(*iNbr)
            || (fNext.count(*iNbr) > 1))
          aShower.Grade(TEclCFShower::ISOLATED);
        else if (fNext.count(*iNbr) == 1) {
          aShower.Accumulate(*iNbr, fEnergyHits[*iNbr].Energy());
          aShower.fE5x5unf += fEnergyHits[*iNbr].Energy();
          fFreeHits.erase(*iNbr);
        }
      }
    }
    fShowers[cid] = aShower;
  }
  return
    id - startId;
}


void
TEclCFCR::Unfold(void)
/// 2nd phase clustering
/// only Unfold Energy Hits(sharing)
/// (*) zero suppression for a shower < 10MeV is not done...
{
  typedef MMap::iterator I;
  EclEnergyHitMap::iterator iHit = fFreeHits.begin();

  while (iHit != fFreeHits.end())
    /// seeds, exclusive cells are removed already(in FormShowers)
  {
    Identifier cid = (*iHit++).first;
    if (fNear.count(cid) > 1) {
      EclGeV sum = 0;
      std::pair<I, I> b = fNear.equal_range(cid);
      for (I i = b.first; i != b.second; ++i)
        sum += fShowers[(*i).second].Energy();
      for (I i = b.first; i != b.second; ++i) {
        EclGeV fraction = fShowers[(*i).second].Energy() / sum;
        fShowers[(*i).second].Assign(cid, fraction);
        fShowers[(*i).second].fE3x3unf
        += fraction * fEnergyHits[cid].Energy();
        fShowers[(*i).second].fE5x5unf
        += fraction * fEnergyHits[cid].Energy();
      }
      fFreeHits.erase(cid);
    } else if (fNext.count(cid) > 1) {
      EclGeV sum = 0;
      std::pair<I, I> b = fNext.equal_range(cid);
      for (I i = b.first; i != b.second; ++i)
        sum += fShowers[(*i).second].Energy();
      for (I i = b.first; i != b.second; ++i) {
        EclGeV fraction = fShowers[(*i).second].Energy() / sum;
        fShowers[(*i).second].Assign(cid, fraction);
        fShowers[(*i).second].fE5x5unf
        += fraction * fEnergyHits[cid].Energy();
      }
      fFreeHits.erase(cid);
    }
  }
}

void
TEclCFCR::UncAttributes(void)
/// only mass, energy for now
/// cr mass is not naive invariant mass!  to relate mass to 2nd moment
/// simple approx. e.g. <x/r> ~ <x>/Rcm, and should use
///   ... anyway r,<r>,Rcm cannot be well-defined, need optimization
///   ... think about staggered case, it'll bias position
{

  ECLGeometryPar* cg = ECLGeometryPar::Instance();
  double e_sum = 0.0;
  TVector3  p_sum;
  TVector3  x_wsum;
  for (EclEnergyHitMap::const_iterator i = fEnergyHits.begin();
       i != fEnergyHits.end(); ++i) {
    Identifier cid = (*i).first;
    double energy = (*i).second.Energy();
    TVector3 x = cg->GetCrystalPos(cid);

    e_sum += energy; // total
    p_sum += energy * x.Unit();  // for gamma e=e*x/r

    x_wsum += energy * x;
  }

  fMass = sqrt(abs(e_sum * e_sum - p_sum.Mag2()));
  fEnergy = e_sum;

  TVector3  x_min = x_wsum * (1 / e_sum);

  double x_minus_x_min_squared = 0.0;
  for (EclEnergyHitMap::const_iterator i = fEnergyHits.begin();
       i != fEnergyHits.end(); ++i) {
    Identifier cid = (*i).first;
    double energy = (*i).second.Energy();
    TVector3 r = cg->GetCrystalPos(cid);
    TVector3  x_minus_xmin = r - x_min;
    x_minus_x_min_squared += x_minus_xmin.Mag2() * energy;
  }

  fWidth = sqrt(x_minus_x_min_squared / e_sum);

  ///_____________________ will be move to another place
  /// drive shower
  for (EclCFShowerMap::iterator iShower = fShowers.begin();
       iShower != fShowers.end(); ++iShower) {
    TEclCFShower& s = (*iShower).second;
    double e_sum = 0.0;
    TVector3  p_sum;
    TVector3  x_wsum;
    for (std::vector<MEclCFShowerHA>::iterator iHA = s.fHA.begin();
         iHA != s.fHA.end(); ++iHA) {
      Identifier cid = iHA->Id();
      double energy = (iHA->Fraction()) * fEnergyHits[cid].Energy();
      TVector3 x = cg->GetCrystalPos(cid);
      e_sum += energy; // total
      p_sum += energy * x.Unit();  // for gamma e=e*x/r == ux_wsum

      x_wsum += energy * x;
    }

    s.fMass = sqrt(abs(e_sum * e_sum - p_sum.Mag2()));

    // this should be equal to uncEnergy!
    s.fEnergy = e_sum;

    TVector3  x_min = x_wsum * (1 / e_sum);
    TVector3  u_min = p_sum * (1 / e_sum);

    /// R,THETA,PHI is not WELL-DEFINED!
    s.fDistance = x_min.Mag();  // define theta, phi properly!!!
    s.fTheta = u_min.Theta();   // by u or x ?
    s.fPhi = u_min.Phi();
    double x_minus_x_min_squared = 0.0;
    for (std::vector<MEclCFShowerHA>::iterator iHA = s.fHA.begin();
         iHA != s.fHA.end(); ++iHA) {
      Identifier cid = iHA->Id();
      double energy = (iHA->Fraction()) * fEnergyHits[cid].Energy();
      TVector3 r = cg->GetCrystalPos(cid);
      TVector3  x_minus_xmin = r - x_min;
      x_minus_x_min_squared += x_minus_xmin.Mag2() * energy;
    }
    s.fWidth = sqrt(x_minus_x_min_squared / e_sum);
  }//for iShower
}


void
TEclCFCR::Attributes(void)
{
}

