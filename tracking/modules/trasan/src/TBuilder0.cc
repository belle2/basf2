//-----------------------------------------------------------------------------
// $Id: TBuilder0.cc 10129 2007-05-18 12:44:41Z katayama $
//-----------------------------------------------------------------------------
// Filename : TBuilder0.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to build a track.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.15  2005/03/11 03:57:46  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.14  2004/03/26 06:07:03  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.13  2003/12/25 12:03:30  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.12  2001/12/23 09:58:44  katayama
// removed Strings.h
//
// Revision 1.11  2001/12/19 02:59:43  katayama
// Uss find,istring
//
// Revision 1.10  2001/04/11 11:24:20  katayama
// For CC
//
// Revision 1.9  2001/04/11 01:09:07  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.8  2000/11/17 06:41:34  yiwasaki
// Trasan 2.18 : option for new CDC geometry
//
// Revision 1.7  2000/10/05 23:54:20  yiwasaki
// Trasan 2.09 : TLink has drift time info.
//
// Revision 1.6  2000/04/11 13:05:37  katayama
// Added std:: to cout, cerr, endl etc.
//
// Revision 1.5  2000/03/21 07:01:25  yiwasaki
// tmp updates
//
// Revision 1.4  2000/03/17 11:01:37  yiwasaki
// Trasan 2.00RC16 : updates for new tracking scheme
//
// Revision 1.3  2000/01/28 06:30:20  yiwasaki
// Trasan 1.67h : new conf modified
//
// Revision 1.2  1999/11/19 09:13:05  yiwasaki
// Trasan 1.65d : new conf. finder updates, no change in default conf. finder
//
// Revision 1.1  1999/10/30 10:12:10  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.43  1999/10/21 15:45:12  yiwasaki
// Trasan 1.65b : T3DLine, T3DLineFitter, TConformalFinder0 added : no change in Trasan outputs
//
// Revision 1.41  1999/09/21 02:01:32  yiwasaki
// Trasan 1.63b release : conformal finder minor changes, TWindow added for debug
//
// Revision 1.39  1999/09/10 09:19:56  yiwasaki
// Trasan 1.61b release : new parameters added, TTrackMC bug fixed
//
// Revision 1.38  1999/08/25 06:25:47  yiwasaki
// Trasan 1.60b release : curl finder updated, conformal accepts fitting flags
//
// Revision 1.37  1999/08/04 01:01:40  yiwasaki
// Trasan 1.59b release : putting parameters from basf enabled
//
// Revision 1.36  1999/07/27 08:01:35  yiwasaki
// Trasan 1.58b release : protection for fpe error(not completed yet)
//
// Revision 1.35  1999/07/23 03:53:22  yiwasaki
// Trasan 1.57b release : minor changes only
//
// Revision 1.34  1999/07/09 01:47:18  yiwasaki
// Trasan 1.53a release : cathode updates by T.Matsumoto, minor change of Conformal finder
//
// Revision 1.33  1999/07/01 08:15:21  yiwasaki
// Trasan 1.51a release : builder bug fix, TRGCDC bug fix again, T0 determination has more parameters
//
// Revision 1.32  1999/06/15 06:33:40  yiwasaki
// Trasan 1.43 release : minor changes in TRGCDCClust and TBuilder
//
// Revision 1.31  1999/06/14 12:40:20  yiwasaki
// Trasan 1.42 release : bug in findCloseHits fixed, sakura 1.06
//
// Revision 1.30  1999/06/10 09:44:50  yiwasaki
// Trasan 1.40 release : minor changes only
//
// Revision 1.29  1999/03/21 15:45:37  yiwasaki
// Trasan 1.28 release : TrackManager bug fix, CosmicFitter added in BuilderCosmic, parameter salvage level added
//
// Revision 1.28  1999/03/11 23:27:09  yiwasaki
// Trasan 1.24 release : salvaging improved
//
// Revision 1.27  1999/03/09 13:42:59  yiwasaki
// Trasan 1.22 release : TrackManager bug fix
//
// Revision 1.26  1999/03/08 05:47:51  yiwasaki
// Trasan 1.20 release : Fitter in TBuilder is modified
//
// Revision 1.25  1999/02/04 02:12:21  yiwasaki
// Trasan 1.15 release : bug fix in cluster table output from S.Suzuki
//
// Revision 1.24  1999/02/03 06:23:06  yiwasaki
// Trasan 1.14 release : bugs in curl finder and trasan fixed, new salvage installed
//
// Revision 1.23  1999/01/11 03:03:04  yiwasaki
// Fitters added
//
// Revision 1.22  1998/12/24 08:46:53  yiwasaki
// stereo building modified by J.Suzuki
//
// Revision 1.21  1998/11/10 09:08:55  yiwasaki
// Trasan 1.1 beta 8 release : negative sqrt fixed, curl finder updated by j.tanaka, TRGCDC classes modified by y.iwasaki
//
// Revision 1.20  1998/10/09 17:35:31  yiwasaki
// Trasan 1.1 beta 6 release : TBuilder::buildStereo bug, introduced by y.iwasaki, removed.
//
// Revision 1.19  1998/10/09 03:01:04  yiwasaki
// Trasan 1.1 beta 4 release : memory leak stopped, and minor changes
//
// Revision 1.18  1998/10/06 02:30:01  yiwasaki
// Trasan 1.1 beta 3 relase : only minor change
//
// Revision 1.17  1998/09/29 01:24:18  yiwasaki
// Trasan 1.1 beta 1 relase : TBuilderCurl added
//
// Revision 1.16  1998/08/31 05:15:39  yiwasaki
// Trasan 1.09 release : curl finder updated by J.Tanaka, MC classes updated by Y.Iwasaki
//
// Revision 1.15  1998/08/17 01:00:30  yiwasaki
// TSegmentLinker::roughSelection added, TRASAN_DEBUG option works
//
// Revision 1.14  1998/08/12 16:32:39  yiwasaki
// Trasan 1.08 release : stereo finder updated by J.Suzuki, new MC classes added by Y.Iwasaki
//
// Revision 1.13  1998/07/29 04:34:46  yiwasaki
// Trasan 1.06 release : back to Trasan 1.02
//
// Revision 1.9  1998/07/06 15:48:45  yiwasaki
// Trasan 1.01 release:CurlFinder default on, bugs in TLine and TTrack::fit fixed
//
// Revision 1.8  1998/07/02 09:04:35  yiwasaki
// Trasan 1.0 official release
//
// Revision 1.7  1998/06/24 06:54:58  yiwasaki
// Trasan 1 beta 5.2 release, memory leaks fixed
//
// Revision 1.6  1998/06/21 18:38:14  yiwasaki
// Trasan 1 beta 5 release, rphi improved?
//
// Revision 1.5  1998/06/19 12:17:10  yiwasaki
// Trasan 1 beta 4.1 release, TBuilder::buildStereo updated
//
// Revision 1.4  1998/06/17 20:22:55  yiwasaki
// Trasan 1 beta 4 release again, KS effi. improved?
//
// Revision 1.3  1998/06/17 20:12:33  yiwasaki
// Trasan 1 beta 4 release, KS effi. improved?
//
// Revision 1.2  1998/06/15 09:58:13  yiwasaki
// Trasan 1 beta 3.2, bug fixed
//
// Revision 1.1  1998/06/14 11:09:47  yiwasaki
// Trasan beta 3 release, TBuilder and TSelector added
//
//-----------------------------------------------------------------------------



