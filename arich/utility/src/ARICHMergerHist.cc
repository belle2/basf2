/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 *                                                                        *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <arich/utility/ARICHMergerHist.h>
#include <framework/logging/Logger.h>

#include <TLine.h>
#include <TColor.h>
#include <TGraph.h>
#include <TAxis.h>
#include <TLatex.h>
#include <map>

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace Belle2;

// Map: MergerID -> internal index (1..72)
static const std::map<int, int> MERGER_TO_INTERNAL = {
  {53, 1}, {24, 2}, {22, 3}, {11, 4}, {2, 5}, {12, 6}, {76, 7}, {34, 8}, {32, 9}, {72, 10},
  {7, 11}, {66, 12}, {97, 13}, {63, 14}, {19, 15}, {57, 16}, {5, 17}, {79, 18}, {6, 19}, {80, 20},
  {10, 21}, {37, 22}, {60, 23}, {35, 24}, {30, 25}, {41, 26}, {69, 27}, {20, 28}, {55, 29}, {47, 30},
  {71, 31}, {15, 32}, {14, 33}, {74, 34}, {77, 35}, {64, 36}, {62, 37}, {67, 38}, {82, 39}, {56, 40},
  {68, 41}, {78, 42}, {46, 43}, {81, 44}, {27, 45}, {39, 46}, {18, 47}, {44, 48}, {17, 49}, {25, 50},
  {16, 51}, {51, 52}, {1, 53}, {61, 54}, {54, 55}, {48, 56}, {8, 57}, {29, 58}, {3, 59}, {73, 60},
  {93, 61}, {38, 62}, {58, 63}, {59, 64}, {50, 65}, {26, 66}, {31, 67}, {33, 68}, {49, 69}, {65, 70},
  {52, 71}, {75, 72}
};

// --------------------------------------------------
// Geometry
// --------------------------------------------------
static const double SECTOR_CENTER_DEG[7] =
{0.0, 30.0, 90.0, 150.0, 210.0, 270.0, 330.0};

// Global drawing scale (conversion to ROOT coordinates)
static const double GEOM_SCALE = 11.0;

// Block size
static const double BLOCK_W = 1.3 * GEOM_SCALE;
static const double BLOCK_H = 0.8 * GEOM_SCALE;

// Ring radii
static const double R1 = 6.1  * GEOM_SCALE;
static const double R2 = 7.7  * GEOM_SCALE;
static const double R3 = 9.5  * GEOM_SCALE;
static const double R4 = 11.25 * GEOM_SCALE;

static const double ROW1_OFFSETS[3] = {+21.5,  0.0, -21.5};
static const double ROW2_OFFSETS[4] = {+22.5, +9.5, -9.5, -22.5};
static const double ROW3_OFFSETS[4] = {+19.0,  +8.0,  -8.0, -19.0};
static const double ROW4_OFFSETS[1] = {+1.0};
// --------------------------------------------------

void ARICHMergerHist::pol2xy(double r, double angDeg, double& x, double& y)
{
  const double th = angDeg * M_PI / 180.0;
  x = r * std::cos(th);
  y = r * std::sin(th);
}

