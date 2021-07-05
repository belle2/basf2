/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A class to find tracks usning Hough algorithm
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include <cstdlib>
#include "trg/trg/Debug.h"
#include "trg/cdc/Circle.h"
#include "trg/cdc/PeakFinder.h"
#include "trg/cdc/HoughPlaneMulti2.h"
#include "trg/cdc/HoughTransformationCircle.h"

////////////////////////////////////////////////save data

#include <iostream>
#include <math.h>
////////////////////////////////////////////////save data

using namespace std;

namespace Belle2 {

  string
  TRGCDCPeakFinder::version(void) const
  {
    return string("TRGCDCPeakFinder 5.05");
  }

  TRGCDCPeakFinder::TRGCDCPeakFinder(const string& name)
    : _name(name)
  {
  }

  TRGCDCPeakFinder::~TRGCDCPeakFinder()
  {
  }

  void
  TRGCDCPeakFinder::regions(TCHPlane& hp, const unsigned threshold) const
  {

    TRGDebug::enterStage("Making regions");

    //...Search cells above threshold...
    unsigned nCells = hp.nX() * hp.nY();
    static unsigned* candidates =
      (unsigned*) malloc(nCells * sizeof(unsigned));
    unsigned nActive = 0;
    for (unsigned j = 0; j < hp.nY(); j++) {
      //minus x direction ,  plus  -x direction
      if ((hp.name()) == "circle hough minus")
        for (unsigned i = 0; i < hp.nX(); i++) {
          //...Threshold check...
          const unsigned n = hp.entry(i, j);
          if (n < threshold) continue;
          candidates[nActive] = hp.serialId(i, j);
          ++nActive;
        }
      else
        for (unsigned z = hp.nX(); z > 0 ; --z) {
          //...Threshold check...
          unsigned i = 0;
          i = z - 1;
          const unsigned n = hp.entry(i, j);
          if (n < threshold) continue;
          candidates[nActive] = hp.serialId(i, j);
          ++nActive;
        }
    }

    if (TRGDebug::level())
      cout << TRGDebug::tab() << "Active cells=" << nActive << endl;

    //...Loop over active cells...
    const unsigned used = nCells;
    for (unsigned i = 0; i < nActive; i++) {
      if (candidates[i] == used) continue;
      const unsigned id0 = candidates[i];
      candidates[i] = used;

      //...Make a new region...
      vector<unsigned>* region = new vector<unsigned>;
      region->push_back(id0);
      if (TRGDebug::level() > 2)
        cout << TRGDebug::tab(4) << "new region made" << endl;

      //...Search neighbors...
      for (unsigned j = 0; j < nActive; j++) {
        if (candidates[j] == used) continue;
        const unsigned id1 = candidates[j];

        unsigned x1 = 0;
        unsigned y1 = 0;
        hp.id(id1, x1, y1);
        if (TRGDebug::level() > 2)
          cout << TRGDebug::tab(8) << "cell:x=" << x1 << ",y=" << y1
               << endl;

        for (unsigned k = 0; k < unsigned(region->size()); k++) {
          unsigned id2 = (* region)[k];
          unsigned x2 = 0;
          unsigned y2 = 0;
          hp.id(id2, x2, y2);
          int difx = abs(int(x1) - int(x2));
          int dify = abs(int(y1) - int(y2));
          if (difx > (int) hp.nX() / 2) difx = hp.nX() - difx;
          if (dify > (int) hp.nY() / 2) dify = hp.nY() - dify;

          if (TRGDebug::level() > 2) {
            cout << TRGDebug::tab(12) << "x=" << x2 << ",y=" << y2
                 << ":difx=" << difx << ",dify=" << dify;
            if ((difx < 2) && (dify < 3))
              cout << " ... connected" << endl;
            else
              cout << endl;
          }

          if ((difx < 2) && (dify < 3)) {
            region->push_back(id1);
            candidates[j] = used;
            break;
          }

        }
      }
      hp.setRegion(region);
    }

    if (TRGDebug::level())
      cout << TRGDebug::tab() << "Regions=" << hp.regions().size() << endl;

    TRGDebug::leaveStage("Making regions");
  }

