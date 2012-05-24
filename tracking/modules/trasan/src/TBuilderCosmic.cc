//-----------------------------------------------------------------------------
// $Id: TBuilderCosmic.cc 10129 2007-05-18 12:44:41Z katayama $
//-----------------------------------------------------------------------------
// Filename : TBuilderCosmic.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to build a cosmic track.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.13  2005/03/11 03:57:47  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.12  2003/12/25 12:03:30  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.11  2001/12/23 09:58:44  katayama
// removed Strings.h
//
// Revision 1.10  2001/12/19 02:59:43  katayama
// Uss find,istring
//
// Revision 1.9  2001/04/11 01:09:07  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.8  2000/01/08 09:44:37  yiwasaki
// Trasan 1.66d : debug info. modified
//
// Revision 1.7  1999/11/19 09:13:05  yiwasaki
// Trasan 1.65d : new conf. finder updates, no change in default conf. finder
//
// Revision 1.6  1999/10/30 10:12:10  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.5  1999/03/21 15:45:38  yiwasaki
// Trasan 1.28 release : TrackManager bug fix, CosmicFitter added in BuilderCosmic, parameter salvage level added
//
// Revision 1.4  1999/02/21 13:08:53  yiwasaki
// Trasan 1.18 : Cosmic builder loose cut
//
// Revision 1.3  1999/01/11 03:03:06  yiwasaki
// Fitters added
//
// Revision 1.2  1998/11/27 08:15:25  yiwasaki
// Trasan 1.1 RC 3 release : salvaging improved
//
// Revision 1.1  1998/09/28 16:52:10  yiwasaki
// TBuilderCosmic added
//
//-----------------------------------------------------------------------------



#include "tracking/modules/trasan/TBuilderCosmic.h"
#include "tracking/modules/trasan/TLink.h"
#include "tracking/modules/trasan/TTrack.h"
#include "tracking/modules/trasan/TLine0.h"

namespace Belle {

  TBuilderCosmic::TBuilderCosmic(const std::string& name, float salvageLevel)
    : TBuilder0(name, salvageLevel), _fitter("TBuilderCosmic Fitter")
  {
  }

  TBuilderCosmic::~TBuilderCosmic()
  {
  }

  TTrack*
  TBuilderCosmic::buildStereo(TTrack& track, const AList<TLink> & list) const
  {
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << name() << "(stereo) ... dump of stereo candidate hits" << std::endl;
    AList<TLink> tmp = list;
    tmp.sort(TLink::sortByWireId);
    std::cout << "    ";
    for (unsigned i = 0; i < (unsigned) tmp.length(); i++) {
      TLink* l = tmp[i];
      std::cout << l->wire()->layerId() << "-";
      std::cout << l->wire()->localId() << ",";
    }
    std::cout << std::endl;
#endif

    //...Check # of links...
    if ((unsigned) list.length() < _lineSelector.nLinksStereo()) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << name() << "(stereo) ... rejected by nLinks(";
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

      //... Require Fitting vaildation
      if (!(l->hit()->state()& CellHitFittingValid)) continue;

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
    std::cout << name() << "(stereo) ... dump of sz links" << std::endl;
    std::cout << "    ";
    tmp = forLine;
    tmp.sort(TLink::sortByWireId);
    for (unsigned i = 0; i < (unsigned) tmp.length(); i++) {
      TLink* l = tmp[i];
      std::cout << l->wire()->layerId() << "-";
      std::cout << l->wire()->localId() << ",";
    }
    std::cout << std::endl;
#endif

    //...Check # of sz links...
    if ((unsigned) forLine.length() < _lineSelector.nLinksStereo()) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << name() << "(stereo) ... rejected by sz nLinks(";
      std::cout << forLine.length() << ") < ";
      std::cout << _lineSelector.nLinks() << std::endl;
#endif
      HepAListDeleteAll(forLine);
      return NULL;
    }

    //...Make a line...
    unsigned nLine = forLine.length();
    TLine0 line(forLine);
    int err = line.fit();

    //...Linear fit...
    if (err < 0) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << name() << "(stereo) ... linear fit failure. nLinks(";
      std::cout << forLine.length() << ")" << std::endl;
#endif
      HepAListDeleteAll(forLine);
      return NULL;
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << name() << "(stereo) ... dump of left-right" << std::endl;
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
      std::cout << name() << "(stereo) ... rejected by lr nLinks(";
      std::cout << forNewLine.length() << ") < ";
      std::cout << _lineSelector.nLinks() << std::endl;
#endif
      HepAListDeleteAll(forLine);
      HepAListDeleteAll(forNewLine);
      return NULL;
    }

    //...Create new line...
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << name() << "(stereo) ... creating a new line" << std::endl;
#endif
//cnv    unsigned nNewLine = forNewLine.length();
    TLine0 newLine(forNewLine);

    //...Make a seed track again
    err = newLine.fit();

    //...Linear fit...
    if (err < 0) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << name() << "(stereo) ... 2nd linear fit failure. nLinks(";
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
    newLine.refine(bad, 20.);
    err = newLine.fit();
    newLine.refine(bad, 10.);
    err = newLine.fit();

    //...Linear fit again...
    if (err < 0) {
      HepAListDeleteAll(forLine);
      HepAListDeleteAll(forNewLine);
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "    appendStereo cut ... new line 2nd linear fit failure. ";
      std::cout << "# of links = " << n << "," << nLine;
//  std::cout << "," << nNewLine << std::endl;
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
    a[3] = newLine.b();
    a[4] = track.charge() * newLine.a();
    track._helix->a(a);

    //...Refine...
    err = _fitter.fit(track);
    track.refine(bad, _trackSelector.maxSigma() * 30.);
    err = _fitter.fit(track);
    track.refine(bad, _trackSelector.maxSigma() * 3.);
    err = _fitter.fit(track);
    track.refine(bad, _trackSelector.maxSigma() * 0.21);
    err = _fitter.fit(track);

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

} // namespace Belle

