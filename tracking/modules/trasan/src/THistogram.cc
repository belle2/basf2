//-----------------------------------------------------------------------------
// $Id: THistogram.cc 10129 2007-05-18 12:44:41Z katayama $
//-----------------------------------------------------------------------------
// Filename : THistogram.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class for a histogram used in tracking.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.24  2005/01/14 00:55:40  katayama
// uninitialized variable
//
// Revision 1.23  2003/12/25 12:03:33  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.22  2003/12/19 07:36:02  yiwasaki
// Trasan 3.06 : small cell cdc available in the conformal finder; Tagir modification is applied to the curl finder; the hough finder is added;
//
// Revision 1.21  2001/12/23 09:58:47  katayama
// removed Strings.h
//
// Revision 1.20  2001/12/19 02:59:46  katayama
// Uss find,istring
//
// Revision 1.19  2000/05/12 04:18:28  katayama
// Added stdio.h
//
// Revision 1.18  2000/04/11 13:05:44  katayama
// Added std:: to cout, cerr, endl etc.
//
// Revision 1.17  2000/01/30 08:17:06  yiwasaki
// Trasan 1.67i = Trasan 2.00 RC1 : new conf modified
//
// Revision 1.16  2000/01/28 06:30:23  yiwasaki
// Trasan 1.67h : new conf modified
//
// Revision 1.15  1999/10/30 10:12:19  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.14  1998/07/02 09:04:39  yiwasaki
// Trasan 1.0 official release
//
// Revision 1.13  1998/06/24 06:55:01  yiwasaki
// Trasan 1 beta 5.2 release, memory leaks fixed
//
// Revision 1.12  1998/06/21 18:38:20  yiwasaki
// Trasan 1 beta 5 release, rphi improved?
//
// Revision 1.11  1998/06/17 20:22:59  yiwasaki
// Trasan 1 beta 4 release again, KS effi. improved?
//
// Revision 1.10  1998/06/17 20:12:37  yiwasaki
// Trasan 1 beta 4 release, KS effi. improved?
//
// Revision 1.9  1998/06/11 08:14:08  yiwasaki
// Trasan 1 beta 1 release
//
// Revision 1.8  1998/05/26 05:10:17  yiwasaki
// cvs repair
//
// Revision 1.7  1998/05/24 14:59:57  yiwasaki
// Trasan 1 alpha 5 release, pivot is moved to the first hit
//
// Revision 1.6  1998/05/22 08:23:19  yiwasaki
// Trasan 1 alpha 4 release, TSegment added, TConformalLink no longer used
//
// Revision 1.5  1998/05/18 08:08:54  yiwasaki
// preparation for alpha 3
//
// Revision 1.4  1998/05/08 09:45:44  yiwasaki
// Trasan 1 alpha 2 relase, stereo recon. added, off-vtx recon. added
//
// Revision 1.3  1998/04/23 17:21:41  yiwasaki
// Trasan 1 alpha 1 release
//
// Revision 1.2  1998/04/16 16:49:33  yiwasaki
// minor changes
//
// Revision 1.1  1998/04/10 00:50:16  yiwasaki
// TCircle, TConformalFinder, TConformalLink, TFinderBase, THistogram, TLink, TTrackBase classes added
//
// Revision 1.1  1998/02/09 04:12:20  yiwasaki
// Trasan 0.1 beta
//
//
//-----------------------------------------------------------------------------
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iomanip>




#include "tracking/modules/trasan/THistogram.h"
#include "tracking/modules/trasan/TCircle.h"
#ifdef TRASAN_DEBUG
#include "tracking/modules/trasan/TDebugUtilities.h"
#endif

namespace Belle {

  THistogram::THistogram(unsigned nBins) : _nBins(nBins)
  {
    _binSize = 2. * M_PI / (float) _nBins;
    if (NULL == (_bins = (unsigned*) malloc(_nBins * sizeof(unsigned)))) {
      perror("$Id: THistogram.cc 10129 2007-05-18 12:44:41Z katayama $:_bins:malloc");
      exit(1);
    }
    if (NULL == (_masks = (bool*) malloc(_nBins * sizeof(bool)))) {
      perror("$Id: THistogram.cc 10129 2007-05-18 12:44:41Z katayama $:_masks:malloc");
      exit(1);
    }
    if (NULL == (_links = (AList<TLink> **) malloc(_nBins * sizeof(AList<TLink> *)))) {
      perror("$Id: THistogram.cc 10129 2007-05-18 12:44:41Z katayama $:_links:malloc");
      exit(1);
    }
    for (unsigned i = 0; i < _nBins; i++) {
      _bins[i] = 0;
      _masks[i] = false;
      _links[i] = new AList<TLink>;
    }
  }