  ////////////////////////////////////////////////

  ///...rlrel...
  // right/left neighboring 2x2 squares
  // a3 a4 | b3 b4
  // a1 a2 | b1 b2
  // unconnected if
  // x x | . x   or    x . | x x
  // x x | . x         x . | x x
  bool rlrel(vector<unsigned> a, vector<unsigned> b)
  {
    bool t = true;
    //case 1
    if (b[1] == 0 && b[3] == 0) {
      t = false;
    }
    //case 2
    if (a[2] == 0 && a[4] == 0) {
      t = false;
    }
    return t;
  }
  ///...udrel...
  // up/down neighboring 2x2 squares
  // b3 b4
  // b1 b2
  // -----
  // a3 a4
  // a1 a2
  // unconnected if
  // x x      x x     . .
  // x x      . .     . .
  // ---  or  ---  or ---
  // . .      . .     x x
  // . .      x x     x x
  bool udrel(vector<unsigned> a, vector<unsigned> b)
  {
    bool t = true;
    //case 1
    if (a[1] == 0 && a[2] == 0 && a[3] == 0 && a[4] == 0) {
      t = false;
    }
    //case 2
    if (a[3] == 0 && a[4] == 0 && b[1] == 0 && b[2] == 0) {
      t = false;
    }
    //case 3
    if (b[1] == 0 && b[2] == 0 && b[3] == 0 && b[4] == 0) {
      t = false;
    }

    return t;
  }
  ///...mirel...
  // diagonal neighboring 2x2 squares
  //       b3 b4
  //       b1 b2
  // a3 a4
  // a1 a2
  // unconnected if
  //     x x          x x           . x
  //     . x          x x           . x
  // x .      or  x .      or   x x
  // x x          x .           x x
  bool mirel(vector<unsigned> a, vector<unsigned> b)
  {
    bool t = true;
    //case 1
    if (a[4] == 0 && b[1] == 0) {
      t = false;
    }
    //case 2
    if (a[2] == 0 && a[4] == 0) {
      t = false;
    }
    //case 3
    if (b[1] == 0 && b[3] == 0) {
      t = false;
    }
    return t;
  }

