/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell, Christian Wessel                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/SVDHoughTracking/SVDHoughTrackingModule.h>
#include <vxd/geometry/GeoCache.h>
//#include <vxd/geometry/GeoVXDPosition.h>
#include <geometry/bfieldmap/BFieldMap.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <tracking/dataobjects/SVDHoughCluster.h>
#include <tracking/dataobjects/SVDHoughTrack.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDSimHit.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <vector>
#include <set>
#include <map>
#include <cmath>
#include <root/TMath.h>
#include <root/TGeoMatrix.h>
#include <root/TRandom.h>


using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;

/* Hack for re-plot */
int gplot_first = 1;

void
SVDHoughTrackingModule::houghTrafoPlot(bool p_side)
{
  FILE* fp;
  TVector2 pos;
  ostringstream fmt_s;
  string n_path = "dbg/n_hough.plot";
  string n_pathX = "dbg/n_houghX.plot";
  string n_pathY = "dbg/n_houghY.plot";
  string p_path = "dbg/p_hough.plot";
  string n_load = "n_rect.plot";
  string n_loadX = "n_rectX.plot";
  string n_loadY = "n_rectY.plot";
  string p_load = "p_rect.plot";
  string fmt;
  houghPair hp;
  VxdID sensorID;

  if (!p_side) {
    if ((fp = gplotCreate(n_path.c_str(), NULL, GPLOT_TMPL1)) == NULL) {
      B2ERROR("Cannot open hough debug file!");
      return;
    }
    gplotSetLoad(fp, n_load.c_str());
    for (auto it = n_hough.begin(); it != n_hough.end(); ++it) {
      hp = it->second;
      sensorID = hp.first;
      pos = hp.second;
      fmt_s.str("");
      fmt_s.clear();
      fmt_s << "plot " << pos.X() << " * cos(x) + " << pos.Y() << " * sin(x) notitle linestyle "
            << sensorID.getLayerNumber() + 10 << endl;
      fmt = fmt_s.str();
      gplotInsert(fp, fmt.c_str(), GPLOT_LINE);
    }
  } else {
    if ((fp = gplotCreate(p_path.c_str(), NULL, GPLOT_TMPL1)) == NULL) {
      B2ERROR("Cannot open hough debug file!");
      return;
    }
    gplotSetLoad(fp, p_load.c_str());
    for (auto it = p_hough.begin(); it != p_hough.end(); ++it) {
      hp = it->second;
      sensorID = hp.first;
      pos = hp.second;
      fmt_s.str("");
      fmt_s.clear();
      if (m_xyHoughPside) {
        /* Old version for Michael's Hough trafo */
        fmt_s << "plot " << pos.X() << " * 2.0 * cos(x) + " << pos.Y() << " * 2.0 * sin(x) notitle linestyle "
              << sensorID.getLayerNumber() + 10 << endl;
      } else if (m_rphiHoughPside) {
        /* New version */
        fmt_s << "plot " << 2.0 / pos.Y() << " * sin(x - " << pos.X() << ") notitle linestyle " << sensorID.getLayerNumber() + 10 << endl;
      }
      fmt = fmt_s.str();
      gplotInsert(fp, fmt.c_str(), GPLOT_LINE);
    }
  }
  gplotClose(fp, GPLOT_INPLOT);
}


/*
 * Create rectangulars for debugging in gnuplot
 */
void
SVDHoughTrackingModule::gplotRect(const string name, vector<houghDbgPair>& hp)
{
  unsigned int iteration;
  ofstream os;
  TVector2 v1, v2;
  coord2dPair hc;
  int i = 1;

  os.open(name.c_str(), ofstream::out);

  for (auto it = hp.begin(); it != hp.end(); ++it) {
    iteration = it->first;
    hc = it->second;
    v1 = hc.first;
    v2 = hc.second;
    os << "set object " << i << " rect from " << v1.X() << ", " << v1.Y() << " to " << v2.X() << ", " << v2.Y();
    os << " fc rgb \"" << rectColor[iteration % 8] << "\" fs solid 0.5 behind" << endl;
    i++;
  }

  os.close();
}


/*
 * Create rectangulars for debugging in gnuplot
 */
void
SVDHoughTrackingModule::gplotRect(const string name, vector<SVDHoughCand>& cands)
{
  ofstream os;
  TVector2 v1, v2;
  coord2dPair hc;

  os.open(name.c_str(), ofstream::out);

  for (auto it = cands.begin(); it != cands.end(); ++it) {
    hc = it->getCoord();
    v1 = hc.first;
    v2 = hc.second;
    os << "set object rect from " << v1.X() << ", " << v1.Y() << " to " << v2.X() << ", " << v2.Y();
    os << " fc rgb \"" << "blue" << "\" fs solid 0.5 behind" << endl;
  }

  os.close();
}


FILE*
SVDHoughTrackingModule::gplotCreate(const char* name, char* dat, int what)
{
  FILE* fp;

  if (name == NULL) {
    fp = stdout;
  } else {
    if ((fp = fopen(name, "w")) == NULL) {
      perror("Error: fopen()");

      return NULL;
    }
  }

  if (dat == NULL) {
    B2DEBUG(400, "gplotCreate: Dat is zero");
  }

  switch (what) {
    case GPLOT_INPLOT:
      fprintf(fp, "%s", gplot_text2.c_str());
      break;
    case GPLOT_TMPL1:
      fprintf(fp, "%s", gplot_tmpl1.c_str());
      break;
  }

  return fp;
}


void
SVDHoughTrackingModule::gplotSetLoad(FILE* fp, const char* option)
{
  if (fp == NULL) {
    fp = stdout;
  }

  fprintf(fp, "load '%s'\n", option);
}

void
SVDHoughTrackingModule::gplotSetOpt(FILE* fp, const char* option)
{
  if (fp == NULL) {
    fp = stdout;
  }

  fprintf(fp, "set %s\n", option);
}


void
SVDHoughTrackingModule::gplotInsert(FILE* fp, const char* dat, int what)
{
  switch (what) {
    case GPLOT_LINE:
      if (!gplot_first) {
        fprintf(fp, "%s", "re");
      }
      fprintf(fp, "%s", dat);
      gplot_first = 0;
      break;
    case GPLOT_INPLOT:
      if (gplot_first) {
        fprintf(stdout, "%s", "plot \"-\" notitle\n");

        gplot_first = 0;
      }

      if (dat != NULL) {
        fprintf(fp, "%s", dat);
      }
      break;
    case GPLOT_PLOTGEO:
      if (dat != NULL) {
        fprintf(fp, "%s", gplot_geo.c_str());
      }
      break;
  }
}

void
SVDHoughTrackingModule::gplotClose(FILE* fp, int what)
{
  if (fp == NULL) {
    fp = stdout;
  }

  switch (what) {
    case GPLOT_INPLOT:
      fprintf(fp, "%s%s", "\n", "pause -1\n");
      break;
  }

  if (fp != NULL) {
    fclose(fp);
  }

  gplot_first = 1;
}