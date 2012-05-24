//-----------------------------------------------------------------------------
// $Id: TBuilderConformal.cc 10305 2007-12-05 05:19:24Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : TBuilderConfrmal.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to build a track in Conformal finder.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.2  2005/03/11 03:57:46  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.1  2004/03/26 06:17:35  yiwasaki
// Trasan 3.10 : new files
//
//-----------------------------------------------------------------------------
#include <string>
#include <exception>


#include "tracking/modules/trasan/TBuilderConformal.h"
#include "tracking/modules/trasan/TSegment.h"
#include "tracking/modules/trasan/TLine.h"
#include "tracking/modules/trasan/TCircle.h"
#include "tracking/modules/trasan/TTrack.h"
#include "tracking/modules/trasan/TRobustLineFitter.h"
#ifdef TRASAN_DEBUG
#include "tracking/modules/trasan/TDebugUtilities.h"
#endif
#ifdef TRASAN_WINDOW
#include "tracking/modules/trasan/TWindow.h"
TWindow sz2("sz2");
#endif
#ifdef TRASAN_WINDOW_GTK
#include "tracking/modules/trasan/TWindowGTK.h"
#endif

namespace Belle {

  TBuilderConformal::TBuilderConformal(const std::string& a,
                                       float maxSigma,
                                       float maxSigmaStereo,
                                       float salvageLevel,
                                       float szSegmentDistance,
                                       float szLinkDistance,
                                       unsigned fittingFlag)
    : TBuilder(a,
               maxSigma,
               maxSigmaStereo,
               salvageLevel,
               szLinkDistance,
               fittingFlag),
    _szSegmentDistance(szSegmentDistance)
  {
  }

  TBuilderConformal::~TBuilderConformal()
  {
  }

// TTrack *
// TBuilderConformal::buildRphi(const AList<TSegment> & list) const {

// #ifdef TRASAN_DEBUG_DETAIL
// std::cout << "... building rphi by segments : # of segments = ";
// std::cout << list.length() << std::endl;
//     for (unsigned i = 0; i < (unsigned) list.length(); i++)
//  list[i]->dump("hits sort flag", "    ");
// #endif

//     //...Pick up links...
//     AList<TLink> links = Links(list);

//     //...Main funtion...
//     TTrack * t = TBuilder::buildRphi(links);

//     //...Check used segments...
//     if (t) {
// // const AList<TLink> & usedLinks = t->links();
//  unsigned n = list.length();
//  for (unsigned i = 0; i < n; i++) {
//      TSegment & segment = * list[i];
//      AList<TLink> used = Links(segment, * t);
//      if (used.length()) {
//    t->segments().append(segment);
//    segment.tracks().append(t);
//      }
//  }
//     }

//     return t;
// }

  TTrack*
  TBuilderConformal::buildStereo(const TTrack& t0,
                                 AList<TSegment> & segments) const
  {

    TTrack& t = * new TTrack(t0);

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "... building stereo by segments : # of segments = ";
    std::cout << segments.length() << std::endl;
    for (unsigned i = 0; i < (unsigned) segments.length(); i++)
      segments[i]->dump("hits sort flag", "    ");
#endif
#ifdef TRASAN_WINDOW
    sz2.clear();
    sz2.skip(false);
    sz2.mode(2);
    sz2.appendSz(t, segments, leda_black);
    AList<TSegment> tmps = segments;
    std::string s = "# of segments = " + itostring(int(segments.length()));
#endif

    //...Find initial line...
    TLine* line = initialLineOld(t, segments);
    if (! line) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "... building stereo failure : no initial line found" << std::endl;
#endif
#ifdef TRASAN_WINDOW
      s = "no initial line found : " + s;
      sz2.text(s);
      sz2.wait();
#endif
      return NULL;
    }
#ifdef TRASAN_WINDOW
    sz2.append(* line, leda_red);
    sz2.text(s);
    sz2.wait();
#endif

    //...Pick up links...
    AList<TLink> links;
    unsigned n = segments.length();
    for (unsigned i = 0; i < n; i++)
      links.append(segments[i]->links());

    //...Main funtion...
    TTrack* ts = buildStereo(t, * line, links);

    //...Check used segments...
    if (ts) {
      AList<TLink> usedLinks = TLink::stereoHits(t.links());
      for (unsigned i = 0; i < n; i++) {
        TSegment& segment = * segments[i];
        AList<TLink> used = Links(segment, t);
        if (used.length()) {
          t.segments().append(segment);
          segment.tracks().append(t);
        }
      }
    }