  ///...Find center Pattern1 from Pattern 2
  // Pattern 2 is a 3x2 rectangle of shape
  // a4 a5
  // a2 a3
  // a0 a1
  // function returns index of peak depending on pattern
  unsigned short FindCP1(vector<unsigned> a)
  {
    unsigned center = 0;
    //...1...
    //(x on, . off, return O)
    // . .
    // . .
    // O .
    if (a[0] == 1 && a[1] == 0 && a[2] == 0 && a[3] == 0 && a[4] == 0 && a[5] == 0) {center = 0;}
    //...2...
    //(x on, . off, return O)
    // . .   . .   . .
    // . .   x .   . x
    // O x   O .   O .
    if (a[0] == 1 && a[1] == 1 && a[2] == 0 && a[3] == 0 && a[4] == 0 && a[5] == 0) {center = 0;}
    if (a[0] == 1 && a[1] == 0 && a[2] == 1 && a[3] == 0 && a[4] == 0 && a[5] == 0) {center = 0;}
    if (a[0] == 1 && a[1] == 0 && a[2] == 0 && a[3] == 1 && a[4] == 0 && a[5] == 0) {center = 0;}
    //...3...
    //(x on, . off, return O)
    // . .   . .   . .   x .   . x   x .   . x
    // x .   . x   O x   O .   O .   . O   . O
    // O x   x O   x .   x .   x .   x .   x .
    if (a[0] == 1 && a[1] == 1 && a[2] == 1 && a[3] == 0 && a[4] == 0 && a[5] == 0) {center = 0;}
    if (a[0] == 1 && a[1] == 1 && a[2] == 0 && a[3] == 1 && a[4] == 0 && a[5] == 0) {center = 1;}
    if (a[0] == 1 && a[1] == 0 && a[2] == 1 && a[3] == 1 && a[4] == 0 && a[5] == 0) {center = 2;}
    if (a[0] == 1 && a[1] == 0 && a[2] == 1 && a[3] == 0 && a[4] == 1 && a[5] == 0) {center = 2;}
    if (a[0] == 1 && a[1] == 0 && a[2] == 1 && a[3] == 0 && a[4] == 0 && a[5] == 1) {center = 2;}
    if (a[0] == 1 && a[1] == 0 && a[2] == 0 && a[3] == 1 && a[4] == 1 && a[5] == 0) {center = 3;}
    if (a[0] == 1 && a[1] == 0 && a[2] == 0 && a[3] == 1 && a[4] == 0 && a[5] == 1) {center = 3;}
    //...4...
    //(x on, . off, return O)
    // . .   x .   . x   x .   . x   x .   . x   x x   x x
    // x x   O .   O .   . O   . O   O x   O x   O .   . O
    // O x   x x   x x   x x   x x   x .   x .   x .   x .
    if (a[0] == 1 && a[1] == 1 && a[2] == 1 && a[3] == 1 && a[4] == 0 && a[5] == 0) {center = 0;}
    if (a[0] == 1 && a[1] == 1 && a[2] == 1 && a[3] == 0 && a[4] == 1 && a[5] == 0) {center = 2;}
    if (a[0] == 1 && a[1] == 1 && a[2] == 1 && a[3] == 0 && a[4] == 0 && a[5] == 1) {center = 2;}
    if (a[0] == 1 && a[1] == 1 && a[2] == 0 && a[3] == 1 && a[4] == 1 && a[5] == 0) {center = 3;}
    if (a[0] == 1 && a[1] == 1 && a[2] == 0 && a[3] == 1 && a[4] == 0 && a[5] == 1) {center = 3;}
    if (a[0] == 1 && a[1] == 0 && a[2] == 1 && a[3] == 1 && a[4] == 1 && a[5] == 0) {center = 2;}
    if (a[0] == 1 && a[1] == 0 && a[2] == 1 && a[3] == 1 && a[4] == 0 && a[5] == 1) {center = 2;}
    if (a[0] == 1 && a[1] == 0 && a[2] == 1 && a[3] == 0 && a[4] == 1 && a[5] == 1) {center = 2;}
    if (a[0] == 1 && a[1] == 0 && a[2] == 0 && a[3] == 1 && a[4] == 1 && a[5] == 1) {center = 3;}
    //...5...
    //(x on, . off, return O)
    // x .   . x   x x   x x   x x
    // O x   O x   O .   . O   O x
    // x x   x x   x x   x x   x .
    if (a[0] == 1 && a[1] == 1 && a[2] == 1 && a[3] == 1 && a[4] == 1 && a[5] == 0) {center = 2;}
    if (a[0] == 1 && a[1] == 1 && a[2] == 1 && a[3] == 1 && a[4] == 0 && a[5] == 1) {center = 2;}
    if (a[0] == 1 && a[1] == 1 && a[2] == 1 && a[3] == 0 && a[4] == 1 && a[5] == 1) {center = 2;}
    if (a[0] == 1 && a[1] == 1 && a[2] == 0 && a[3] == 1 && a[4] == 1 && a[5] == 1) {center = 3;}
    if (a[0] == 1 && a[1] == 0 && a[2] == 1 && a[3] == 1 && a[4] == 1 && a[5] == 1) {center = 2;}
    //...6...
    //(x on, . off, return O)
    // x x
    // O x
    // x x
    if (a[0] == 1 && a[1] == 1 && a[2] == 1 && a[3] == 1 && a[4] == 1 && a[5] == 1) {center = 2;}
    return center;
  }

