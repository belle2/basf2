/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 *                                                                        *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <arich/utility/ARICHMergerHist.h>
#include <framework/logging/Logger.h>
#include <TLine.h>
#include <TColor.h>
#include <cmath>
#include <TGraph.h>
#include <TAxis.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace Belle2;

// ---- Geometry ----
static const double SECTOR_CENTER_DEG[7] =
{0.0, 30.0, 90.0, 150.0, 210.0, 270.0, 330.0};

static const double BLOCK_W = 14.0;
static const double BLOCK_H = 8.0;

static const double R1 = 70.0;
static const double R2 = 88.0;
static const double R3 = 106.0;
static const double R4 = 122.0;

static const double ROW1_OFFSETS[3] = {+18.0, 0.0, -18.0};
static const double ROW2_OFFSETS[4] = {+23.0, +13.0, -13.0, -23.0};
static const double ROW3_OFFSETS[4] = {+22.0, +9.0,  -9.0,  -22.0};
static const double ROW4_OFFSETS[1] = {0.0};

static const double RLINE = 180.0;
// ---------------------------------------------

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

void ARICHMergerHist::Draw(Option_t* option)
{
  TH2Poly::Draw(option);

  for (int k = 0; k < 6; ++k) {
    const double aDeg = k * 60.0 + m_globalRotationDeg;

    double x1, y1, x2, y2;
    pol2xy(-RLINE, aDeg, x1, y1);
    pol2xy(+RLINE, aDeg, x2, y2);

    m_lines[k] = TLine(x1, y1, x2, y2);
    m_lines[k].SetLineWidth(2);
    m_lines[k].SetLineColor(kRed);
    m_lines[k].Draw();
  }
}