    HepAListDeleteAll((AList<TLink> &) line->links());
    delete line;
    return ts;
  }

  TTrack*
  TBuilderConformal::buildStereoNew(const TTrack& t,
                                    AList<TSegment> & segments,
                                    AList<TSegment> & badSegments) const
  {

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "... building stereo by segments(new) : # of segments = ";
    std::cout << segments.length() << std::endl;
    for (unsigned i = 0; i < (unsigned) segments.length(); i++)
      segments[i]->dump("hits sort flag", "    ");
#endif
#ifdef TRASAN_WINDOW
    sz2.clear();
#endif
    TTrack* bestCandidate = NULL;
    AList< AList<TLink> > poorSeeds;
    bool ok = initializeForStereo(t, segments, badSegments);
    unsigned nSuperLayers = _nSuperLayers + 1;
    if (! ok) goto endOfBuilding;

    //...Main loop...
    while (--nSuperLayers) {

      //...Initial line search...
      AList<TLine> initialLines = searchInitialLines(nSuperLayers);
#ifdef TRASAN_WINDOW
      sz2.clear();
      sz2.skip(false);
      sz2.mode(2);
      sz2.appendSz(t, segments, leda_black);
      AList<TSegment> tmps = segments;
      std::string s = "# of segments = " + itostring(int(segments.length()));
      sz2.appendSz(t, _allLinks, leda_black);
      sz2.append(_forLine, leda_brown);
      s = "nSprLyr=" + itostring(int(nSuperLayers)) +
          ", # of initial lines = " + itostring(int(initialLines.length()));
      for (unsigned i = 0; i < (unsigned) initialLines.length(); i++)
        sz2.append(* initialLines[i], leda_red);
      sz2.text(s);
      sz2.wait();
#endif
      if (initialLines.length() == 0) continue;

      //...Line loop...
      bool found = false;
      unsigned nInitialLines = initialLines.length();
      for (unsigned i = 0; i < nInitialLines; i++) {

        //...Linear fit...
        const TLine& line = * initialLines[i];
        TLine newLine = searchLine(line);

        //...Skip if this is a seed of the poor result...
        if (poorSeeds.length()) {
          bool poorCase = false;
          for (unsigned j = 0; j < (unsigned) poorSeeds.length(); j++) {
            if (poorSeeds[j]->length() == newLine.links().length()) {
              AList<TLink> tmp = * poorSeeds[j];
              tmp.remove(newLine.links());
              if (tmp.length() == 0) {
#ifdef TRASAN_DEBUG_DETAIL
                std::cout << "    ... This is a poor seed :"
                          << " skipped"
                          << " : # of poor seeds = "
                          << poorSeeds.length() << std::endl;
#endif
                poorCase = true;
                break;
              }
            }
          }
          if (poorCase) continue;
        }

        //...Is this a good line...
        if (! stereoQuality(newLine.links()))
          continue;

        //...3D fit...
        TTrack* t3d = new TTrack(t);
        t3d = build(* t3d, newLine);
        if (t3d == NULL) continue;

        //...Check quality...
        unsigned quality = stereoQuality(t3d->links());

        //...Best case...
        if (quality == 2) {
#ifdef TRASAN_WINDOW
          sz2.text("stereo finished");
          sz2.oneShot(* t3d, leda_blue);
#endif
          if (bestCandidate) delete bestCandidate;
          bestCandidate = t3d;
          found = true;
          break;
        }

        //...Poor case...
        AList<TLink> * tmp = new AList<TLink>();
        tmp->append(newLine.links());
        poorSeeds.append(tmp);
        if (quality == 0) {
#ifdef TRASAN_WINDOW
          sz2.text("this candidate discarded");
          sz2.oneShot(* t3d, leda_black);
#endif
          delete t3d;
          continue;
        }

        //...Not enough...
        if (bestCandidate) {
          if (bestCandidate->cores().length() <
              t3d->cores().length()) {
#ifdef TRASAN_WINDOW
            sz2.text("new candidate");
            sz2.append(* bestCandidate, leda_brown);
            sz2.oneShot(* t3d, leda_green);
            sz2.remove(* bestCandidate);
#endif
            delete bestCandidate;
            bestCandidate = t3d;
          } else {
#ifdef TRASAN_WINDOW
            sz2.text("this candidate discarded");
            sz2.oneShot(* t3d, leda_black);
#endif
            delete t3d;
          }
        } else {
          bestCandidate = t3d;
#ifdef TRASAN_WINDOW
          sz2.text("new candidate");
          sz2.oneShot(* bestCandidate, leda_green);
          sz2.remove(* t3d);
#endif
        }
      }

      //...Termination of a loop...
      HepAListDeleteAll(initialLines);
      if (found) break;
    }

endOfBuilding:
    _allLinks.removeAll();
    for (unsigned i = 0; i < 5; i++)
      HepAListDeleteAll(_links[i]);
    HepAListDeleteAll(_forLine);

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "    ... # of poor seeds = " << poorSeeds.length() << std::endl;
#endif
#ifdef TRASAN_WINDOW
    if (bestCandidate == NULL) {
      sz2.text("3D failed");
      sz2.wait();
    }
#endif

    //...Check used segments...
    if (bestCandidate) {
      AList<TLink> usedLinks = TLink::stereoHits(bestCandidate->links());
      for (unsigned i = 0; i < (unsigned) segments.length(); i++) {
        TSegment& segment = * segments[i];
        AList<TLink> used = Links(segment, * bestCandidate);
        if (used.length()) {
          bestCandidate->segments().append(segment);
          segment.tracks().append(bestCandidate);
        }
      }
    }

    if (poorSeeds.length())
      HepAListDeleteAll(poorSeeds);

#ifdef TRASA_DEBUG_DETAIL
    if (bestCandidate == NULL)
      std::cout << "... building stereo(new) failed" << std::endl;
    else
      std::cout << "... building stereo(new) ok" << std::endl;
#endif
    return bestCandidate;
  }

  bool
  TBuilderConformal::initializeForStereo(const TTrack& t,
                                         const AList<TSegment> & segments,
                                         const AList<TSegment> & badSegments) const
  {
    _nSuperLayers = 0;
    for (unsigned i = 0; i < 6; i++)
      _nHits[i] = 0;

    //...sz position of segments...
    unsigned nSegments = segments.length();
    for (unsigned i = 0; i < nSegments; i++) {
      TLink& l = * new TLink();
      int err = t.szPosition(* segments[i], l);

      //...Remove if sz can not be calculcated...
      if (err) {
        delete & l;
        continue;
      }
      _links[l.wire()->superLayerId() / 2].append(l);
//? _allLinks.append(segments[i]->links());
      _allLinks.append(segments[i]->cores());
    }

    //...Count a number of super layers...
    for (unsigned i = 0; i < 5; i++) {
      if (_links[i].length() > 0) {
        ++_nSuperLayers;
      }
//      else {
//          _allLinks.append(badSegments[i]->links());
//  #ifdef TRASAN_DEBUG_DETAIL
//        std::cout << "    ... stereo super layer " << i * 2 + 1
//       << " has no link,"
//       << badSegments[i]->links().length() << " (bad) links added"
//       << std::endl;
//        badSegments[i]->dump("hits sort flag", "        ");
//  #endif
//      }
    }

    //...Append bad links also...
    if (badSegments.length()) {
      for (unsigned i = 0; i < 5; i++) {
        if (badSegments[i]->links().length()) {
          _allLinks.append(badSegments[i]->links());
#ifdef TRASAN_DEBUG_DETAIL
          std::cout << "    ... bad links added for stereo super layer "
                    << i * 2 + 1 << std::endl;
          badSegments[i]->dump("hits sort flag", "        ");
#endif
        }
      }
    }

    //...sz position of links...
    unsigned nCores = _allLinks.length();
    if (nCores < minNCores()) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "    ... initializeForStereo : # of cores(=" << nCores
                << ") is less then " << minNCores() << std::endl;
#endif
      return false;
    }
    for (unsigned i = 0; i < nCores; i++) {
      TLink& link = * _allLinks[i];
      for (unsigned i = 0; i < 2; i++) {
        TLink& tt = * new TLink(link);
        tt.leftRight(i);
        int err = t.szPosition(tt);
        if (err) {
          delete & tt;
          continue;
        }
        tt.link(& link);
        _forLine.append(tt);
      }
    }

    //...Count # of axial hits...
    unsigned nA = t.cores().length();
    for (unsigned i = 0; i < nA; i++)
      ++_nHits[t.cores()[i]->wire()->superLayerId() / 2];

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "    ... initializeForStereo : axial super layer usage = ";
    for (unsigned i = 0; i < 6; i++)
      std::cout << _nHits[i] << ",";
    std::cout << std::endl
              << "                            : # of stereo super layers="
              << _nSuperLayers << std::endl;