  ///...Pattern1 Center...
  // Pattern 1 is a 2x2 square of shape
  // a3 a4
  // a1 a2
  // function returns index of peak depending on pattern
  unsigned FindP1C(vector<unsigned> a)
  {
    unsigned hits = 0;
    unsigned short center = 0;
    for (unsigned short k = 1; k < a.size(); k++) {
      if (a[k] == 1) {hits++;}
    }
    //...1...
    //(x on, . off, return O)
    // . .   . .   O .   . O
    // O .   . O   . .   . .
    if (hits == 1) {
      if (a[1] == 1 && a[2] == 0 && a[3] == 0 && a[4] == 0) {center = 1;}
      if (a[1] == 0 && a[2] == 1 && a[3] == 0 && a[4] == 0) {center = 2;}
      if (a[1] == 0 && a[2] == 0 && a[3] == 1 && a[4] == 0) {center = 3;}
      if (a[1] == 0 && a[2] == 0 && a[3] == 0 && a[4] == 1) {center = 4;}

    }
    //...2...
    //(x on, . off, return O)
    // . .   x .   . x   x .   . x   O x
    // O x   O .   O .   . O   . O   . .
    if (hits == 2) {
      if (a[1] == 1 && a[2] == 1 && a[3] == 0 && a[4] == 0) {center = 1;}
      if (a[1] == 1 && a[2] == 0 && a[3] == 1 && a[4] == 0) {center = 1;}
      if (a[1] == 1 && a[2] == 0 && a[3] == 0 && a[4] == 1) {center = 1;}
      if (a[1] == 0 && a[2] == 1 && a[3] == 1 && a[4] == 0) {center = 2;}
      if (a[1] == 0 && a[2] == 1 && a[3] == 0 && a[4] == 1) {center = 2;}
      if (a[1] == 0 && a[2] == 0 && a[3] == 1 && a[4] == 1) {center = 3;}

    }
    //...3...
    //(x on, . off, return O)
    // x .   . x   O x   x O
    // O x   x O   x .   . x
    if (hits == 3) {
      if (a[1] == 1 && a[2] == 1 && a[3] == 1 && a[4] == 0) {center = 1;}
      if (a[1] == 1 && a[2] == 1 && a[3] == 0 && a[4] == 1) {center = 2;}
      if (a[1] == 1 && a[2] == 0 && a[3] == 1 && a[4] == 1) {center = 3;}
      if (a[1] == 0 && a[2] == 1 && a[3] == 1 && a[4] == 1) {center = 4;}


    }
    //...4...
    //(x on, . off, return O)
    // x x
    // O x
    if (hits == 4) {
      if (a[1] == 1 && a[2] == 1 && a[3] == 1 && a[4] == 1) {center = 1;}

    }

    return center;
  }