#include "tracking/modules/trasan/TBuilder0.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Layer.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "tracking/modules/trasan/TLink.h"
#include "tracking/modules/trasan/TCircle.h"
#include "tracking/modules/trasan/TLine0.h"
#include "tracking/modules/trasan/TTrack.h"
//#include "tracking/modules/trasan/TSegment0.h"
#ifdef TRASAN_WINDOW
#include "tracking/modules/trasan/TWindow.h"
#endif

namespace Belle {

  extern const HepGeom::Point3D<double>  ORIGIN;

  TBuilder0::TBuilder0(const std::string& a)
    : _name(a),
      _fitter("TBuilder0 Fitter"),
      _salvageLevel(30.),
      _stereoZ3(20.),
      _stereoZ4(20.),
      _stereoChisq3(15.),
      _stereoChisq4(9.),
      _stereoMaxSigma(30.)
  {
  }

  TBuilder0::TBuilder0(const std::string& a, float salvageLevel)
    : _name(a),
      _fitter("TBuilder0 Fitter"),
      _salvageLevel(salvageLevel),
      _stereoZ3(20.),
      _stereoZ4(20.),
      _stereoChisq3(15.),
      _stereoChisq4(9.),
      _stereoMaxSigma(30.)
  {
  }

  TBuilder0::TBuilder0(const std::string& a,
                       float stereoZ3,
                       float stereoZ4,
                       float stereoChisq3,
                       float stereoChisq4,
                       float stereoMaxSigma,
                       unsigned corrections,
                       float salvageLevel)
    : _name(a),
      _fitter("TBuilder0 Fitter"),
      _salvageLevel(salvageLevel),
      _stereoZ3(stereoZ3),
      _stereoZ4(stereoZ4),
      _stereoChisq3(stereoChisq3),
      _stereoChisq4(stereoChisq4),
      _stereoMaxSigma(stereoMaxSigma)
  {
    if (corrections & 1) _fitter.sag(true);
    if (corrections & 2) _fitter.propagation(true);
    if (corrections & 4) _fitter.tof(true);
    if (corrections & 8) _fitter.freeT0(true);
  }

  TBuilder0::~TBuilder0()
  {
  }

  void
  TBuilder0::dump(const std::string&, const std::string&) const
  {
  }

  TTrack*
  TBuilder0::buildRphi(const AList<TLink> & list) const
  {
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << _name << " ... building a rphi track" << std::endl;
    std::cout << _name << " ... selecting good hits" << std::endl;
//      TTrackBase tmp;
//      tmp.append(list);
//      rphiWindow->append(tmp, leda_red);
#endif

    //...Check # of links...
    if ((unsigned) list.length() < _circleSelector.nLinks()) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << _name << " ... rejected by nLinks(";
      std::cout << list.length() << ") < ";
      std::cout << _circleSelector.nLinks() << std::endl;
#endif
    }

    //...Select core hits...
    AList<TLink> cores = list;
    selectHits(cores);
    if (cores.length() < 5) cores = list;

    //...Core check...
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << _name << " ... checking cores : cores=" << std::endl;
    TLink::dump(cores, "detail");
#endif
    unsigned sLinks = TLink::superLayer(list);
    unsigned sUsed = TLink::superLayer(cores);
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "    super layer ptn=" << sLinks;
    std::cout << ",super layer used=" << sUsed << std::endl;
#endif
    if (sLinks != sUsed) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << _name << "    ... appending hits to cores" << std::endl;
#endif
      unsigned diff = sLinks - sUsed;
      for (unsigned j = 0; j < 6; j++) {
        if (diff & (1 << ((5 - j) * 2))) {
#ifdef TRASAN_DEBUG_DETAIL
          std::cout << "    super layer " << (5 - j) * 2 << "searching";
          std::cout << std::endl;
#endif
          AList<TLink> links = TLink::sameSuperLayer(list, (5 - j) * 2);
          TLink* best = NULL;
          double bestD = 999.;
          for (unsigned i = 0; i < (unsigned) links.length(); i++) {
            double dist = links[i]->hit()->drift();
            if (dist < 0.02) {
#ifdef TRASAN_DEBUG_DETAIL
              std::cout << "    " << links[i]->wire()->name();
              std::cout << " appended (small dist)" << std::endl;
#endif
              cores.append(links[i]);
              continue;
            }
            if (dist < bestD) {
              best = links[i];
              bestD = dist;
            }
          }
          if (best) {
            cores.append(best);
#ifdef TRASAN_DEBUG_DETAIL
            std::cout << "    " << best->wire()->name();
            std::cout << " appended (best)" << std::endl;
#endif
          }
        }
      }
    }

    //...Check cores again...
    unsigned nCores = cores.length();
    AList<TLink> realCores;
    for (unsigned i = 0; i < nCores; i++) {
      TLink* l = cores[i];
      if (l->hit()->state() & CellHitFittingValid)
        realCores.append(l);
    }
    if (TLink::nSuperLayers(realCores) < 2) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "    ... rejected by small number of super layers" << std::endl;
#endif
      return NULL;
    }
    if (TLink::nLayers(realCores) < 5) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "    ... rejected by small number of layers" << std::endl;
#endif
      return NULL;
    }

    //...Make a circle...
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << _name << " ... making a circle : #cores=" << cores.length();
    std::cout << std::endl;
#endif
    AList<TLink> hits = list;
    hits.remove(cores);
    TCircle c(cores);

    //...Test it...
    if (! _circleSelector.preSelect(c)) return NULL;

    //...Fitting...
    int err = c.fit();
    if (err < 0) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "    ... rejected by failure of the 1st fit : ";
      std::cout << "err = " << err << std::endl;
#endif
      return NULL;
    }

    //...Test it...
    if (! _circleSelector.select(c)) return NULL;

    //...Make a track...
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << _name << " ... making a track" << std::endl;
#endif
    TTrack* t = new TTrack(c);
    if (! _trackSelector.preSelect(* t)) {
      delete t;
      return NULL;
    }

    //...Fitting...
    AList<TLink> bad;
    err = fit(* t);
    if (err < 0) goto discard;
    t->refine(bad, _trackSelector.maxSigma() * 100.);
    err = fit(* t);
    if (err < 0) goto discard;
#ifdef TRASAN_DEBUG_DETAIL
    t->dump("detail", "    1st> ");
#endif

    //...Test it...
    if (! _trackSelector.select(* t)) goto discard;

    //...Try to append non-core hits...
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << _name << " ... appending non-core hits" << std::endl;
#endif
    t->appendByApproach(hits, sqrt(_trackSelector.maxSigma()));
    err = fit(* t);
    if (err < 0) goto discard;
    t->refine(bad, _trackSelector.maxSigma() * 10.);
    err = fit(* t);
    if (err < 0) goto discard;
    t->refine(bad, _trackSelector.maxSigma());
    err = fit(* t);
    if (err < 0) goto discard;
#ifdef TRASAN_DEBUG_DETAIL
    t->dump("detail", "    2nd> ");
#endif

    //...Test it...
    if (! _trackSelector.select(* t)) goto discard;

    //...OK...
#ifdef TRASAN_DEBUG_DETAIL
//      rphiWindow->append(* t, leda_blue);
//      rphiWindow->draw();
//      rphiWindow->wait();
//      rphiWindow->remove(tmp);
//      rphiWindow->remove(* t);
#endif

    return t;

    //...Something happened...