ARICHMergerHist::ARICHMergerHist(const char* name,
                                 const char* title,
                                 double globalRotationDeg,
                                 bool rightToLeftNumbering)
  : TH2Poly()
{
  m_globalRotationDeg = globalRotationDeg;
  m_rightToLeft = rightToLeftNumbering;
  m_merger2bin.assign(72, 0);

  SetName(name);
  SetTitle(title);

  int polyBin = 1;
  int mergerNumber = 1;

  auto add_block = [&](double r, double angDeg) {

    double cx, cy;
    pol2xy(r, angDeg, cx, cy);

    const double rot = (angDeg + 90.0) * M_PI / 180.0;
    const double c = std::cos(rot);
    const double s = std::sin(rot);

    const double hx = BLOCK_W / 2.0;
    const double hy = BLOCK_H / 2.0;

    const double lx[5] = {-hx, +hx, +hx, -hx, -hx};
    const double ly[5] = {-hy, -hy, +hy, +hy, -hy};

    double gx[5], gy[5];
    for (int i = 0; i < 5; ++i) {
      const double rx = lx[i] * c - ly[i] * s;
      const double ry = lx[i] * s + ly[i] * c;
      gx[i] = rx + cx;
      gy[i] = ry + cy;
    }

    auto* g = new TGraph(5, gx, gy);
    AddBin(g);

    if (mergerNumber >= 1 && mergerNumber <= 72)
      m_merger2bin[mergerNumber - 1] = polyBin;

    ++polyBin;
    ++mergerNumber;
  };

  auto add_ring = [&](double r, const double * offsets, int nOffsets, double centerDeg) {
    if (m_rightToLeft) {
      for (int i = nOffsets - 1; i >= 0; --i)
        add_block(r, centerDeg + offsets[i]);
    } else {
      for (int i = 0; i < nOffsets; ++i)
        add_block(r, centerDeg + offsets[i]);
    }
  };

  for (int X = 1; X <= 6; ++X) {
    const double cdeg = SECTOR_CENTER_DEG[X] + m_globalRotationDeg;

    add_ring(R1, ROW1_OFFSETS, 3, cdeg);
    add_ring(R2, ROW2_OFFSETS, 4, cdeg);
    add_ring(R3, ROW3_OFFSETS, 4, cdeg);
    add_block(R4, cdeg + ROW4_OFFSETS[0]);
  }

  if (mergerNumber != 73)
    B2ERROR("ARICHMergerHist internal error: expected 72 bins, got "
            << mergerNumber - 1);

  SetOption("colz");
  SetStats(0);
  GetXaxis()->SetLimits(-140., 140.);
  GetYaxis()->SetLimits(-140., 140.);
}

// --------------------------------------------------
// Using 1..72 Merger position (1_1,...,1_12,...,6_12)
// --------------------------------------------------
void ARICHMergerHist::fillFromTH1(TH1* hist)
{
  if (!hist) return;

  if (hist->GetNbinsX() != 72) {
    B2ERROR("ARICHMergerHist::fillFromTH1: histogram must have 72 bins (got "
            << hist->GetNbinsX() << ")");
    return;
  }

  for (int i = 1; i <= 72; ++i) {
    const int b = m_merger2bin[i - 1];
    SetBinContent(b, hist->GetBinContent(i));
  }
}

// --------------------------------------------------
// Using MergerID numbering
// --------------------------------------------------
void ARICHMergerHist::setBinContent(unsigned mergerID, double value)
{
  if (mergerID < 1 || mergerID > 100) return;

  int internal = MERGER_TO_INTERNAL.at(mergerID);
  int polybin  = m_merger2bin[internal - 1];

  SetBinContent(polybin, value);
}

void ARICHMergerHist::Draw(Option_t* option)
{
  TH2Poly::Draw(option);

  // -------------------------------------------------
  // Sector lines
  // -------------------------------------------------
  const double rInner = 50.0;
  const double rOuter = 130.0;

  for (int k = 0; k < 6; ++k) {
    const double aDeg = k * 60.0 + m_globalRotationDeg;

    double x1, y1, x2, y2;
    pol2xy(rInner, aDeg, x1, y1);
    pol2xy(rOuter, aDeg, x2, y2);

    m_lines[k] = TLine(x1, y1, x2, y2);
    m_lines[k].SetLineWidth(2);
    m_lines[k].SetLineColor(kGray + 2);
    m_lines[k].Draw();
  }

  // -------------------------------------------------
  // Sector labels S-1 ... S-6
  // -------------------------------------------------
  const double rLabel = 45.0;

  for (int s = 0; s < 6; ++s) {
    const double centerDeg = SECTOR_CENTER_DEG[s + 1] + m_globalRotationDeg;

    double x, y;
    pol2xy(rLabel, centerDeg, x, y);

    auto* txt = new TLatex(x, y, Form("S-%d", s + 1));
    txt->SetBit(kCanDelete);
    txt->SetTextAlign(22);
    txt->SetTextFont(62);
    txt->SetTextSize(0.030);
    txt->Draw();
  }
}