  THistogram::~THistogram()
  {
    free(_bins);
    free(_masks);
    for (unsigned i = 0; i < _nBins; i++)
      delete _links[i];
    free(_links);
  }

  void
  THistogram::dump(const std::string& msg, const std::string& pre) const
  {
    std::cout << pre << "THistogram dump:#bins=" << _nBins << std::endl;
    unsigned nLoops = _nBins / 15 + 1;
    unsigned n0 = 0;
    unsigned n1 = 0;
    for (unsigned i = 0; i < nLoops; i++) {
      for (unsigned j = 0; j < 15; j++) {
        if (n0 == _nBins) break;
        std::cout << n0 << std::endl;
        ++n0;
      }
      std::cout << std::endl;
      for (unsigned j = 0; j < 15; j++) {
        if (n1 == _nBins) break;
        if (! _masks[n1])std::cout << std::setw(4) << _bins[n1] << std::endl;
        else             std::cout << "-" << std::setw(3) << _bins[n1] << std::endl;
        ++n1;
      }
      std::cout << std::endl;
    }

    if (msg.find("detail") != std::string::npos) {
      for (unsigned i = 0; i < _nBins; i++) {
        std::cout << "bin " << i << " : ";
        for (unsigned j = 0; j < (unsigned) _links[i]->length(); j++) {
          std::cout << (* _links[i])[j]->wire()->name() << ",";
        }
        std::cout << std::endl;
      }
    }

    return;
  }

  void
  THistogram::fillX(const AList<TLink> & links)
  {
    _all = (AList<TLink> &) links;
    unsigned nLinks = links.length();
    double offset = _binSize / 4.;
    for (unsigned i = 0; i < nLinks; i++) {
      TLink* l = links[i];
      const HepGeom::Point3D<double> & p = l->position();
      unsigned pos = (unsigned) floor((p.x() + offset) / _binSize);

      //...Why is this needed?...
      pos %= _nBins;

      ++_bins[pos];
      _links[pos]->append(l);
    }
  }

  void
  THistogram::fillY(const AList<TLink> & links)
  {
    _all = (AList<TLink> &) links;
    unsigned nLinks = links.length();
    for (unsigned i = 0; i < nLinks; i++) {
      TLink* l = links[i];
      const HepGeom::Point3D<double> & p = l->position();
      unsigned pos = (unsigned) floor(p.y() / _binSize);

      //...Why is this needed?...
      pos %= _nBins;

      ++_bins[pos];
      _links[pos]->append(l);
    }
  }

  void
  THistogram::fillPhi(const AList<TLink> & links)
  {
    _all = (AList<TLink> &) links;
    unsigned nLinks = links.length();
    double offset = _binSize / 4.;
    for (unsigned i = 0; i < nLinks; i++) {
      TLink* l = links[i];
      const HepGeom::Point3D<double> & p = l->position();
      float phi = atan2(p.y(), p.x()) + M_PI;
      unsigned pos = (unsigned) floor((phi + offset) / _binSize);

      //...Why is this needed?...
      pos %= _nBins;

      ++_bins[pos];
      _links[pos]->append(l);
    }
  }

  void
  THistogram::remove(const AList<TLink> & links)
  {
    for (unsigned i = 0; i < _nBins; i++) {
      _links[i]->remove(links);
      _bins[i] = _links[i]->length();
    }
    _all.remove(links);
  }

  AList<TLink>
  THistogram::contents(unsigned center, unsigned width) const
  {
    AList<TLink> links;
    for (int i = - (int) width;
         i <= (int) width;
         i++) {
      links.append(* bin((int) center + i));
    }
    return links;
  }