discard:
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "    ... rejected by fitting failure : ";
    std::cout << " err = " << err << std::endl;
//      rphiWindow->append(* t, leda_blue);
//      rphiWindow->draw();
//      rphiWindow->wait();
//      rphiWindow->remove(tmp);
//      rphiWindow->remove(* t);
#endif
    delete t;
    return NULL;
  }

  void
  TBuilder0::selectHits(AList<TLink> & list) const
  {
    AList<TLink> bad;
    for (unsigned i = 0; i < (unsigned) list.length(); i++) {
      unsigned state = list[i]->hit()->state();
      if ((!(state & CellHitContinuous)) ||
          (!(state & CellHitIsolated))) {
        bad.append(list[i]);
        continue;
      }
      if ((!(state & CellHitPatternLeft)) &&
          (!(state & CellHitPatternRight))) {
        bad.append(list[i]);
        continue;
      }
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << _name << "(TBuilder0::selectHits) ... dump of candidates" << std::endl;
    TLink::dump(list, "detail");
#endif

    list.remove(bad);
  }

  TTrack*
  TBuilder0::buildStereo0(TTrack& track, const AList<TLink> & list) const
  {
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << _name << "(stereo) ... dump of stereo candidate hits" << std::endl;
    TLink::dump(list, "sort flag", "    ");
#endif

    //...Check # of links...
    if ((unsigned) list.length() < _lineSelector.nLinksStereo()) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << _name << "(stereo) ... rejected by nLinks(";
      std::cout << list.length() << ") < ";
      std::cout << _lineSelector.nLinks() << std::endl;
#endif
      return NULL;
    }

    //...Calculate s and z for every links...
    unsigned n = list.length();
    AList<TLink> forLine;
    for (unsigned i = 0; i < n; i++) {
      TLink* l = list[i];
      TLink* t = new TLink(* l);

      //...Assuming wire position...
      t->leftRight(2);
      int err = track.szPosition(* t);
      if (err) {
        delete t;
        continue;
      }

      //...Store the sz link...
      t->link(l);
      forLine.append(t);
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << _name << "(stereo) ... dump of sz links" << std::endl;
    TLink::dump(forLine, "sort flag", "    ");
#endif

    //...Check # of sz links...
    if ((unsigned) forLine.length() < _lineSelector.nLinksStereo()) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << _name << "(stereo) ... rejected by sz nLinks(";
      std::cout << forLine.length() << ") < ";
      std::cout << _lineSelector.nLinks() << std::endl;
#endif
      HepAListDeleteAll(forLine);
      return NULL;
    }

    //...Make a line...
    unsigned nLine = forLine.length();
    TLine0 line(forLine);
    int err = line.fit2sp();
    if (err < 0) {
      err = line.fit2p();
      if (err < 0) err = line.fit();
    }

    //...Linear fit...
    if (err < 0) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << _name << "(stereo) ... linear fit failure. nLinks(";
      std::cout << forLine.length() << ")" << std::endl;
#endif
      HepAListDeleteAll(forLine);
      return NULL;
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << _name << "(stereo) ... dump of left-right" << std::endl;
#endif

    //...Decide Left or Right...
    AList<TLink> forNewLine;
    for (unsigned i = 0; i < nLine; i++) {
      TLink* t = forLine[i];
      TLink* tl = new TLink(* t);
      TLink* tr = new TLink(* t);

      tl->leftRight(CellHitLeft);
      tr->leftRight(CellHitRight);

      int err = track.szPosition(* tl);
      if (err) {
        delete tl;
        tl = NULL;
      }
      err = track.szPosition(* tr);
      if (err) {
        delete tr;
        tr = NULL;
      }
      if ((tl == NULL) && (tr == NULL)) continue;

      TLink* best;
      if (tl == NULL) best = tr;
      else if (tr == NULL) best = tl;
      else {
        if (line.distance(* tl) < line.distance(* tr)) {
          best = tl;
          delete tr;
        } else {
          best = tr;
          delete tl;
        }
      }

#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "    ";
      std::cout << t->wire()->layerId() << "-";
      std::cout << t->wire()->localId();
      if (tl != NULL)
        std::cout << ",left " << tl->position() << "," << line.distance(* tl);
      if (tr != NULL)
        std::cout << ",right " << tr->position() << "," << line.distance(* tr);
      std::cout << std::endl;
#endif

      best->link(t->link());
      forNewLine.append(best);
    }

    //...Check # of sz links...
    if ((unsigned) forNewLine.length() < _lineSelector.nLinksStereo()) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << _name << "(stereo) ... rejected by lr nLinks(";
      std::cout << forNewLine.length() << ") < ";
      std::cout << _lineSelector.nLinks() << std::endl;
#endif
      HepAListDeleteAll(forLine);
      HepAListDeleteAll(forNewLine);
      return NULL;
    }

    //...Create new line...
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << _name << "(stereo) ... creating a new line" << std::endl;
    unsigned nNewLine = forNewLine.length();
#endif
    TLine0 newLine(forNewLine);

    //... Remove extremely bad points
    newLine.removeChits();

    //...Make a seed track again
    err = newLine.fit2sp();
    if (err < 0) {
      err = newLine.fit2p();
      if (err < 0) err = newLine.fit();
    }

    //...Linear fit...
    if (err < 0) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << _name << "(stereo) ... 2nd linear fit failure. nLinks(";
      std::cout << forNewLine.length() << ")" << std::endl;