#endif

    return true;
  }

  AList<TLine>
  TBuilderConformal::searchInitialLines(unsigned nSuperLayerMax) const
  {
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "    ... searchInitialLines : # of segments in sl : ";
    for (unsigned i = 0; i < 5; i++)
      std::cout << _links[i].length() << ",";
    std::cout << std::endl
              << "                           : max # of super layers="
              << nSuperLayerMax << std::endl;
#endif

    AList<TLine> lines;
    if (nSuperLayerMax > 4)
      lines.append(searchLines5());
    else if (nSuperLayerMax > 3)
      lines.append(searchLines4());
    else if (nSuperLayerMax > 2)
      lines.append(searchLines3());
    else if (nSuperLayerMax > 1)
      lines.append(searchLines2());
//      else
//    lines.append(searchLines1());

    lines.sort(SortByB);
    return lines;
  }

  AList<TLine>
  TBuilderConformal::searchLines5(void) const
  {
    unsigned n[5];
    for (unsigned i = 0; i < 5; i++)
      n[i] = _links[i].length();

    AList<TLine> lines;
    for (unsigned i0 = 0; i0 < n[0]; i0++) {
      for (unsigned i1 = 0; i1 < n[1]; i1++) {
        for (unsigned i2 = 0; i2 < n[2]; i2++) {
          for (unsigned i3 = 0; i3 < n[3]; i3++) {
            for (unsigned i4 = 0; i4 < n[4]; i4++) {

              AList<TLink> forLine;
              forLine.append(_links[0][i0]);
              forLine.append(_links[1][i1]);
              forLine.append(_links[2][i2]);
              forLine.append(_links[3][i3]);
              forLine.append(_links[4][i4]);

              TLine& line = * new TLine(forLine);
              int err = line.fit();
              if (err) {
                delete & line;
                continue;
              }

              lines.append(line);
            }
          }
        }
      }
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "        ... searchLines5 : # of lines found = "
              << lines.length() << std::endl;
#endif

    return lines;
  }

  AList<TLine>
  TBuilderConformal::searchLines4(void) const
  {
    AList<TLine> lines;

    const AList<TLink> * l[4];
    for (unsigned skip = 0; skip < 5; skip++) {
      if (skip == 0) {
        l[0] = & _links[1];
        l[1] = & _links[2];
        l[2] = & _links[3];
        l[3] = & _links[4];
      } else if (skip == 1) {
        l[0] = & _links[0];
      } else if (skip == 2) {
        l[1] = & _links[1];
      } else if (skip == 3) {
        l[2] = & _links[2];
      } else if (skip == 4) {
        l[3] = & _links[3];
      }

      unsigned n[4];
      for (unsigned i = 0; i < 4; i++)
        n[i] = l[i]->length();

      for (unsigned i0 = 0; i0 < n[0]; i0++) {
        for (unsigned i1 = 0; i1 < n[1]; i1++) {
          for (unsigned i2 = 0; i2 < n[2]; i2++) {
            for (unsigned i3 = 0; i3 < n[3]; i3++) {
              AList<TLink> forLine;
              forLine.append((* l[0])[i0]);
              forLine.append((* l[1])[i1]);
              forLine.append((* l[2])[i2]);
              forLine.append((* l[3])[i3]);

              TLine& line = * new TLine(forLine);
              int err = line.fit();
              if (err) {
                delete & line;
                continue;
              }

              lines.append(line);
            }
          }
        }
      }
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "        ... searchLines4 : # of lines found = "
              << lines.length() << std::endl;
#endif

    return lines;
  }

  AList<TLine>
  TBuilderConformal::searchLines3(void) const
  {
    AList<TLine> lines;

    const AList<TLink> * l[3];
    for (unsigned i = 0; i < 10; i++) {
      if (i == 0) {
        l[0] = & _links[0];
        l[1] = & _links[1];
        l[2] = & _links[2];
      } else if (i == 1) {
        l[2] = & _links[3];
      } else if (i == 2) {
        l[2] = & _links[4];
      } else if (i == 3) {
        l[2] = & _links[3];
      } else if (i == 4) {
        l[1] = & _links[2];
        l[2] = & _links[4];
      } else if (i == 5) {
        l[1] = & _links[3];
      } else if (i == 6) {
        l[0] = & _links[1];
        l[1] = & _links[2];
        l[2] = & _links[3];
      } else if (i == 7) {
        l[2] = & _links[4];
      } else if (i == 8) {
        l[1] = & _links[3];
      } else if (i == 9) {
        l[0] = & _links[2];
        l[2] = & _links[4];
      }

      unsigned n[3];
      for (unsigned i = 0; i < 3; i++)
        n[i] = l[i]->length();

      for (unsigned i0 = 0; i0 < n[0]; i0++) {
        for (unsigned i1 = 0; i1 < n[1]; i1++) {
          for (unsigned i2 = 0; i2 < n[2]; i2++) {
            AList<TLink> forLine;
            forLine.append((* l[0])[i0]);
            forLine.append((* l[1])[i1]);
            forLine.append((* l[2])[i2]);

            TLine& line = * new TLine(forLine);
            int err = line.fit();
            if (err) {
              delete & line;
              continue;
            }

            lines.append(line);
          }
        }
      }
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "        ... searchLines3 : # of lines found = "
              << lines.length() << std::endl;
#endif

    return lines;
  }

  AList<TLine>
  TBuilderConformal::searchLines2(void) const
  {
    AList<TLine> lines;

    const AList<TLink> * l[2];
    for (unsigned i = 0; i < 10; i++) {
      if (i == 0) {
        l[0] = & _links[0];
        l[1] = & _links[1];
      } else if (i == 1) {
        l[1] = & _links[2];
      } else if (i == 2) {
        l[1] = & _links[3];
      } else if (i == 3) {
        l[1] = & _links[4];
      } else if (i == 4) {
        l[0] = & _links[1];
        l[1] = & _links[2];
      } else if (i == 5) {
        l[1] = & _links[3];
      } else if (i == 6) {
        l[1] = & _links[4];
      } else if (i == 7) {
        l[0] = & _links[2];
        l[1] = & _links[3];
      } else if (i == 8) {
        l[1] = & _links[4];
      } else if (i == 9) {
        l[0] = & _links[3];
        l[1] = & _links[4];
      }

      unsigned n[2];
      for (unsigned i = 0; i < 2; i++)
        n[i] = l[i]->length();

      for (unsigned i0 = 0; i0 < n[0]; i0++) {
        for (unsigned i1 = 0; i1 < n[1]; i1++) {
          AList<TLink> forLine;
          forLine.append((* l[0])[i0]);
          forLine.append((* l[1])[i1]);

          TLine& line = * new TLine(forLine);
          int err = line.fit();
          if (err) {
            delete & line;
            continue;
          }

          lines.append(line);
        }
      }
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "        ... searchLines2 : # of lines found = "
              << lines.length() << std::endl;
#endif

    return lines;
  }

  AList<TLine>
  TBuilderConformal::searchLines1(void) const
  {
    AList<TLine> lines;

    TLine& line = * new TLine(_forLine);
    int err = line.fit();
    if (err) {
      delete & line;
    } else {
      lines.append(line);
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "        ... searchLines1 : # of lines found = "
              << lines.length() << std::endl;
#endif

    return lines;
  }

  TLine
  TBuilderConformal::searchLine(const TLine& initialLine) const
  {
#ifdef TRASAN_WINDOW
    std::string old = sz2.text();
#endif

    static float _szLinkMaxDistance = 5;

    unsigned nLinks = _forLine.length();
    TLine line0 = initialLine;
    line0.remove(initialLine.links());
#ifdef TRASAN_DEBUG
    line0.fitted(true);
#endif
    unsigned nGoodLinks0 = 0;
    AList<TLink> targets;
    unsigned nLoops = 0;
    while (1) {
      ++nLoops;

      unsigned nGoodLinks = 0;
      for (unsigned i = 0; i < nLinks; i++) {
        float distance = line0.distance(* _forLine[i]);
        if (distance < szLinkDistance())
          targets.append(_forLine[i]);
        if (distance < _szLinkMaxDistance) {
          ++nGoodLinks;
        }
      }

#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "    ... searchLine : # of close hits(last) = " << nGoodLinks0
                << ", # of close hits = " << nGoodLinks << std::endl;
#endif

      //...Check condition...
      if (nGoodLinks0 == nGoodLinks) break;

      //...Do fit...
      TLine newLine(targets);
      static TRobustLineFitter fitter("robust fitter");
      int err = fitter.fit(newLine);
      if (err) {
#ifdef TRASAN_WINDOW
        std::cout << "    ... searchLine : failed to fit" << std::endl;
        std::string s = "line search failed";
        sz2.text(s);
        sz2.wait();
#endif
        break;
      }

      //...To protect infinite loop...
      if (nLoops > 10) {
#ifdef TRASAN_DEBUG_DETAIL
        std::cout << "        reached to max # of loops(10) : break";
#endif
        break;
      }

#ifdef TRASAN_WINDOW
      std::string ss = ", # of close hits(last) = " + itostring(int(nGoodLinks0))
                       + ", # of close hits = " + itostring(int(nGoodLinks));
      sz2.append(line0, leda_brown);
      sz2.append(newLine, leda_green);
      sz2.text(old + ", nLoops = " + itostring(int(nLoops)) + ss);
      sz2.wait();
      sz2.remove(line0);
      sz2.remove(newLine);
#endif

      line0 = newLine;
      nGoodLinks0 = nGoodLinks;
      targets.removeAll();
    }

    return line0;
  }

  unsigned
  TBuilderConformal::stereoQuality(const AList<TLink> & links) const
  {
    unsigned n[5] = {0, 0, 0, 0, 0};

    //...Check superlayers...
    unsigned nLinks = links.length();
    for (unsigned i = 0; i < nLinks; i++) {
      const TLink& l = * links[i];
      if (l.wire()->axial()) continue;
      ++n[l.wire()->superLayerId() / 2];
    }

    unsigned nTotal = 0;
    unsigned nMissing = 0;
    for (unsigned i = 0; i < 5; i++) {
      if (_links[i].length() > 0) {
        if ((_nHits[i] > 1) && (_nHits[i + 1] > 1))
          if (n[i] < 2)
            ++nMissing;
      }
      if (n[i] > 1) ++nTotal;
    }

    unsigned toBeReturn = 0;
    if (nMissing <= 1) toBeReturn = 2;
    else if (nMissing == 2) toBeReturn  = 1;
    if (nTotal < 2) toBeReturn = 0;

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "    ... stereoQuality : axial ";
    for (unsigned i = 0; i < 6; i++)
      std::cout << _nHits[i] << ",";
    std::cout << std::endl
              << "                      : stereo ";
    for (unsigned i = 0; i < 5; i++)
      std::cout << n[i] << ",";
    std::cout << " : total " << nTotal << " super layers, "
              << " : quality=" << toBeReturn << std::endl;
#endif

    return toBeReturn;
  }

  void
  TBuilderConformal::salvage(TTrack& t, AList<TSegment> & list) const
  {

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "... salvaging by segments : # of segments = ";
    std::cout << list.length() << std::endl;
    for (unsigned i = 0; i < (unsigned) list.length(); i++)
      list[i]->dump("hits sort flag", "    ");
#endif

    //...Pick up links...
    AList<TLink> links;
    unsigned n = list.length();
    for (unsigned i = 0; i < n; i++)
      links.append(list[i]->links());

    TBuilder::salvage(t, links);
  }

  TLine*
  TBuilderConformal::initialLineOld(const TTrack& t,
                                    AList<TSegment> & list) const
  {

    //...Select good segments in acceptance...
    AList<TSegment> bad;
    AList<TLink> lList;
    unsigned n = list.length();
    for (unsigned i = 0; i < n; i++) {
      TLink& l = * new TLink();
      int err = t.szPosition(* list[i], l);
      if (err) {
        bad.append(list[i]);
        delete & l;
        continue;
      }

      const HepGeom::Point3D<double> & p = l.position();
      if (p.y() > l.wire()->forwardPosition().z() ||
          p.y() < l.wire()->backwardPosition().z()) {
        bad.append(list[i]);
        delete & l;
        continue;
      }
      lList.append(l);
    }
    list.remove(bad);
    n = list.length();

    //...Cal. expected # of super layers...
    unsigned nAMaxSL = 0;
    unsigned nAMinSL = 10;
    const AList<TSegment> & segments = t.segments();
    unsigned nA = segments.length();
    for (unsigned i = 0; i < nA; i++) {
      unsigned sl = segments[i]->superLayerId();
      if (sl > nAMaxSL) nAMaxSL = sl;
      if (sl < nAMinSL) nAMinSL = sl;
    }
    unsigned nExpected = (nAMaxSL - nAMinSL) / 2;
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "    ... initialLine : axial super layer usage = " << nAMinSL;
    std::cout << " ~ " << nAMaxSL << std::endl;
    std::cout << "                    : expected stereo super layers = ";
    std::cout << nExpected << std::endl;
#endif

//    TLine * line = NULL;
    AList<TSegment> list0 = list;
    AList<TLink> lList0 = lList;
    while (1) {
      TLine* line = initialLine1(t, list, lList);
      if (line) {
        AList<TSegment> tmp = selectStereoSegment(* line, list0, lList0);
#ifdef TRASAN_DEBUG_DETAIL
        std::cout << "    ... initialLine1 : # of selected segments = ";
        std::cout << tmp.length() << std::endl;
#endif
        if ((unsigned) tmp.length() >= nExpected) {
          lList0.remove(line->links());
          HepAListDeleteAll(lList0);
          list = tmp;
          return line;
        }
        delete line;
      }

      line = TBuilder::initialLine(t, lList);
//  line = initialLine(t, lList);
      if (line) {
        AList<TSegment> tmp = selectStereoSegment(* line, list0, lList0);
        bool ok = ((unsigned) tmp.length() >= nExpected);
        bool last = (TLink::nSuperLayers(lList) <= nExpected);
#ifdef TRASAN_DEBUG_DETAIL
        std::cout << "    ... initialLine2 : # of selected segments = ";
        std::cout << tmp.length() << " : ok, last = ";
        std::cout << ok << ", " << last << std::endl;
#endif
        if (ok || last) {
          lList0.remove(line->links());
          HepAListDeleteAll(lList0);
          list = tmp;
          return line;
        }
        removeFarSegment(* line, list, lList);
        delete line;
      } else {
        HepAListDeleteAll(lList0);
        return NULL;
      }
    }
  }

  TLine*
  TBuilderConformal::initialLine1(const TTrack&,
                                  const AList<TSegment> & list,
                                  const AList<TLink> & lList) const
  {

    //...Check input...
    if (list.length() < 2) return NULL;

    //...Check super layer pattern...
    AList<TSegment> sl[5];
    AList<TLink> tl[5];
    unsigned n = list.length();
    for (unsigned i = 0; i < n; i ++) {
      unsigned j = list[i]->superLayerId() / 2;
      sl[j].append(list[i]);
      tl[j].append(lList[i]);
    }
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "        ... initialLine1 : super layer ptn = ";
    for (unsigned i = 0; i < 5; i++) std::cout << sl[i].length();
    std::cout << std::endl;
#endif

    //...Count single segment layer...
    unsigned nSingle = 0;
    for (unsigned i = 0; i < 5; i++)
      if (sl[i].length() == 1) ++nSingle;
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "        ... # of single segment layer = " << nSingle << std::endl;
#endif
    if (nSingle < 2) return NULL;

    //...Happy case...
    AList<TSegment> bestCombination;
    AList<TLink> forLine;
    for (unsigned i = 0; i < 5; i++) {
      if (sl[i].length() != 1) continue;
      bestCombination.append(sl[i]);
      forLine.append(tl[i]);
    }
    TLine& line = * new TLine(forLine);
    int err = line.fit();
    if (err) {
      delete & line;
      return NULL;
    }
    return & line;
  }

  TLine*
  TBuilderConformal::initialLine(const TTrack& t,
                                 AList<TSegment> & list) const
  {

    //...Select good segments in acceptance...
    AList<TSegment> bad;
    AList<TLink> lList;
    unsigned n = list.length();
    for (unsigned i = 0; i < n; i++) {
      TLink& l = * new TLink();
      int err = t.szPosition(* list[i], l);
      if (err) {
        bad.append(list[i]);
        delete & l;
        continue;
      }

      const HepGeom::Point3D<double> & p = l.position();
      if (p.y() > l.wire()->forwardPosition().z() ||
          p.y() < l.wire()->backwardPosition().z()) {
        bad.append(list[i]);
        delete & l;
        continue;
      }
      lList.append(l);
    }
    list.remove(bad);
    n = list.length();

    //...Cal. expected # of super layers...
    unsigned nAMaxSL = 0;
    unsigned nAMinSL = 10;
    const AList<TSegment> & segments = t.segments();
    unsigned nA = segments.length();
    for (unsigned i = 0; i < nA; i++) {
      unsigned sl = segments[i]->superLayerId();
      if (sl > nAMaxSL) nAMaxSL = sl;
      if (sl < nAMinSL) nAMinSL = sl;
    }
    unsigned nExpected = (nAMaxSL - nAMinSL) / 2;
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "    ... initialLine : axial super layer usage = " << nAMinSL;
    std::cout << " ~ " << nAMaxSL << std::endl;
    std::cout << "                    : expected stereo super layers = ";
    std::cout << nExpected << std::endl;
#endif

//    TLine * line = NULL;
    AList<TSegment> list0 = list;
    AList<TLink> lList0 = lList;
    while (1) {
      bool last = (TLink::nSuperLayers(lList) <= nExpected);

      TLine* line = TBuilder::initialLine(t, lList);
//  TLine * line = initialLine(t, lList);
      if (line) {
        AList<TSegment> tmp = selectStereoSegment(* line, list0, lList0);
#ifdef TRASAN_DEBUG_DETAIL
        std::cout << "    ... initialLine2 : # of selected segments = ";
        std::cout << tmp.length() << std::endl;
#endif
        if (((unsigned) tmp.length() >= nExpected) ||
            ((unsigned) tmp.length() >= 4)         ||
            last) {
          list = tmp;
          return line;
        }
        delete line;
      }

      line = initialLine1(t, list, lList);
      if (line) {
        AList<TSegment> tmp = selectStereoSegment(* line, list0, lList0);
        bool ok = ((unsigned) tmp.length() >= nExpected);
#ifdef TRASAN_DEBUG_DETAIL
        std::cout << "    ... initialLine1 : # of selected segments = ";
        std::cout << tmp.length() << " : ok, last = ";
        std::cout << ok << ", " << last << std::endl;
#endif
        if (ok) {
          list = tmp;
          return line;
        }
        removeFarSegment(* line, list, lList);
        delete line;
      } else {
        return NULL;
      }
    }
  }


  AList<TSegment>
  TBuilderConformal::selectStereoSegment(const TLine& line,
                                         const AList<TSegment> & list,
                                         const AList<TLink> & szList) const
  {
    AList<TSegment> outList;
    unsigned n = list.length();
    for (unsigned i = 0; i < n; i++) {
      float distance = line.distance(* szList[i]);
      if (distance < _szSegmentDistance) outList.append(list[i]);
    }
    return outList;
  }

  void
  TBuilderConformal::removeFarSegment(const TLine& line,
                                      AList<TSegment> & list,
                                      AList<TLink> & szList) const
  {
    unsigned n = list.length();
    float maxDistance = 0.;
    unsigned maxId = 0;
    for (unsigned i = 0; i < n; i++) {
      float distance = line.distance(* szList[i]);
      if (distance > maxDistance) maxId = i;
    }
    list.remove(maxId);
    szList.remove(maxId);
  }

  TTrack*
  TBuilderConformal::buildStereo(TTrack& track,
                                 TLine& line,
                                 const AList<TLink> & list) const
  {

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "... building stereo by links : # of links = ";
    std::cout << list.length() << std::endl;
#endif

    //...Classify TLink's...
    AList<TLink> cores;
    AList<TLink> nonCores;
    TLink::separateCores(list, cores, nonCores);

    //...Check # of links...
    unsigned nCores = cores.length();
    if (nCores < minNCores()) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "... stereo building failure : # of cores(=" << nCores;
      std::cout << ") is less then " << minNCores() << std::endl;
#endif
      return NULL;
    }

#ifdef TRASAN_WINDOW
    sz2.appendSz(track, cores, leda_brown);
#endif

    //...Cal. left and right position...
    AList<TLink> forNewLine;
    for (unsigned i = 0; i < nCores; i++) {
      TLink& t = * cores[i];
      for (unsigned i = 0; i < 2; i++) {
        TLink& tt = * new TLink(t);
        tt.leftRight(i);
        int err = track.szPosition(tt);
        if (err) {
          delete & tt;
        } else {
          if (line.distance(tt) < szLinkDistance()) {
            tt.link(& t);
            forNewLine.append(tt);
          } else {
            delete & tt;
          }
        }
      }
    }

    //...Create new line...
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "    ... creating a new line" << std::endl;
#endif
    unsigned nNewLine = forNewLine.length();
    TLine newLine(forNewLine);
    static TRobustLineFitter fitter("abc");
    int err = fitter.fit(newLine);
    // int err = newLine.fit();
    if (err < 0) {
      HepAListDeleteAll(forNewLine);
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "    ... 2nd linear fit failure. nLinks(";
      std::cout << forNewLine.length() << ")" << std::endl;
#endif
      return NULL;
    }

#ifdef TRASAN_WINDOW
    sz2.append(newLine, leda_green);
    sz2.wait();
#endif

#ifdef TRASAN_DEBUG_DETAIL
    TLink::dump(forNewLine, "sort hits stereo", "        ");
#endif

    //...Decide left/right...
    AList<TLink> toRemove;
    TLink* last = NULL;
    for (unsigned i = 0; i < nNewLine; i++) {
      if (last == NULL) last = forNewLine[i]->link();
      else {
        if (last == forNewLine[i]->link()) {
          if (newLine.distance(* forNewLine[i - 1]) >
              newLine.distance(* forNewLine[i]))
            toRemove.append(forNewLine[i - 1]);
          else
            toRemove.append(forNewLine[i]);
          last = NULL;
        } else {
          last = forNewLine[i]->link();
        }
      }
    }
    forNewLine.remove(toRemove);
    nNewLine = forNewLine.length();

#ifdef TRASAN_DEBUG_DETAIL
    TLink::dump(toRemove, "sort hits stereo", "      x ");
    TLink::dump(forNewLine, "sort hits stereo", "        ");
#endif

    //...3D fit...
    for (unsigned i = 0; i < nNewLine; i++)
      track.append(* forNewLine[i]->link());
    CLHEP::HepVector a(5);
    a = track.helix().a();
    a[3] = newLine.b();
    a[4] = track.charge() * newLine.a();
    track._helix->a(a);

    //...Refine...
    AList<TLink> bad;
    err = helixFitter().fit(track);
    track.refine(bad, maxSigmaStereo() * 100.);
    err = helixFitter().fit(track);
    track.refine(bad, maxSigmaStereo() * 10.);
    err = helixFitter().fit(track);
    track.refine(bad, maxSigmaStereo());
    err = helixFitter().fit(track);

#ifdef TRASAN_WINDOW
    sz2.text("stereo finished");
    sz2.oneShot(track, leda_blue);
#endif

    //...Termination...
    HepAListDeleteAll(toRemove);
    HepAListDeleteAll(forNewLine);
    return & track;
  }

  TTrack*
  TBuilderConformal::buildRphi(const AList<TSegment> & list) const
  {
#ifdef TRASAN_DEBUG_DETAIL
    const std::string stage = "BldConf::buildRphi";
    EnterStage(stage);
    std::cout << Tab() << "building rphi by segments : # of segments = "
              << list.length() << std::endl;
    for (unsigned i = 0; i < (unsigned) list.length(); i++)
      list[i]->dump("hits sort flag breif", Tab() + itostring(i) + " ");
#endif

    //...Pick up links...
    AList<TLink> links = Links(list);

    //...Main funtion...
    TTrack* t = buildRphi(links);

    //...Check used segments...
    if (t) {
      unsigned n = list.length();
      for (unsigned i = 0; i < n; i++) {
        TSegment& segment = * list[i];
        AList<TLink> used = Links(segment, * t);
        if (used.length()) {
          t->segments().append(segment);
          segment.tracks().append(t);
        }
      }
    }

#ifdef TRASAN_DEBUG_DETAIL
    LeaveStage(stage);
#endif

    return t;
  }

  TTrack*
  TBuilderConformal::buildRphi(const AList<TLink> & list) const
  {
#ifdef TRASAN_DEBUG_DETAIL
    const std::string stage = "bld2";
    EnterStage(stage);
    std::cout << Tab() << "building rphi by links:#links="
              << list.length() << std::endl;
#endif

    //...Classify TLink's...
    AList<TLink> cores;
    AList<TLink> nonCores;
    TLink::separateCores(list, cores, nonCores);

    //...Check # of links...
    unsigned nCores = cores.length();
    if (nCores < _minNCores) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "building rphi failure:#cores(=" << nCores
                << ") is less then " << _minNCores << std::endl;
      LeaveStage(stage);
#endif
      return NULL;
    }

    //...Make a circle...
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "making a circle:#cores=" << cores.length()
              << std::endl;
#endif
    TCircle c(cores);
    int err = c.fit();
    if (err < 0) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "building rphi failure:circle fit error="
                << err << std::endl;
      LeaveStage(stage);
#endif
      return NULL;
    }

    //...Make a track...
    TTrack* t(NULL);
#if defined(BELLE_DEBUG)
    try {
#endif
      t = new TTrack(c);
      AList<TLink> bad;
      err = _fitter.fit(* t);
      if (err < 0) goto discard;
      t->refine(bad, _maxSigma * 100.);
      err = _fitter.fit(* t);
      t->refine(bad, _maxSigma * 10.);
      err = _fitter.fit(* t);
      t->refine(bad, _maxSigma);
      err = _fitter.fit(* t);
      if (err < 0) goto discard;
#if defined(BELLE_DEBUG)
    } catch (std::string& e) {
      goto discard;
    }
#endif

#ifdef TRASAN_DEBUG_DETAIL
    c.dump("detail", Tab() + "ccl> ");
    t->dump("detail", Tab() + "1st> ");
#endif

    //...Try to append non-core hits...
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "appending non-core hits:#=" << nonCores.length()
              << std::endl;
#endif

    t->appendByApproach(nonCores, _salvageLevel);

#ifdef TRASAN_DEBUG_DETAIL
    t->dump("hits sort flag", Tab());
    LeaveStage(stage);
#endif

    return t;

    //...Something happened...
discard:
    if (t) delete t;

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "building rphi failure:helix fit error="
              << err << std::endl;
    LeaveStage(stage);
#endif

    return NULL;
  }

} // namespace Belle