  void
  TRGCDCPeakFinder::findPeaksTrasan(TCHPlaneMulti2& hp,
                                    const unsigned threshold,
                                    const bool centerIsPeak,
                                    vector<unsigned>& peakSerialIds) const
  {

    TRGDebug::enterStage("Peak Finding (trasan methode)");
    if (TRGDebug::level())
      cout << TRGDebug::tab() << "threshold=" << threshold
           << ",plane name=[" << hp.name() << "]" << endl;

    //...Make ionnected regions (is this the best way???)...
    regions(hp, threshold);

    //...Determine peaks...
    const vector<vector<unsigned> *>& regions = hp.regions();
    for (unsigned i = 0; i < (unsigned) regions.size(); i++) {

      if (TRGDebug::level() > 1)
        cout << TRGDebug::tab() << "region " << i << " contents" << endl;

      //...Calculate size and center of a region...
      const vector<unsigned>& r = * regions[i];
      unsigned minX = hp.nX();
      unsigned maxX = 0;
      unsigned minY = hp.nY();
      unsigned maxY = 0;
      for (unsigned j = 0; j < (unsigned) r.size(); j++) {
        const unsigned s = r[j];
        unsigned x = 0;
        unsigned y = 0;
        hp.id(s, x, y);
        if (x < minX) minX = x;
        if (x > maxX) maxX = x;
        if (y < minY) minY = y;
        if (y > maxY) maxY = y;

        if (TRGDebug::level() > 1)
          cout << TRGDebug::tab(4) << "x=" << x << ",y=" << y << endl;
      }
      const unsigned cX = minX + (maxX - minX) / 2;
      const unsigned cY = minY + (maxY - minY) / 2;

      //...Determine a center of a region...
      unsigned ncX = hp.nX() * hp.nY();
      unsigned ncY = ncX;
      if (TRGDebug::level() > 1)
        cout << TRGDebug::tab() << "center of region:x=" << cX << ",y="
             << cY << endl;

      if (! centerIsPeak) {

        if (TRGDebug::level() > 1)
          cout << TRGDebug::tab() << "Searching a cell closest to the "
               << "region center" << endl;

        //...Search for a cell which is the closest to the center...
        float minDiff2 = float(hp.nX() * hp.nX() + hp.nY() * hp.nY());
        for (unsigned j = 0; j < (unsigned) r.size(); j++) {
          const unsigned s = r[j];
          unsigned x = 0;
          unsigned y = 0;
          hp.id(s, x, y);

          const float diff2 = (float(x) - float(cX)) *
                              (float(x) - float(cX))
                              + (float(y) - float(cY)) *
                              (float(y) - float(cY));

          if (diff2 < minDiff2) {
            minDiff2 = diff2;
            ncX = x;
            ncY = y;
          }
          if (TRGDebug::level() > 1)
            cout << TRGDebug::tab(4) << "x=" << ncX << ",y=" << ncY
                 << ":diff2=" << diff2 << endl;
        }
      } else {

        //...Search for a peak...
        float max = 0;
        for (unsigned j = 0; j < (unsigned) r.size(); j++) {
          const unsigned s = r[j];
          const float entry = hp.entry(s);
          if (max < entry) {
            max = entry;
            unsigned x = 0;
            unsigned y = 0;
            hp.id(s, x, y);
            ncX = x;
            ncY = y;
          }
        }
      }

      //...Store the center cell...
      const unsigned serialId = hp.serialId(ncX, ncY);
      peakSerialIds.push_back(serialId);

      if (TRGDebug::level()) {
        cout << TRGDebug::tab() << "region " << i << " final center:x="
             << ncX << ",y=" << ncY << endl
             << TRGDebug::tab(4) << "position in HP:x="
             << hp.position(ncX, ncY).x() << ",y="
             << hp.position(ncX, ncY).y() << endl;
      }
    }

    if (TRGDebug::level())
      cout << TRGDebug::tab() << peakSerialIds.size() << " peak(s)"
           << " found in total" << endl;

    TRGDebug::leaveStage("Peak Finding (trasan methode)");
    return;
  }

  void
  TRGCDCPeakFinder::findPeaks(const TCHPlaneMulti2& hp,
                              const unsigned threshold,
                              vector<vector<unsigned>>& peaks) const
  {

    const string sn = "Peak Finding";
    TRGDebug::enterStage(sn);

    if (TRGDebug::level())
      cout << TRGDebug::tab() << "threshold=" << threshold
           << ",plane name=[" << hp.name() << "]" << endl;

    //  p1p2(hp, threshold, peaks);
    p1p2Methode(hp, threshold, peaks);

    TRGDebug::leaveStage(sn);

    return;
  }