#endif
      HepAListDeleteAll(forLine);
      HepAListDeleteAll(forNewLine);
      return NULL;
    }

    //...Remove bad points...
    AList<TLink> bad;
    newLine.refine(bad, 40.);
    err = newLine.fit();
    newLine.refine(bad, 30.);
    err = newLine.fit();
    newLine.refine(bad, 20.);
    err = newLine.fit();
    newLine.refine(bad, 10.);
    err = newLine.fit();
    float R = fabs(track.helix().curv());
    if (R > 80.) {
      newLine.refine(bad, 5.);
      err = newLine.fit();
    }

    //...Linear fit again...
    if (err < 0) {
      HepAListDeleteAll(forLine);
      HepAListDeleteAll(forNewLine);
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "    appendStereo cut ... new line 2nd linear fit failure. ";
      std::cout << "# of links = " << n << "," << nLine;
      std::cout << "," << nNewLine << std::endl;
#endif
      return NULL;
    }

    //...3D fit...
    const AList<TLink> & good = newLine.links();
    unsigned nn = good.length();
    for (unsigned i = 0; i < nn; i++) {
      track.append(* good[i]->link());
    }
    CLHEP::HepVector a(5);
    a = track.helix().a();
    a[4] = track.charge() * newLine.a();
    track._helix->a(a);

    //...Refine...
    err = fit(track);
    track.refine(bad, _trackSelector.maxSigma() * 100.);
    err = fit(track);
    track.refine(bad, _trackSelector.maxSigma() * 10.);
    err = fit(track);
    track.refine(bad, _trackSelector.maxSigma());
    err = fit(track);

    //...Test it...
    if (! _trackSelector.select(track)) {
      HepAListDeleteAll(forLine);
      HepAListDeleteAll(forNewLine);
      return NULL;
    }

    //...Termination...
    HepAListDeleteAll(forLine);
    HepAListDeleteAll(forNewLine);
    return & track;
  }

  TTrack*
  TBuilder0::buildStereo(TTrack& track, const AList<TLink> & list) const
  {
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << _name << "(stereo) ... building in 3D" << std::endl;
    std::cout << "... dump of stereo candidate hits" << std::endl;
    TLink::dump(list, "sort flag", "    ");
#endif

    //...Check # of links...
    if ((unsigned) list.length() < _lineSelector.nLinksStereo()) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "... rejected by nLinks(";
      std::cout << list.length() << ") < ";
      std::cout << _lineSelector.nLinks() << std::endl;
#endif
      return NULL;
    }

    //...Setup...
    int ichg;
    if (track.helix().curv() > 0.) ichg = 1;
    else                           ichg = -1;
    unsigned nlyr[18] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    unsigned llyr[18] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    //...Check
    //... # of hits in a wire,
    //... 2 contineus hits or not
    //...
    //...and Calculate s and z for every links...
    unsigned n = list.length();
    AList<TLink> forLine;
    unsigned iforLine = 0;
    for (unsigned i = 0; i < n; i++) {
      TLink* l = list[i];

      if (i < n - 1) {
        TLink* lnext = list[i + 1];

        //...2 consective hits ?...
        // int LorR = check2CnHits(* l, * lnext, ichg);
        int LorR = consectiveHits(* l, * lnext, ichg);
//...For debug...
//        if (check2CnHits(* l, * lnext, ichg) !=
//      consectiveHits(* l, * lnext, ichg)) {
//      AList<TLink> tmp;
//      tmp.append(l);
//      tmp.append(lnext);
//      std::cout << "!!! consective diff !!!" << std::endl;
//      std::cout << "    original = " << check2CnHits(* l, * lnext, ichg);
//      std::cout << ", iw = " << consectiveHits(* l, * lnext, ichg);
//      std::cout << std::endl;
//      Dump(tmp, "detail");
//        }
//...For debug end...

        //...Left/right solved by two consective hits...
        if (LorR == CellHitLeft || LorR == CellHitRight) {

          //... Set Left/Right
          if (LorR == CellHitLeft) {
            l->leftRight(CellHitLeft);
            lnext->leftRight(CellHitRight);
          } else {
            l->leftRight(CellHitRight);
            lnext->leftRight(CellHitLeft);
          }

          //...Calculate z...
          int err1 = track.szPosition(* l);
          int err2 = track.szPosition(* lnext);
          if (err1 == 0 && err2 == 0) {
            double deltaZ = fabs(l->position().y() -
                                 lnext->position().y());
            if (deltaZ < 1.5) {

              //... O.K. l and lnext should be good 2 consectvie hits
              l->zStatus(20);
              lnext->zStatus(20);
              l->zPair(iforLine + 1);
              lnext->zPair(iforLine);
            }
#ifdef TRASAN_DEBUG_DETAIL
            else {
              std::cout << "    ... rejected because delta z > 1.5";
              std::cout << std::endl;
            }
#endif
          }
#ifdef TRASAN_DEBUG_DETAIL
          else {
            if (err1) {
              std::cout << "    ... s-z calculation error with ";
              std::cout << l->wire()->name() << std::endl;
            }
            if (err2) {
              std::cout << "    ... s-z calculation error with ";
              std::cout << lnext->wire()->name() << std::endl;
            }
          }
#endif
        }
      }

      //... Calculate s and z...
      //... Aleady solved
      if (l->zStatus() == 20) {
        TLink* t = new TLink(* l);
        t->zStatus(l->zStatus());
        t->zPair(l->zPair());
        int err = track.szPosition(* t);
        if (err) {
          delete t;
          continue;
        } else {
          //...Store the sz link...
          t->link(l);
          forLine.append(t);
          //... Check # of hits in a wire layer and Clustering them
          // nlyr[layertoStlayer(l->wire()->layerId())] += 1;
          nlyr[l->wire()->layer().axialStereoLayerId()] += 1;
          // llyr[layertoStlayer(l->wire()->layerId())]  = iforLine;
          llyr[l->wire()->layer().axialStereoLayerId()]  = iforLine;
          iforLine += 1;
        }
      } else {
        //...Assuming wire position...
        //... for left
        TLink* tl = new TLink(* l);
        tl->leftRight(CellHitLeft);
        tl->zStatus(-10);
        tl->zPair(0);
        int err = track.szPosition(* tl);
        if (err) {
          delete tl;
        } else {
          //...Store the sz link...
          tl->link(l);
          forLine.append(tl);
          //... Check # of hits in a wire layer and Clustering them
          // nlyr[layertoStlayer(l->wire()->layerId())] += 1;
          nlyr[l->wire()->layer().axialStereoLayerId()] += 1;
          // llyr[layertoStlayer(l->wire()->layerId())]  = iforLine;
          llyr[l->wire()->layer().axialStereoLayerId()]  = iforLine;
          //...
          iforLine += 1;
        }

        //... for right
        TLink* tr = new TLink(* l);
        tr->leftRight(CellHitRight);
        tr->zStatus(-10);
        tr->zPair(0);
        err = track.szPosition(* tr);
        if (err) {
          delete tr;
        } else {
          //...Store the sz link...
          tr->link(l);
          forLine.append(tr);
          //... Check # of hits in a wire layer and Clustering them
          // nlyr[layertoStlayer(l->wire()->layerId())] += 1;
          nlyr[l->wire()->layer().axialStereoLayerId()] += 1;
          // llyr[layertoStlayer(l->wire()->layerId())]  = iforLine;
          llyr[l->wire()->layer().axialStereoLayerId()]  = iforLine;
          iforLine += 1;
        }
      }
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "... dump of sz links" << std::endl;
    TLink::dump(forLine, "sort flag", "    ");
    for (unsigned i = 0; i < 18; i++) {
      std::cout << i << " : " << nlyr[i] << ", " << llyr[i] << std::endl;
    }
#endif

    //...Check # of sz links...
    if ((unsigned) forLine.length() < _lineSelector.nLinksStereo()) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "... rejected by sz nLinks(";
      std::cout << forLine.length() << ") < ";
      std::cout << _lineSelector.nLinks() << std::endl;
#endif
      HepAListDeleteAll(forLine);
      return NULL;
    }

    //... Select the best segment in each Superlayer
    AList<TLink> gdSLine0;
    AList<TLink> gdSLine1;
    AList<TLink> gdSLine2;
    AList<TLink> gdSLine3;
    AList<TLink> gdSLine4;
    double min_chi2[5] = {9999., 9999., 9999., 9999., 9999.};
    double min_a[5] = {9999., 9999., 9999., 9999., 9999.};

#ifdef TRASAN_DEBUG_DETAIL
//      TTrackBase base;
//      base.append(forLine);
//      TWindow baseWindow("s-z window");
//      baseWindow.append(base);

    bool display = false;
