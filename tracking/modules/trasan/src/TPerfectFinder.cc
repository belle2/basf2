//-----------------------------------------------------------------------------
// $Id: TPerfectFinder.cc 10129 2007-05-18 12:44:41Z katayama $
//-----------------------------------------------------------------------------
// Filename : TPerfectFinder.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to find tracks using MC info.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.9  2004/03/26 06:07:04  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.8  2003/12/25 12:03:34  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.7  2001/12/23 09:58:48  katayama
// removed Strings.h
//
// Revision 1.6  2001/12/19 02:59:47  katayama
// Uss find,istring
//
// Revision 1.5  2001/04/11 11:24:20  katayama
// For CC
//
// Revision 1.4  2001/04/11 01:09:10  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.2  2000/09/13 08:42:26  yiwasaki
// Perfect finder updated
//
// Revision 1.1  2000/08/31 23:51:46  yiwasaki
// Trasan 2.04 : pefect finder added
//
//-----------------------------------------------------------------------------




#include "trg/cdc/WireHitMC.h"
#include "trg/cdc/TrackMC.h"
#include "tracking/modules/trasan/TPerfectFinder.h"
#include "tracking/modules/trasan/TTrack.h"
#include "tracking/modules/trasan/THelix.h"

namespace Belle {

  TPerfectFinder::TPerfectFinder(int perfectFitting,
                                 float maxSigma,
                                 float maxSigmaStereo,
                                 unsigned fittingFlag)
    : _perfectFitting(perfectFitting),
      _builder("standard builder",
               maxSigma,
               maxSigmaStereo,
               0,
               0,
               fittingFlag),
      _fitter("helix fitter")
  {
  }

  TPerfectFinder::~TPerfectFinder()
  {
  }

  std::string
  TPerfectFinder::version(void) const
  {
    return "2.04";
  }

  void
  TPerfectFinder::dump(const std::string& msg, const std::string& pre) const
  {
    std::cout << pre;
    TFinderBase::dump(msg);
  }

