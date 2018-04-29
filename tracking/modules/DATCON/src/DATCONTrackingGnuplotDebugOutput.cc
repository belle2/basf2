/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Michael Schnell, Christian Wessel                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <tracking/modules/DATCON/DATCONTrackingModule.h>
#include <tracking/modules/DATCON/gplota.h>


using namespace std;
using namespace Belle2;

/* Hack for re-plot */
int gplot_firsta = 1;

void
DATCONTrackingModule::houghTrafoPlot(bool u_side)
{
  FILE* fp;
  TVector2 pos;
  ostringstream fmt_s;
  string v_path = "dbg/v_hough.plot";
  string u_path = "dbg/u_hough.plot";
  string v_load = "v_rect.plot";
  string u_load = "u_rect.plot";
  string fmt;
  houghPair hp;
  VxdID sensorID;

  if (!u_side) {
    if ((fp = gplotCreate(v_path.c_str(), NULL, GPLOT_TMPL1)) == NULL) {
      B2ERROR("Cannot open hough debug file!");
      return;
    }
    gplotSetLoad(fp, v_load.c_str());
    for (auto it = vHough.begin(); it != vHough.end(); ++it) {
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
    if ((fp = gplotCreate(u_path.c_str(), NULL, GPLOT_TMPL1)) == NULL) {
      B2ERROR("Cannot open hough debug file!");
      return;
    }
    gplotSetLoad(fp, u_load.c_str());
    for (auto it = uHough.begin(); it != uHough.end(); ++it) {
      hp = it->second;
      sensorID = hp.first;
      pos = hp.second;
      fmt_s.str("");
      fmt_s.clear();
      if (m_xyHoughUside) {
        /* Old version for Michael's Hough trafo */
        fmt_s << "plot " << pos.X() << " * 2.0 * cos(x) + " << pos.Y() << " * 2.0 * sin(x) notitle linestyle "
              << sensorID.getLayerNumber() + 10 << endl;
      } else if (m_rphiHoughUside) {
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
DATCONTrackingModule::gplotRect(const string name, vector<houghDbgPair>& hp)
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
DATCONTrackingModule::gplotRect(const string name, vector<DATCONHoughCand>& cands)
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
DATCONTrackingModule::gplotCreate(const char* name, char* dat, int what)
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
      fprintf(fp, "%s", gplot_text2a.c_str());
      break;
    case GPLOT_TMPL1:
      fprintf(fp, "%s", gplot_tmpl1a.c_str());
      break;
  }

  return fp;
}


void
DATCONTrackingModule::gplotSetLoad(FILE* fp, const char* option)
{
  if (fp == NULL) {
    fp = stdout;
  }

  fprintf(fp, "load '%s'\n", option);
}

void
DATCONTrackingModule::gplotSetOpt(FILE* fp, const char* option)
{
  if (fp == NULL) {
    fp = stdout;
  }

  fprintf(fp, "set %s\n", option);
}


void
DATCONTrackingModule::gplotInsert(FILE* fp, const char* dat, int what)
{
  switch (what) {
    case GPLOT_LINE:
      if (!gplot_firsta) {
        fprintf(fp, "%s", "re");
      }
      fprintf(fp, "%s", dat);
      gplot_firsta = 0;
      break;
    case GPLOT_INPLOT:
      if (gplot_firsta) {
        fprintf(stdout, "%s", "plot \"-\" notitle\n");

        gplot_firsta = 0;
      }

      if (dat != NULL) {
        fprintf(fp, "%s", dat);
      }
      break;
    case GPLOT_PLOTGEO:
      if (dat != NULL) {
        fprintf(fp, "%s", gplot_geoa.c_str());
      }
      break;
  }
}

void
DATCONTrackingModule::gplotClose(FILE* fp, int what)
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

  gplot_firsta = 1;
}