//      for (unsigned i = 0; i < forLine.length(); i++) {
//    if (forLine[i]->wire()->name() == "15=31")
//        display = true;
//      }
#endif

    //...For all stereo super layers...
    for (unsigned isl = 0; isl < 5; isl++) {
      AList<TLink> tmpLine;
      AList<TLink> goodLine;

#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "    ... stereo supter layer " << isl << std::endl;
#endif

      //...3-stereo-layer case...
      if (isl < 2) {

        //...Initialize...
//      unsigned ily1 = TRGCDC::getTRGCDC("1.0")->superLayer(isl * 2 + 1)
//    ->first()->axialStereoLayerId();
        unsigned ily1 = (* Belle2::TRGCDC::getTRGCDC("1.0")->superLayer(isl * 2 + 1))[0]->axialStereoLayerId();
        unsigned ily2 = ily1 + 1;
        unsigned ily3 = ily2 + 1;

        //...Loop for the first layer...
        unsigned ilst = llyr[ily1] + 1;
        unsigned ifst = ilst - nlyr[ily1];
        for (unsigned i = ifst; i < ilst; i++) {
          TLink& l = * forLine[i];
          tmpLine.append(l);
          TLink& m = *forLine[l.zPair()];
          if (l.zStatus() == 20) {
            if ((unsigned) l.zPair() < i) {
              tmpLine.append(m);
            } else {
              //...Already considered...
              tmpLine.remove(l);
              continue;
            }
          }

          //...Loop for the second layer...
          unsigned jlst = llyr[ily2] + 1;
          unsigned jfst = jlst - nlyr[ily2];
          for (unsigned j = jfst; j < jlst; j++) {
            TLink& l2 = * forLine[j];
            tmpLine.append(l2);
            TLink& m2 = *forLine[l2.zPair()];
            if (l2.zStatus() == 20) {
              if ((unsigned) l2.zPair() < j) {
                tmpLine.append(m2);
              } else {
                //...Already considered.
                tmpLine.remove(l2);
                continue;
              }
            }

            //...Loop for the third layer...
            unsigned klst = llyr[ily3] + 1;
            unsigned kfst = klst - nlyr[ily3];
            for (unsigned k = kfst; k < klst; k++) {
              TLink& l3 = * forLine[k];
              tmpLine.append(l3);
              TLink& m3 = * forLine[l3.zPair()];
              if (l3.zStatus() == 20) {
                if ((unsigned) l3.zPair() < k) {
                  tmpLine.append(m3);
                } else {
                  //...Already considered.
                  tmpLine.remove(l3);
                  continue;
                }
              }

              //... Check the hits in neighbor wirelayer
              //...
              //...    x|o|o|x   |
              //...      |o|     |
              //...              V IP ,  o means OK.
              //...
              int relation12 = -1;
              int relation23 = -1;

              //... Check the hit in fist and it in the second layer
              relation12 = checkHits(l.hit()->wire().localId(),
                                     l2.hit()->wire().localId(),
                                     isl);
              //...
              if (l.zStatus() == 20 && relation12 < 0)
                relation12 = checkHits(m.hit()->wire().localId(),
                                       l2.hit()->wire().localId(),
                                       isl);

              //...Check the hit in second  and it in the third layer
              relation23 = checkHits(l2.hit()->wire().localId(),
                                     l3.hit()->wire().localId(),
                                     isl);
              if (l.zStatus() == 20 && relation23 < 0)
                relation23 = checkHits(m2.hit()->wire().localId(),
                                       l3.hit()->wire().localId(),
                                       isl);

              //...Bad relation...
              if (relation12 || relation23) {
#ifdef TRASAN_DEBUG_DETAIL
                std::cout << "    ... bad relations";
                std::cout << " : segment rejected";
                TLink::dump(tmpLine, "detail stereo", "    ");
                if (display) {
                  TLine0 line(tmpLine);
                  line.fit();
//        int err = line.fit();
//          baseWindow.append(line);
//          baseWindow.draw();
//          baseWindow.wait();
//          baseWindow.remove(line);
                }
#endif

                tmpLine.remove(l3);
                if (l3.zStatus() == 20) tmpLine.remove(m3);
                continue;
              }

              //...Make a segment
              unsigned ntmp = tmpLine.length();
              if (ntmp < 3) { //...Is this needed???...
                std::cout << "!!! is this possible !!!???" << std::endl;

                tmpLine.remove(l3);
                if (l3.zStatus() == 20) tmpLine.remove(m3);
                continue;
              }

              //...Do the linear fit
              TLine0 line(tmpLine);
              int err = line.fit();
              if (err < 0) {
#ifdef TRASAN_DEBUG_DETAIL
                std::cout << "    ... line fit error";
                std::cout << " : segment rejected" << std::endl;
                TLink::dump(line.links(), "detail stereo", "    ");
                if (display) {
//          baseWindow.append(line);
//          baseWindow.draw();
//          baseWindow.wait();
//          baseWindow.remove(line);
                }
#endif

                tmpLine.remove(l3);
                if (l3.zStatus() == 20) tmpLine.remove(m3);
                continue;
              }

              //...Check quality for the fit
              //... if |z0| > 70 or chi2 > 0.7, remove it
              //              double chi2 = line.chi2()/(ntmp-2);
              double chi2 = line.reducedChi2();
              if (fabs(line.b()) < _stereoZ3 &&
                  chi2 < _stereoChisq3 &&
                  chi2 < min_chi2[isl]) {
                goodLine = tmpLine;
                min_chi2[isl] = chi2;
                min_a[isl] = line.a();
#ifdef TRASAN_DEBUG_DETAIL
                std::cout << "    ... segment accepted : " << std::endl;
#endif
              }
#ifdef TRASAN_DEBUG_DETAIL
              else {
                std::cout << "    ... bad quality : segment rejected :";
                std::cout << "chi2=" << chi2;
                std::cout << ", b=" << line.b() << std::endl;
              }
              TLink::dump(line.links(), "detail stereo", "    ");
              if (display) {
//            baseWindow.append(line);
//            baseWindow.draw();
//            baseWindow.wait();
//            baseWindow.remove(line);
              }
#endif

              //...end of third loop
              if (l3.zStatus() == 20) tmpLine.remove(m3);
              tmpLine.remove(l3);
            }
            //... end of second loop
            if (l2.zStatus() == 20) tmpLine.remove(m2);
            tmpLine.remove(l2);
          }
          //... end of first loop
          if (l.zStatus() == 20) tmpLine.remove(m);
          tmpLine.remove(l);
        }

        //... Keep best segments
        if (isl == 0) gdSLine0 = goodLine;
        if (isl == 1) gdSLine1 = goodLine;
        goodLine.removeAll();

      } else {

        //... Nlayer == 4

        //...Initialize
        // unsigned ily1 = firstStlayer(isl);
        unsigned ily1 = (* Belle2::TRGCDC::getTRGCDC("1.0")->superLayer(isl * 2 + 1))[0]
                        ->axialStereoLayerId();
//      unsigned ily1 = TRGCDC::getTRGCDC("1.0")->superLayer(isl * 2 + 1)
//    ->first()->axialStereoLayerId();
        unsigned ily2 = ily1 + 1;
        unsigned ily3 = ily2 + 1;
        unsigned ily4 = ily3 + 1;
        if (nlyr[ily1] == 0 ||
            nlyr[ily2] == 0 ||
            nlyr[ily3] == 0 ||
            nlyr[ily4] == 0) continue;

        //...loop for the first layer
        unsigned ilst = llyr[ily1] + 1;
        unsigned ifst = ilst - nlyr[ily1];
        for (unsigned i = ifst; i < ilst; i++) {
          TLink& l = * forLine[i];
          tmpLine.append(l);
          TLink& m = *forLine[l.zPair()];
          if (l.zStatus() == 20) {
            if ((unsigned) l.zPair() < i) {
              tmpLine.append(m);
            } else {
              tmpLine.remove(l);
              continue;
            }
          }

          //...loop for the second layer
          unsigned jlst = llyr[ily2] + 1;
          unsigned jfst = jlst - nlyr[ily2];
          for (unsigned j = jfst; j < jlst; j++) {
            TLink& l2 = * forLine[j];
            tmpLine.append(l2);
            TLink& m2 = *forLine[l2.zPair()];
            if (l2.zStatus() == 20) {
              if ((unsigned) l2.zPair() < j) {
                tmpLine.append(m2);
              } else {
                tmpLine.remove(l2);
                continue;
              }
            }

            //...loop for the third layer
            unsigned klst = llyr[ily3] + 1;
            unsigned kfst = klst - nlyr[ily3];
            for (unsigned k = kfst; k < klst; k++) {
              TLink& l3 = * forLine[k];
              tmpLine.append(l3);
              TLink& m3 = *forLine[l3.zPair()];
              if (l3.zStatus() == 20) {
                if ((unsigned) l3.zPair() < k) {
                  tmpLine.append(m3);
                } else {
                  tmpLine.remove(l3);
                  continue;
                }
              }

              //...loop for the 4th layer
              unsigned hlst = llyr[ily4] + 1;
              unsigned hfst = hlst - nlyr[ily4];
              for (unsigned h = hfst; h < hlst; h++) {
                TLink& l4 = * forLine[h];
                tmpLine.append(l4);
                TLink& m4 = *forLine[l4.zPair()];
                if (l4.zStatus() == 20) {
                  if ((unsigned) l4.zPair() < h) {
                    tmpLine.append(m4);
                  } else {
                    tmpLine.remove(l4);
                    continue;
                  }
                }

                //...Check the relation between the hit
                //   in neighbor wirelayer
                //...
                //...    x|o|o|x      |
                //...      |o|        |
                //...                 V IP ,   o means OK.
                //...
                int relation12 = -1;
                int relation23 = -1;
                int relation34 = -1;

//...For debug...
//          if (l.hit()->wire().consective(* l2.hit()->wire())
//...For debug end...


                //... Check the hit in fist and it in the second
                relation12 = checkHits(l.hit()->wire().localId(),
                                       l2.hit()->wire().localId(),
                                       isl);
                if (l.zStatus() == 20 && relation12 < 0)
                  relation12 =
                    checkHits(m.hit()->wire().localId(),
                              l2.hit()->wire().localId(),
                              isl);

                //... Check the hit in second  and it in the third
                relation23 = checkHits(l2.hit()->wire().localId(),
                                       l3.hit()->wire().localId(),
                                       isl);
                if (l.zStatus() == 20 && relation23 < 0)
                  relation23 =
                    checkHits(m2.hit()->wire().localId(),
                              l3.hit()->wire().localId(),
                              isl);

                //... Check the hit in second  and it in the forth
                relation34 = checkHits(l3.hit()->wire().localId(),
                                       l4.hit()->wire().localId(),
                                       isl);
                if (l3.zStatus() == 20 && relation34 < 0)
                  relation34 =
                    checkHits(m3.hit()->wire().localId(),
                              l4.hit()->wire().localId(),
                              isl);

                //...remove Bad segments
                if (relation12 || relation23 || relation34) {

#ifdef TRASAN_DEBUG_DETAIL
                  std::cout << "    ... bad relations";
                  std::cout << " : segment rejected";
                  TLink::dump(tmpLine, "detail stereo", "    ");
                  if (display) {
                    TLine0 line(tmpLine);
                    line.fit();
//        int err = line.fit();
//          baseWindow.append(line);
//          baseWindow.draw();
//          baseWindow.wait();
//          baseWindow.remove(line);
                  }
#endif

                  tmpLine.remove(l4);
                  if (l4.zStatus() == 20) tmpLine.remove(m4);
                  continue;
                }

                //...Make a segment
                unsigned ntmp = tmpLine.length();
                if (ntmp < 4) {
                  tmpLine.remove(l4);
                  if (l4.zStatus() == 20) tmpLine.remove(m4);
                  continue;
                }

                //...Do the linear fit
                TLine0 line(tmpLine);
                int err = line.fit();
                if (err < 0) {
#ifdef TRASAN_DEBUG_DETAIL
                  std::cout << "    ... line fit error";
                  std::cout << " : segment rejected" << std::endl;
                  TLink::dump(line.links(), "detail stereo", "    ");
                  if (display) {
//          baseWindow.append(line);
//          baseWindow.draw();
//          baseWindow.wait();
//          baseWindow.remove(line);
                  }
#endif

                  tmpLine.remove(l4);
                  if (l4.zStatus() == 20) tmpLine.remove(m4);
                  continue;
                }

                //...Check qualit of the fit
                //... if |z0| > 20 or chi2 > 0.5, remove it
                double chi2 = line.reducedChi2();
                if (fabs(line.b()) < _stereoZ4 &&
                    chi2 < _stereoChisq4 &&
                    chi2 < min_chi2[isl]) {

                  //...Keep good segments
                  goodLine = tmpLine;
                  min_chi2[isl] = chi2;
                  min_a[isl] = line.a();

#ifdef TRASAN_DEBUG_DETAIL
                  std::cout << "    segment accepted : " << std::endl;
#endif
                }
#ifdef TRASAN_DEBUG_DETAIL
                else {
                  std::cout << "    ... bad quality : segment rejected :";
                  std::cout << " chi2=" << chi2;
                  std::cout << ", b=" << line.b() << std::endl;
                }
                TLink::dump(line.links(), "detail stereo", "    ");
                if (display) {
//            baseWindow.append(line);
//            baseWindow.draw();
//            baseWindow.wait();
//            baseWindow.remove(line);
                }
#endif

                //...end of the 4th loop
                if (l4.zStatus() == 20) tmpLine.remove(m4);
                tmpLine.remove(l4);
              }
              //...end of the third loop
              if (l3.zStatus() == 20) tmpLine.remove(m3);
              tmpLine.remove(l3);
            }
            //...end of the second loop
            if (l2.zStatus() == 20) tmpLine.remove(m2);
            tmpLine.remove(l2);
          }
          //... end of the first loop
          if (l.zStatus() == 20) tmpLine.remove(m);
          tmpLine.remove(l);
        }

        //...
        if (isl == 2) gdSLine2 = goodLine;
        if (isl == 3) gdSLine3 = goodLine;
        if (isl == 4) gdSLine4 = goodLine;
        goodLine.removeAll();
      }
    }


    //... Link segments

    //...Check how many segments are made
    unsigned Nsgmnts[5] = {0, 0, 0, 0, 0};
    Nsgmnts[0] = gdSLine0.length();
    Nsgmnts[1] = gdSLine1.length();
    Nsgmnts[2] = gdSLine2.length();
    Nsgmnts[3] = gdSLine3.length();
    Nsgmnts[4] = gdSLine4.length();

    unsigned NusedSgmnts = 0;
    for (unsigned jsl = 0; jsl < 5; jsl++) {
      if (Nsgmnts[jsl] > 0) NusedSgmnts += 1;
    }

    //...Require at least one Segment
    if (NusedSgmnts == 0) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "... rejected because no segment found" << std::endl;