  AList<TLink>
  THistogram::contents(int start, int end) const
  {
    AList<TLink> links;
    for (int i = start; i <= end; i++)
      links.append(* bin(i));
    return links;
  }

  AList<TSegment0>
  THistogram::clusters0(void) const
  {
    AList<TSegment0> list;

    //...Serach for empty bin...
    unsigned begin = 0;
    while (_bins[begin] > 0) begin++;
    if (begin == _nBins) return list;

    //...Start searching...
    unsigned loop = 0;
    while (loop < _nBins) {
      ++loop;
      unsigned id = (begin + loop) % _nBins;
      if (_bins[id]) {
        unsigned size = 0;
        TSegment0* c = new TSegment0();
        while (_bins[id]) {
          if (_bins[id]) ++size;
          c->append(* _links[id]);
          ++loop;
          id = (begin + loop) % _nBins;
          if (loop == _nBins) break;
        }
        list.append(c);
      }
    }
    return list;
  }

  AList<TSegment0>
  THistogram::segments0(void) const
  {

    //...Obtain raw clusters...
    AList<TSegment0> list = clusters0();
    unsigned n = list.length();
    if (n == 0) return list;

    //...Examine each cluster...
    AList<TSegment0> splitted;
    for (unsigned i = 0; i < n; i++) {
      TSegment0* c = list[i];

      AList<TSegment0> newClusters = c->split();
      if (newClusters.length() == 0) {
        c->solveDualHits();
        continue;
      }

      list.append(newClusters);
      splitted.append(c);
#ifdef TRASAN_DEBUG_DETAIL
      c->dump("hits", "    ");
      std::cout << "    ... splitted as" << std::endl;
      for (unsigned j = 0; j < (unsigned) newClusters.length(); j++) {
        std::cout << "    " << j << " : ";
        newClusters[j]->dump("hits");
      }
#endif
    }
    list.remove(splitted);
    HepAListDeleteAll(splitted);

    return list;

  }

  AList<TSegment>
  THistogram::clusters(void) const
  {
    AList<TSegment> list;

    //...Serach for empty bin...
    unsigned begin = 0;
    while (_bins[begin] > 0) begin++;
    if (begin == _nBins) return list;

    //...Start searching...
    unsigned loop = 0;
    while (loop < _nBins) {
      ++loop;
      unsigned id = (begin + loop) % _nBins;
      if (_bins[id]) {
        unsigned size = 0;
        TSegment* c = new TSegment();
        while (_bins[id]) {
          if (_bins[id]) ++size;
          c->append(* _links[id]);
          ++loop;
          id = (begin + loop) % _nBins;
          if (loop == _nBins) break;
        }
        list.append(c);
      }
    }
    return list;
  }

  AList<TSegment>
  THistogram::segments(void) const
  {

#ifdef TRASAN_DEBUG_DETAIL
    const std::string stage = "THistogram::segments";
    EnterStage(stage);
#endif

    //...Obtain raw clusters...
    AList<TSegment> list = clusters();
    unsigned n = list.length();
    if (n == 0) return list;

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "THistogram::segments ... #segments=" << n
              << std::endl;
#endif

    //...Examine each cluster...
    AList<TSegment> splitted;
    for (unsigned i = 0; i < n; i++) {
      TSegment* c = list[i];

#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "Base segment:";
      c->dump("breif");
//  c->dump("detail", "    ");
#endif

      AList<TSegment> newClusters = c->split();
      if (newClusters.length() == 0) {
#ifdef TRASAN_DEBUG_DETAIL
        std::cout << Tab() << "    ... Solving dual hits" << std::endl;
#endif
        c->solveDualHits();
        continue;
      }

      list.append(newClusters);
      splitted.append(c);
#ifdef TRASAN_DEBUG_DETAIL
      c->dump("breif", "    ");
      std::cout << Tab() << "    ... splitted as" << std::endl;
      for (unsigned j = 0; j < (unsigned) newClusters.length(); j++) {
        std::cout << "    " << j << " : ";
        newClusters[j]->dump("breif", Tab());
      }
#endif
    }
    list.remove(splitted);
    HepAListDeleteAll(splitted);

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "    ... # of found segments=" << list.length()
              << std::endl;
    LeaveStage(stage);
#endif

    return list;
  }

} // namespace Belle