  void
  TRGCDCPeakFinder::p1p2Methode(const TCHPlane& hp,
                                const unsigned threshold,
                                vector<vector<unsigned>>& peak_xy) const
  {
    const string sn = "p1p2";
    TRGDebug::enterStage(sn);

    unsigned nCells = hp.nX() * hp.nY();
    unsigned nX2 = hp.nX() / 2;
    unsigned nY2 = hp.nY() / 2;

    //...Search cells above threshold...
    static unsigned* candidates = (unsigned*) malloc(nCells * sizeof(unsigned));
    unsigned nActive = 0;
    for (unsigned j = 0; j < hp.nY(); ++j) {
      //minus x direction, plus -x direction
      if ((hp.name()) == "circle hough minus") {
        for (unsigned i = 0; i < hp.nX(); ++i) {
          //...Threshold check...
          const unsigned n = hp.entry(i, j);
          if (n < threshold) continue;
          candidates[nActive] = hp.serialId(i, j);
          ++nActive;
        }
      } else {
        for (unsigned i = 0; i < hp.nX(); ++i) {
          //...Threshold check...
          const unsigned n = hp.entry(hp.nX() - i - 1, j);
          if (n < threshold) continue;
          candidates[nActive] = hp.serialId(hp.nX() - i - 1, j);
          ++nActive;
        }
      }
    }

    vector<unsigned> p;
    vector<vector<unsigned>> p1m;
    unsigned short no = 0;
    //...create pattern1...begin
    // divide the plane into squares of 2x2
    // outer loop (n x m) goes over the squares,
    // inner loop (j x k) goes over the cells in each square
    for (unsigned n = 0; n < nY2; n++) {
      for (unsigned m = 0; m < nX2; m++) {
        unsigned a = m * 2;
        unsigned b = n * 2;
        bool ot = false;
        ++no; // numbering starts at 1, not 0
        p.push_back(no);

        //...find 4 cells...begin
        for (unsigned j = 0; j < 2; j++) {
          unsigned yy = b + j;
          for (unsigned k = 0; k < 2; k++) {
            unsigned xx = a + k;
            // Plus plane transform (x axis mirrored compared to minus plane)
            if ((hp.name()) == "circle hough plus") {
              xx = hp.nX() - xx - 1;
            }

            // go over the candidates and look for candidate in current cell
            // if cell is peak candidate, add 1 to p, else add 0
            unsigned short t = 0;
            for (unsigned i = 0; i < nActive; i++) {
              unsigned id1 = candidates[i];
              unsigned x1 = 0;
              unsigned y1 = 0;
              hp.id(id1, x1, y1);
              if (xx == x1 && yy == y1) {
                t = 1;
                ot = true;
                break;
              }
            }
            p.push_back(t);
          }
        }
        //...find 4 cells...end

        // p = [n a b c d]
        // n: number of 2x2 square
        // a b c d: on/off flag for cells in 2x2 square
        // shape:
        //  c d
        //  a b
        if (ot == true) {
          p1m.push_back(p);
        }

        p.clear();
      }
    }
    if (TRGDebug::level()) cout << TRGDebug::tab() << "size of p1m=" << p1m.size() << endl;
    if (TRGDebug::level()) cout << TRGDebug::tab() << "~~~~~~~~~~~~~~~~~~~~~~~~~pattern1~~~~~~~~~~~~~~~~~~~" << endl;
    //...create pattern1...end   (output p1m)


    //...Pattern2 & Find Peak...begin
    if (TRGDebug::level()) cout << TRGDebug::tab() << ">>>>>>>>>>Pattern 2 & Find Peak Begin!!!>>>>>>>>>>" << endl;

    vector<unsigned> p0(5, 0);
    vector<vector<unsigned>> op2;

    // loop over 2x2 peak candidates
    for (unsigned short i = 0; i < p1m.size(); i++) {
      unsigned short j = p1m[i][0]; // 2x2 square number (starting at 1)
      unsigned short a = 0;
      bool p1rel = false;
      if (TRGDebug::level()) cout << TRGDebug::tab() << "no." << j << endl;

      // XYZ  (begin)
      // check for connections to neighboring 2x2 candidates
      // if connection is found, continue to next candidate

      //X (horizontal connection to the left)
      if ((j % nX2) == 1) {
        a = j + nX2 - 1;
      } else {
        a = j - 1;
      }
      // loop over rest of candidates
      for (unsigned k = 0; k < p1m.size(); k++) {
        if (a == p1m[k][0]) {
          // check connection to left neighbor
          // by predefined subpattern in 2x2 square
          if (!rlrel(p1m[k], p1m[i])) {
            if (TRGDebug::level()) cout << TRGDebug::tab() << "no." <<  j << " & no." << a << " / X no rel" << endl;
            p1rel = false;
          } else {
            if (TRGDebug::level()) cout << TRGDebug::tab() << "no." <<  j << " & no." << a << " / X rel" << endl;
            p1rel = true;
          }
          break;
        }
      }
      if (p1rel) {
        continue;
      }

      //Y (vertical connection to lower neighbor)
      if (j > nX2) {
        a = j - nX2;
        // loop over rest of candidates
        for (unsigned k = 0; k < p1m.size(); k++) {
          if (a == p1m[k][0]) {
            // check connection to lower neighbor
            // by predefined subpattern in 2x2 square
            if (!udrel(p1m[k], p1m[i])) {
              if (TRGDebug::level()) cout << TRGDebug::tab() << "no." <<  j << " & no." << a << " / Y no rel" << endl;
              p1rel = false;
            } else {
              if (TRGDebug::level()) cout << TRGDebug::tab() << "no." <<  j << " & no." << a << " / Y rel" << endl;
              p1rel = true;
            }
            break;
          }
        }
        if (p1rel) {
          continue;
        }


        //Z (diagonal connection to lower left)
        if ((j % nX2) == 1) {
          a = j - 1;
        } else         {
          a = j - nX2 - 1;
        }
        // loop over test of candidates
        for (unsigned k = 0; k < p1m.size(); k++) {
          if (a == p1m[k][0]) {
            // check connection to lower left neighbor
            // by predefined subpattern in 2x2 square
            if (!mirel(p1m[k], p1m[i])) {
              if (TRGDebug::level()) cout << TRGDebug::tab() << "no." <<  j << " & no." << a << " / Z no rel" << endl;
              p1rel = false;
            } else {
              if (TRGDebug::level()) cout << TRGDebug::tab() << "no." <<  j << " & no." << a << " / Z rel" << endl;
              p1rel = true;
            }
            break;
          }
        }
        if (p1rel) {
          continue;
        }
      }
      // XYZ (end)

      // Pattern2 value (begin)
      // make 3x2 rectangle of 2x2 squares
      // p2v = [A B C D E F] (numbers of 2x2 squares in 3x2 rectangle)
      // A: number of current 2x2 candidate
      // shape of p2v:
      // E F
      // C D
      // A B
      vector<unsigned> p2v;
      for (unsigned ip2 = 0; ip2 < 3; ++ip2) {
        for (unsigned jp2 = 0; jp2 < 2; ++jp2) {
          p2v.push_back(j + jp2 + ip2 * nX2);
        }
        if ((j % nX2) == 0) {
          p2v[ip2 * 3 + 1] -= nX2;
        }
      }
      // Pattern2 value(End)

      // make Pattern2(begin)
      // get subpattern for each 2x2 square within 3x2 rectangle
      // stored in op2

      // loop over 3x2 rectangle
      for (unsigned short imp2 = 0; imp2 < p2v.size(); imp2++) {
        unsigned short p2v_i = p2v[imp2];
        // loop over 2x2 candidates
        for (unsigned short jmp2 = 0; jmp2 < p1m.size(); jmp2++) {
          unsigned short p1m_no = p1m[jmp2][0];
          // if none of match number in p1m then pass this scan
          if (p2v_i != p1m_no) {
            if (jmp2 == (p1m.size() - 1)) {
              // if no match is found use default (0 0 0 0 0)
              op2.push_back(p0);
            }
            continue;
          }
          op2.push_back(p1m[jmp2]);

          break;
        }
      }
      // make Pattern2(End)

      // Pattern2 relation(Begin)
      // go over 3x2 rectangle and keep only cells connected to lower left 2x2 square
      vector<vector<unsigned>> final_op2;
      vector<unsigned> p2_state;
      // A (start point)
      final_op2.push_back(op2[0]);
      p2_state.push_back(1);
      // B (keep if connected to A)
      if (rlrel(op2[0], op2[1])) {
        final_op2.push_back(op2[1]);
        p2_state.push_back(1);
      } else {
        final_op2.push_back(p0);
        p2_state.push_back(0);
      }
      // C (keep if connected to A)
      if (udrel(op2[0], op2[2])) {
        final_op2.push_back(op2[2]);
        p2_state.push_back(1);
      } else {
        final_op2.push_back(p0);
        p2_state.push_back(0);
      }
      // D (keep connected to A, B or D)
      if (mirel(op2[0], op2[3]) || udrel(op2[1], op2[3]) || rlrel(op2[2], op2[3])) {
        final_op2.push_back(op2[3]);
        p2_state.push_back(1);
      } else {
        final_op2.push_back(p0);
        p2_state.push_back(0);
      }
      // E (keep if connected to C)
      if (udrel(op2[2], op2[4])) {
        final_op2.push_back(op2[4]);
        p2_state.push_back(1);
      } else {
        final_op2.push_back(p0);
        p2_state.push_back(0);
      }
      // F (keep if connected to C, D or E)
      if (mirel(op2[2], op2[5]) || udrel(op2[3], op2[5]) || rlrel(op2[4], op2[5])) {
        final_op2.push_back(op2[5]);
        p2_state.push_back(1);
      } else {
        final_op2.push_back(p0);
        p2_state.push_back(0);
      }
      // TODO: should compare connection to final_op2 instead of op2?
      // otherwise e.g. op2 = A C-E with connection C-E
      // would give final_op2 = A . E (should be A . .)

      // Pattern2 relation(End)

      // Find center peak(begin)
      unsigned short fcpi = 0;  // number of peak in 2x2 square (start: 1)
      unsigned short fcpn = 0;  // number of peak 2x2 square (start: 1)
      unsigned short fcpx = 0;  // x index in original hough plane (start: 0)
      unsigned short fcpxs = 0; // x index in 2x2 square (0, 1)
      unsigned short fcpy = 0;  // y index in original hough plane (start: 0)
      unsigned short fcpys = 0; // y index in 2x2 square (0, 1)

      // p2_state:               vector of on/off flags for 3x3 square
      // FindCP1(p2_state):      peak within 3x3 square by predefined pattern
      // op2[FindCP1(p2_state)]: 2x2 subpattern of this peak
      // FindP1C(...):           peak within 2x2 square by predefined pattern
      fcpi = FindP1C(op2[FindCP1(p2_state)]);

      fcpn = op2[FindCP1(p2_state)][0];

      // get x index
      if (fcpi >= 3) {
        fcpxs = fcpi - 3;
      } else {
        fcpxs = fcpi - 1;
      }
      fcpx = ((fcpn - 1) % nX2) * 2 + fcpxs;
      // Plus plane transform back to original numbering
      if ((hp.name()) == "circle hough plus") {
        fcpx = hp.nX() - fcpx - 1;
      }

      // get y index
      if (fcpi >= 3) {
        fcpys = 1;
      }
      fcpy = fcpy + ((fcpn - 1) / nX2) * 2 + fcpys;

      if (TRGDebug::level()) cout << TRGDebug::tab() << "center of peak  x=" << fcpx << "  y=" << fcpy << endl;

      p.push_back(fcpx);
      p.push_back(fcpy);
      peak_xy.push_back(p);
      p.clear();
      // Find center peak(end)

      if (TRGDebug::level()) cout << TRGDebug::tab() << "~~~~~~~~~~Pattern 2 & Find Peak End!!!~~~~~~~~~~" << endl;
      p2_state.clear();
      final_op2.clear();
      p2v.clear();
      op2.clear();
    } // end of loop over 2x2 candidates

    //... Pattern 2...end

    if (TRGDebug::level())
      cout << TRGDebug::tab() << "total peaks=" << peak_xy.size() << endl;

    p1m.clear();

    TRGDebug::leaveStage(sn);
  }

} // namespace Belle2