#endif
      HepAListDeleteAll(forLine);
      return NULL;
    }

    //... Make a line
    AList<TLink> forNewLine;
    forNewLine.append(gdSLine0);
    forNewLine.append(gdSLine1);
    forNewLine.append(gdSLine2);
    forNewLine.append(gdSLine3);
    forNewLine.append(gdSLine4);

    //...
#ifdef TRASAN_DEBUG_DETAIL
    unsigned nNewLine = forNewLine.length();
#endif
    float R = fabs(track.helix().curv());
    TLine0 newLine(forNewLine);

    //...Do linear fit
    int err = newLine.fit();
    //    double newLine_chi2 = newLine.chi2()/(nNewLine - 2);
    double newLine_chi2 = newLine.reducedChi2();
    double newLine_a = newLine.a();

    //...Check the quality of the line.

    //...If chi2 > 0.25 for R > 80.( > 0.8 for R < 80.), refine the line.
    //    if(((R > 80. && newLine_chi2 > 0.25) ||(R < 80. && newLine_chi2 > 0.8))&& NusedSgmnts > 1){
    if (((R > 80. && newLine_chi2 > 4.0) || (R < 80. && newLine_chi2 > 13.0)) && NusedSgmnts > 1) {
      //...Look at difference between the slope of the line and that of segment
      double max_diff_a = 0.;
      unsigned this_sly = 999;
      for (unsigned isl = 0; isl < 5; isl++) {
        if (Nsgmnts[isl] == 0) continue;
        double diff_a = fabs((min_a[isl] - newLine_a) / newLine_a);
        if (diff_a > max_diff_a) {
          max_diff_a = diff_a;
          this_sly = isl;
        }
      }

      //...If max slope diff. > 0.4 for R < 50.(> 0.3 for R < 50), remove it.
      if ((R < 50. && max_diff_a > 0.4) || (R > 50. && max_diff_a > 0.3)) {

        //...clear # of entries in the segement
        Nsgmnts[this_sly] = 0;

        //... remove the worst setment
        if (this_sly == 0) {
          newLine.removeSLY(gdSLine0);
        } else if (this_sly == 1) {
          newLine.removeSLY(gdSLine1);
        } else if (this_sly == 2) {
          newLine.removeSLY(gdSLine2);
        } else if (this_sly == 3) {
          newLine.removeSLY(gdSLine3);
        } else if (this_sly == 4) {
          newLine.removeSLY(gdSLine4);
        }

        //... fit again
        unsigned NnewLine_2 = newLine.links().length();
        if (NnewLine_2 < 3) {
#ifdef TRASAN_DEBUG_DETAIL
          std::cout << "... rejected because of few links for line" << std::endl;
#endif
          HepAListDeleteAll(forLine);
          return NULL;
        }

        int err = newLine.fit();
        if (err < 0) {
#ifdef TRASAN_DEBUG_DETAIL
          std::cout << "... rejected because of line fit failure" << std::endl;
#endif
          HepAListDeleteAll(forLine);
          return NULL;
        }
        // double newLine_chi2_2 = newLine.chi2()/NnewLine_2;
      }
    }

    //...Remove bad points...
    AList<TLink> bad;
    double maxSigma = 1.0;
    if (R < 80) maxSigma = 1.5;
    newLine.refine(bad, maxSigma);
    if (newLine.links().length() < 2) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "... rejected because of few links for line after refine";
      std::cout << std::endl;