  int
  TPerfectFinder::doit(const CAList<Belle2::TRGCDCWireHit> & axialHits,
                       const CAList<Belle2::TRGCDCWireHit> & stereoHits,
                       AList<TTrack> & tracks,
                       AList<TTrack> &)
  {
//cnv
//     //...Preparations...
//     static const HepGeom::Point3D<double> dummy(0, 0, 0);
//     CAList<TRGCDCWireHit> hits;
//     hits.append(axialHits);
//     hits.append(stereoHits);

//     //...Make a list of HEP track...
//     CAList<TTrackHEP> heps;
//     const unsigned nHits = hits.length();
//     for (unsigned i = 0; i < nHits; i++) {
//  const TRGCDCWireHitMC * const mc = hits[i]->mc();
//  if (! mc) continue;
//  const TTrackHEP * const hep = mc->hep();

//  if (! hep) continue;
//  heps.append(hep);
//     }
//     heps.purge();

//     //...Create tracks...
//     const AList<TRGCDCWireHitMC> & mcHits = TRGCDC::getTRGCDC()->hitsMC();
// //  const unsigned nHitsMC = mcHits.length();
//     const unsigned nHeps = heps.length();
//     for (unsigned i = 0; i < nHeps; i++) {
//  const TTrackHEP & hep = * heps[i];
//  const float chg = charge(hep.pType());
//  AList<TLink> hepLinks;
//  Point3D posIn, posOut;
//  HepGeom::Vector3D<double> momIn, momOut;
//  float rMin = 99999;
//  float rMax = 0.;
//  TLink * linkIn;
//  unsigned lastLayer = 0;
//  for (unsigned j = 0; j < (unsigned) mcHits.length(); j++) {
//      const TRGCDCWireHitMC * const mc = mcHits[j];
//      if (! mc) continue;
//      if (! mc->hit()) continue;
//      if (& hep != mc->hep()) continue;
//      if (! hits.hasMember(mc->hit())) continue;

//      //...Remove hits by curl back...(assuming order of mc hits)
//      if (mc->wire()->layerId() < lastLayer) break;
//      lastLayer = mc->wire()->layerId();
//      HepGeom::Point3D<double> ent = mc->entrance();
//      Vector3D dir = mc->direction();
//      ent.setZ(0.);
//      dir.setZ(0.);
//      if ((ent.unit()).dot(dir.unit()) < 0.5) continue;

//      //...Hit...
//      TLink * l = new TLink(0, mc->hit(), dummy);
//      l->leftRight(mc->leftRight());
//      hepLinks.append(l);
//      _links.append(l);

//      //...Check r to get MC mom...
//      const float r = ent.mag();
//      if (r < rMin) {
//    posIn = mc->entrance();
//    momIn = mc->momentum();
//    rMin = r;
//    linkIn = l;
//      }
//      if (r > rMax) {
//    posOut = mc->entrance();
//    momOut = mc->momentum();
//    rMax = r;
//      }
//  }
//  if (_links.length() == 0) continue;

//  //...Do perfect fitting...
//  TTrack * t = 0;
// //     THelix h(posIn, momIn, chg);
//  THelix h(posOut, momOut, chg);
// //     h.pivot(posIn);
//  h.pivot(linkIn->wire()->xyPosition());
//  t = new TTrack(h);
//  t->append(hepLinks);
//  t->assign(0);

// //       THelix hX(posOut, momOut, chg);

// //       std::cout << "    gen@cdc i = " << h.a() << std::endl;
// //       std::cout << "       pivot  = " << h.pivot() << std::endl;
// //       std::cout << "       mom    = " << h.momentum() << std::endl;
// //       std::cout << "    gen@cdc o = " << hX.a() << std::endl;
// //       std::cout << "       pivot  = " << hX.pivot() << std::endl;
// //       std::cout << "       mom    = " << hX.momentum() << std::endl;
// //       std::cout << "    momOut    = " << momOut << std::endl;
// //       std::cout << "    posOut    = " << posOut << std::endl;
// //       std::cout << "    ptOut     = " << momOut.perp() << std::endl;
// //       std::cout << "    costhOut  = " << momOut.z() / momOut.mag();
// //       std::cout << std::endl;

//  if (_perfectFitting == 0) {
//      std::cout << "special test in perfect finder" << std::endl;
//      AList<TLink> tmp;
//      for (unsigned i = 0; i < t->nLinks(); i++) {
//    bool rem = false;
//    if (t->links()[i]->wire()->name() == "0-56")
//        rem = true;
//    else if (t->links()[i]->wire()->name() == "2-56")
//        rem = true;
//    else if (t->links()[i]->wire()->name() == "5-57")
//        rem = true;
//    else if (t->links()[i]->wire()->name() == "6=68")
//        rem = true;
//    else if (t->links()[i]->wire()->name() == "7=69")
//        rem = true;
//    else if (t->links()[i]->wire()->name() == "8=68")
//        rem = true;
//    else if (t->links()[i]->wire()->name() == "10-85")
//        rem = true;
//    else if (t->links()[i]->wire()->name() == "20-126")
//        rem = true;
//    else if (t->links()[i]->wire()->name() == "39-210")
//        rem = true;
//    else if (t->links()[i]->wire()->name() == "41=221")
//        rem = true;
//    else if (t->links()[i]->wire()->name() == "43=221")
//        rem = true;
//    else if (t->links()[i]->wire()->name() == "46-251")
//        rem = true;
//    else if (t->links()[i]->wire()->name() == "48-251")
//        rem = true;
//    if (rem) {
//        std::cout << t->links()[i]->wire()->name() << " removed" << std::endl;
//        tmp.append(* t->links()[i]);
//    }
//      }
//      t->remove(tmp);
//      t->dump("detail","@lastHit");
//      THelix & h = (THelix &) t->helix();
//      h.pivot(HepGeom::Vector3D<double> (0., 0., 0.));
//      t->dump("detail","@origin ");
//      _fitter.fit(* t);
//      t->dump("detail","fitted  ");
//      static const HepGeom::Vector3D<double> p0(1.226, -1.025, 0.120);
//      std::cout << "Pdif mag=" << (t->p() - p0).mag() << std::endl;
//  }

//  tracks.append(t);
//     }
    return 0;
  }

  void
  TPerfectFinder::clear(void)
  {
    HepAListDeleteAll(_links);
  }

  float
  TPerfectFinder::charge(int pType) const
  {
    float chg;

    if (pType == 11) chg = -1;
    else if (pType == -11) chg = 1;
    else if (pType == 13) chg = -1;
    else if (pType == -13) chg = 1;
    else if (pType == 211) chg = 1;
    else if (pType == -211) chg = -1;
    else if (pType == 321) chg = 1;
    else if (pType == -321) chg = -1;
    else if (pType == 2212) chg = 1;
    else if (pType == -2212) chg = -1;
    else {
      std::cout << "TPerfectFinder !!! charge of particle type=";
      std::cout << pType << " is unknown" << std::endl;
    }

    return chg;
  }

} // namespace Belle