#endif
      HepAListDeleteAll(forLine);
      return NULL;
    }
    err = newLine.fit();
    if (err < 0) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "... rejected because of line fit failure(2)" << std::endl;
#endif
      HepAListDeleteAll(forLine);
      return NULL;
    }

    //...Append hits, if the distance between the line and hits <
    for (unsigned isl = 0; isl < 5; isl++) {
      if (Nsgmnts[isl] == 0) {
        double maxdist = 0.5;
        if (R < 80) maxdist = 1.25;
        newLine.appendByszdistance(forLine, 2 * isl + 1, maxdist);
      }
    }
    err = newLine.fit();
    if (err < 0) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "... rejected because of line fit failure(3)" << std::endl;
#endif
      HepAListDeleteAll(forLine);
      return NULL;
    }

    //...Remove bad points again...
    maxSigma = 1.0;
    newLine.refine(bad, maxSigma);
    if (newLine.links().length() < 2) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "... rejected because of few links for line after 2nd refine";
      std::cout << std::endl;
#endif
      HepAListDeleteAll(forLine);
      return NULL;
    }

    //...Linear fit again...
    err = newLine.fit();
    if (err < 0) {
      HepAListDeleteAll(forLine);
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "    appendStereo cut ... new line 2nd linear fit failure. ";
      std::cout << "# of links = " << n << ",";
      std::cout << "," << nNewLine << std::endl;
#endif
      return NULL;
    }

    //...
    // unsigned NnewLine_3 = newLine.links().length();
    // double newLine_chi2_3 = newLine.chi2()/NnewLine_3;

    //... Do we need quality cut?
    //     if(newLine_chi2_3 > 10.) return NULL;

    //...3D fit...
    const AList<TLink> & good = newLine.links();
    unsigned nn = good.length();
    for (unsigned i = 0; i < nn; i++) {
      track.append(* good[i]->link());
    }

    //...Set initial values
    CLHEP::HepVector a(5);
    a = track.helix().a();
    a[3] = newLine.b();
    a[4] = track.charge() * newLine.a();
    track._helix->a(a);

    //...Refine...
    err = fit(track);
    if (err < 0) goto discard;
    track.refine(bad, _stereoMaxSigma * 100.);
    err = fit(track);
    if (err < 0) goto discard;
    track.refine(bad, _stereoMaxSigma * 10.);
    err = fit(track);
    if (err < 0) goto discard;
    track.refine(bad, _stereoMaxSigma);
    err = fit(track);
    if (err < 0) goto discard;

    //...Test it...
    if (! _trackSelector.select(track)) goto discard;

    //...Termination...
    HepAListDeleteAll(forLine);
    return & track;

    //...Something happened...
discard:
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "    ... rejected by fitting failure : ";
    std::cout << " err = " << err << std::endl;
#endif
    HepAListDeleteAll(forLine);
    return NULL;
  }

  const TSelector0&
  TBuilder0::trackSelector(const TSelector0& a)
  {
    _circleSelector.nLinks(a.nLinks());
    _circleSelector.nSuperLayers(a.nSuperLayers());
    _circleSelector.minPt(a.minPt());
    _circleSelector.maxImpact(a.maxImpact());

    _trackSelector.nLinks(a.nLinks());
    _trackSelector.nSuperLayers(a.nSuperLayers());
    _trackSelector.maxSigma(a.maxSigma());

    _lineSelector.nLinksStereo(a.nLinksStereo());
    _lineSelector.maxDistance(a.maxDistance());

    return a;
  }

  void
  TBuilder0::appendClusters(TTrack& track,
                            const AList<TLink> & list) const
  {

    AList<TLink> tmp = list;

    //...Append them...
    track.appendByApproach(tmp, _trackSelector.maxSigma() * 2. / 3.);

    //...Refine it...
    AList<TLink> bad;
    fit(track);
    track.refine(bad, _trackSelector.maxSigma());
  }

  int
  TBuilder0::checkHits(unsigned i, unsigned j, unsigned isl) const
  {

    int nWr[5] = {80, 128, 160, 208, 256};
    int ilast = nWr[isl] - 1;
    int ilocal = (int) i;
    int jlocal = (int) j;
    //...
    if (ilocal > 0 && ilocal < ilast) {
      if (fabs(jlocal - ilocal) > 1) {
        return -1;
      } else {
        return 0;
      }
    } else if (ilocal == 0) {
      if (jlocal > 1 && jlocal < ilast) {
        return -1;
      } else {
        if (jlocal == ilast) {
          return 0;
        } else if (jlocal == 0) {
          return 0;
        } else if (jlocal == 1) {
          return 0;
        } else {
          return -1;
        }
      }
    } else if (ilocal == ilast) {
      if (jlocal > 0 && jlocal < ilast - 1) {
        return -1;
      } else {
        if (jlocal == ilast - 1) {
          return 0;
        } else if (jlocal == ilast) {
          return 0;
        } else if (jlocal == 0) {
          return 0;
        } else {
          return -1;
        }
      }
    }
    //...
    return -1;
  }

  void
  TBuilder0::salvage(TTrack& t, AList<TLink> & hits) const
  {

    if (t.type() == TrackTypeNormal) {
      salvageNormal(t, hits);
      return;
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << name() << " ... salvaging" << std::endl;
    std::cout << "    # of given hits=" << hits.length() << std::endl;
#endif

    unsigned nHits = hits.length();
    if (nHits == 0) return;

    //...Hit loop...
    AList<TLink> candidates;
    for (unsigned i = 0; i < nHits; i++) {
      TLink& l = * hits[i];
      const Belle2::TRGCDCWireHit& h = * l.hit();

      //...Already used?...
      if (h.state() & CellHitUsed) continue;
      candidates.append(l);
    }

    //...Try to append this hit...
    // t.appendByApproach(candidates, 10.);
    t.appendByApproach(candidates, _salvageLevel);
    fit(t);
    hits.remove(candidates);
    t.assign(CellHitConformalFinder);
    t.finder(TrackOldConformalFinder);
    // t.assign(CellHitConformalFinder, TrackOldConformalFinder);
  }

  void
  TBuilder0::salvageNormal(TTrack& t, AList<TLink> & hits) const
  {

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << name() << " ... normal salvaging : ";
    std::cout << " # of hits=" << hits.length() << std::endl;
#endif

    unsigned nHits = hits.length();
    if (nHits == 0) return;

    //...Determine direction to salvage...
    const HepGeom::Point3D<double> & center = t.helix().center();
    const Vector3D a = ORIGIN - center;

    //...Hit loop...
    AList<TLink> candidates;
    for (unsigned i = 0; i < nHits; i++) {
      TLink& l = * hits[i];
      const Belle2::TRGCDCWireHit& h = * l.hit();
      if (a.cross(h.xyPosition()).z() * t.charge() > 0.) continue;

      //...Already used?...
      if (h.state() & CellHitUsed) continue;
      candidates.append(l);
    }

    //...Try to append this hit...
    t.appendByApproach(candidates, 30.);
    hits.remove(candidates);
    t.assign(CellHitConformalFinder);
    t.finder(TrackOldConformalFinder);
    // t.assign(CellHitConformalFinder, TrackOldConformalFinder);
  }

  int
  TBuilder0::check2CnHits(TLink& l, TLink& s, int ichg) const
  {

    //...Check same layer ?...
    if (l.hit()->wire().layerId() != s.hit()->wire().layerId()) return -1;

    //...Initialization...
    int nsl[11] = {64, 80, 96, 128, 144, 160, 192, 208, 240, 256, 288};
    float hcell[50] = {0., 0., 0., 0., 0., 0., 0.687499, 0.747198, 0.806896, 0., 0., 0., 0., 0., 0., 0.782967, 0.820598, 0.858229, 0., 0., 0., 0., 0., 0.878423, 0.908646, 0.939845, 0.970068, 0., 0., 0., 0., 0., 0.892908, 0.916188, 0.940219, 0.963499, 0., 0., 0., 0., 0., 0.901912, 0.920841, 0.940382, 0.959312, 0., 0., 0., 0., 0.};

    int ilast  = nsl[l.hit()->wire().superLayerId()] - 1;
    int ilocal = l.hit()->wire().localId();
    int jlocal = s.hit()->wire().localId();

    double ddist1 = l.hit()->drift() / hcell[l.hit()->wire().layerId()];
    double ddist2 = s.hit()->drift() / hcell[s.hit()->wire().layerId()];
    double ddist = 0.5 * (ddist1 + ddist2);

    //...Case by case...
    if (ilocal > 0 && ilocal < ilast) {
      if (abs(jlocal - ilocal) > 1) {
        return -20;
      } else {
        if (ddist > 0.65  &&
            ((ddist1 > 0.7 && ddist2 > 0.7) ||
             (ddist1 > 0.95 || ddist2 > 0.95))) {

          //...O.K. 2 consective hits
          if (ichg > 0) {
            return 1;
          } else {
            return 0;
          }
        } else {
          return -20;
        }
      }
    } else if (ilocal == 0) {
      if (jlocal > 1 && jlocal < ilast) {
        return -20;
      } else {
        if (ddist > 0.65 &&
            ((ddist1 > 0.7 && ddist2 > 0.7) ||
             (ddist1 > 0.95 || ddist2 > 0.95))) {
          if (jlocal == ilast) {

            //...O.K. 2 consective hits
            if (ichg > 0) {
              return 0;
            } else {
              return 1;
            }
          } else if (jlocal == 1) {
            if (ichg > 0) {
              return 1;
            } else {
              return 0;
            }
          }
        }
      }
    } else if (ilocal == ilast) {
      if (jlocal > 0 && jlocal < ilast - 1) {
        return -20;
      } else {
        if (ddist > 0.65 &&
            ((ddist1 > 0.7 && ddist2 > 0.7) ||
             (ddist1 > 0.95 || ddist2 > 0.95))) {
          if (jlocal == ilast - 1) {
            //...O.K. 2 consective hits
            if (ichg > 0) {
              return 0;
            } else {
              return 1;
            }
          } else if (jlocal == 0) {
            if (ichg > 0) {
              return 1;
            } else {
              return 0;
            }
          }
        } else {
          return -20;
        }
      }
    }

    //...fails
    return -50;
  }

  int
  TBuilder0::consectiveHits(TLink& l, TLink& s, int ichg) const
  {

    //...Diagonal length of a cell...
    static float hcell[50] = {0., 0., 0., 0., 0., 0.,
                              0.687499, 0.747198, 0.806896,
                              0., 0., 0., 0., 0., 0.,
                              0.782967, 0.820598, 0.858229,
                              0., 0., 0., 0., 0.,
                              0.878423, 0.908646, 0.939845, 0.970068,
                              0., 0., 0., 0., 0.,
                              0.892908, 0.916188, 0.940219, 0.963499,
                              0., 0., 0., 0., 0.,
                              0.901912, 0.920841, 0.940382, 0.959312,
                              0., 0., 0., 0., 0.
                             };
    const Belle2::TRGCDCWire& wire = l.hit()->wire();
    const Belle2::TRGCDCWire& next = s.hit()->wire();

    //...Check same layer ?...
    if (wire.layerId() != next.layerId()) return -1;

    //...Are these consective ?...
    if (! wire.consective(next)) return -20;

    //...Drift distance...
    // int ilast  = wire.layer()->nWires() - 1;
    unsigned layerId = wire.layerId();
    double ddist1 = l.hit()->drift() / hcell[layerId];
    double ddist2 = s.hit()->drift() / hcell[layerId];
    double ddist = 0.5 * (ddist1 + ddist2);

    //...Distance check...
    if (ddist > 0.65  &&
        ((ddist1 > 0.7 && ddist2 > 0.7) || (ddist1 > 0.95 || ddist2 > 0.95))) {

      //...O.K. 2 consective hits
      if (ichg > 0) return 1;
      else          return 0;
    } else {
      return -50;
    }
  }

} // namespace Belle

