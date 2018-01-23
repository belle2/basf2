/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Viktor Trusov, Thomas Hauth, Nils Braun                  *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/
#include <tracking/trackFindingCDC/legendre/quadtree/AxialHitQuadTreeProcessor.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <array>
#include <vector>

#include <TF1.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TAxis.h>
#include <TLine.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  bool sameSign(double n1, double n2, double n3, double n4)
  {
    return ((n1 > 0 && n2 > 0 && n3 > 0 && n4 > 0) || (n1 < 0 && n2 < 0 && n3 < 0 && n4 < 0));
  }

  using YSpan = AxialHitQuadTreeProcessor::YSpan;
  YSpan splitCurvSpan(const YSpan& curvSpan, int nodeLevel, int lastLevel, int j)
  {
    const float meanCurv = curvSpan[0] + (curvSpan[1] - curvSpan[0]) / 2.0;
    const std::array<float, 3> binBounds{curvSpan[0], meanCurv, curvSpan[1]};
    const float binWidth = binBounds[j + 1] - binBounds[j];

    const bool standardBinning = (nodeLevel <= lastLevel - 7) or (std::fabs(meanCurv) <= 0.005);
    if (standardBinning) {
      // B2INFO("Case 1* " << meanCurv << " " << (meanCurv <= 0.005));
      float curv1 = binBounds[j];
      float curv2 = binBounds[j + 1];

      // Standard bin division
      return {curv1, curv2};
    }

    // Non-standard binning
    // For level 6 to 7 only expand 1 / 4, for higher levels expand  1 / 8.
    // (assuming last level == 12)
    if (nodeLevel < lastLevel - 5) {
      // B2INFO("Case 2*");
      float curv1 = binBounds[j] - binWidth / 4.;
      float curv2 = binBounds[j + 1] + binWidth / 4.;
      return {curv1, curv2};
    } else {
      // B2INFO("Case 3*");
      float curv1 = binBounds[j] - binWidth / 8.;
      float curv2 = binBounds[j + 1] + binWidth / 8.;
      return {curv1, curv2};
    }
  }

}

std::vector<float> AxialHitQuadTreeProcessor::createCurvBound(YSpan curvSpan, int lastLevel)
{
  std::vector<YSpan> spans{{curvSpan}};

  std::vector<YSpan> nextSpans;
  for (int level = 1; level <= lastLevel; ++level) {
    nextSpans.clear();
    for (const YSpan& span : spans) {
      nextSpans.push_back(splitCurvSpan(span, level, lastLevel, 0));
      nextSpans.push_back(splitCurvSpan(span, level, lastLevel, 1));
    }
    spans.swap(nextSpans);
  }

  std::vector<float> bounds;
  for (const YSpan& span : spans) {
    bounds.push_back(span[0]);
    bounds.push_back(span[1]);
  }

  assert(bounds.size() == std::pow(2, lastLevel));
  return bounds;
}

const LookupTable<Vector2D>& AxialHitQuadTreeProcessor::getCosSinLookupTable()
{
  static const int maxLevel = PrecisionUtil::getLookupGridLevel();
  static const int nBins = std::pow(2, maxLevel);
  static LookupTable<Vector2D> trigonometricLookUpTable(&Vector2D::Phi, nBins, -M_PI, M_PI);
  return trigonometricLookUpTable;
}

AxialHitQuadTreeProcessor::AxialHitQuadTreeProcessor(int lastLevel,
                                                     int seedLevel,
                                                     const XYSpans& ranges,
                                                     PrecisionUtil::PrecisionFunction precisionFunction)
  : QuadTreeProcessor(lastLevel, seedLevel, ranges)
  , m_cosSinLookupTable(&getCosSinLookupTable())
  , m_localOrigin(0.0, 0.0)
  , m_precisionFunction(precisionFunction)
{
  m_twoSidedPhaseSpace = m_quadTree->getYMin() * m_quadTree->getYMax() < 0;
}

AxialHitQuadTreeProcessor::AxialHitQuadTreeProcessor(const Vector2D& localOrigin,
                                                     const YSpan& curvSpan,
                                                     const LookupTable<Vector2D>* cosSinLookupTable)
  : QuadTreeProcessor(0, 0, { {0, cosSinLookupTable->getNPoints() - 1}, curvSpan})
, m_cosSinLookupTable(cosSinLookupTable)
, m_localOrigin(localOrigin)
{
  // Never use two sided mode in off origin extension
  m_twoSidedPhaseSpace = false;
}


bool AxialHitQuadTreeProcessor::isLeaf(QuadTree* node) const
{
  if (node->getLevel() <= 6) return false;
  if (node->getLevel() >= getLastLevel()) return true;

  const double nodeResolution = fabs(node->getYMin() - node->getYMax());
  const double meanCurv = (node->getYMax() + node->getYMin()) / 2;

  const double resolution = m_precisionFunction(meanCurv);
  if (resolution >= nodeResolution) return true;

  return false;
}

AxialHitQuadTreeProcessor::XYSpans
AxialHitQuadTreeProcessor::createChild(QuadTree* node, int i, int j) const
{
  const int nodeLevel = node->getLevel();
  const int lastLevel = getLastLevel();
  const float meanCurv = std::fabs(node->getYMax() + node->getYMin()) / 2;

  // Expand bins for all nodes 7 levels before the last level (for lastLevel = 12 starting at 6)
  // but only in a curvature region higher than 0.005. Lower than that use always standard.
  bool standardBinning = (nodeLevel <= lastLevel - 7) or (meanCurv <= 0.005);

  if (standardBinning) {
    float r1 = node->getYLowerBound(j);
    float r2 = node->getYUpperBound(j);
    long theta1 = node->getXLowerBound(i);
    long theta2 = node->getXUpperBound(i);

    // Standard bin division
    return XYSpans({theta1, theta2}, {r1, r2});
  }

  // Non-standard binning
  // For level 6 to 7 only expand 1 / 4, for higher levels expand  1 / 8.
  // (assuming last level == 12)
  if (nodeLevel < lastLevel - 5) {
    float r1 = node->getYLowerBound(j) - node->getYBinWidth(j) / 4.;
    float r2 = node->getYUpperBound(j) + node->getYBinWidth(j) / 4.;

    // long extension = pow(2, lastLevel - nodeLevel) / 4; is same as:
    long extension = pow(2, lastLevel - nodeLevel - 2);

    long theta1 = node->getXLowerBound(i) - extension;
    if (theta1 < 0) theta1 = 0;

    long theta2 = node->getXUpperBound(i) + extension;
    if (theta2 >= m_cosSinLookupTable->getNPoints()) {
      theta2 = m_cosSinLookupTable->getNPoints() - 1;
    }

    return XYSpans({theta1, theta2}, {r1, r2});
  } else {
    float r1 = node->getYLowerBound(j) - node->getYBinWidth(j) / 8.;
    float r2 = node->getYUpperBound(j) + node->getYBinWidth(j) / 8.;

    // long extension = pow(2, lastLevel - nodeLevel) / 8; is same as
    long extension = pow(2, lastLevel - nodeLevel - 3);

    long theta1 = node->getXLowerBound(i) - extension;
    if (theta1 < 0) theta1 = 0;

    long theta2 = node->getXUpperBound(i) + extension;
    if (theta2 >= m_cosSinLookupTable->getNPoints()) {
      theta2 = m_cosSinLookupTable->getNPoints() - 1;
    }

    return XYSpans({theta1, theta2}, {r1, r2});
  }
}

bool AxialHitQuadTreeProcessor::isInNode(QuadTree* node, const CDCWireHit* wireHit) const
{
  // Check whether the hit lies in the forward direction
  if (node->getLevel() <= 4 and m_twoSidedPhaseSpace and node->getYMin() > -c_curlCurv and
      node->getYMax() < c_curlCurv) {
    if (not checkDerivative(node, wireHit)) return false;
  }

  const double& l = wireHit->getRefDriftLength();
  const Vector2D& pos2D = wireHit->getRefPos2D() - m_localOrigin;
  double r2 = pos2D.normSquared() - l * l;

  using Quadlet = std::array<std::array<float, 2>, 2>;
  Quadlet distRight{};
  Quadlet distLeft{};

  // get top and bottom borders of the node
  float rMin = node->getYMin() * r2 / 2;
  float rMax = node->getYMax() * r2 / 2;

  // get left and right borders of the node
  long thetaMin = node->getXMin();
  long thetaMax = node->getXMax();

  const Vector2D& thetaVecMin = m_cosSinLookupTable->at(thetaMin);
  const Vector2D& thetaVecMax = m_cosSinLookupTable->at(thetaMax);

  float rHitMin = thetaVecMin.dot(pos2D);
  float rHitMax = thetaVecMax.dot(pos2D);

  // compute sinograms at the left and right borders of the node
  float rHitMinRight = rHitMin - l;
  float rHitMaxRight = rHitMax - l;

  float rHitMinLeft = rHitMin + l;
  float rHitMaxLeft = rHitMax + l;

  // Compute distance from the sinograms to bottom and top borders of the node
  distRight[0][0] = rMin - rHitMinRight;
  distRight[0][1] = rMin - rHitMaxRight;
  distRight[1][0] = rMax - rHitMinRight;
  distRight[1][1] = rMax - rHitMaxRight;

  distLeft[0][0] = rMin - rHitMinLeft;
  distLeft[0][1] = rMin - rHitMaxLeft;
  distLeft[1][0] = rMax - rHitMinLeft;
  distLeft[1][1] = rMax - rHitMaxLeft;

  // Compare distance signes from sinograms to the node
  // Check right
  if (not sameSign(distRight[0][0], distRight[0][1], distRight[1][0], distRight[1][1])) {
    return true;
  }

  // Check left
  if (not sameSign(distLeft[0][0], distLeft[0][1], distLeft[1][0], distLeft[1][1])) {
    return true;
  }

  // Check the extremum
  float rHitMinExtr = thetaVecMin.cross(pos2D);
  float rHitMaxExtr = thetaVecMax.cross(pos2D);
  if (rHitMinExtr * rHitMaxExtr < 0.) return checkExtremum(node, wireHit);

  // Not contained
  return false;
}

bool AxialHitQuadTreeProcessor::checkDerivative(QuadTree* node, const CDCWireHit* wireHit) const
{
  const Vector2D& pos2D = wireHit->getRefPos2D() - m_localOrigin;

  long thetaMin = node->getXMin();
  long thetaMax = node->getXMax();

  const Vector2D& thetaVecMin = m_cosSinLookupTable->at(thetaMin);
  const Vector2D& thetaVecMax = m_cosSinLookupTable->at(thetaMax);

  float rMinD = thetaVecMin.cross(pos2D);
  float rMaxD = thetaVecMax.cross(pos2D);

  // Does not really make sense...
  if ((rMinD > 0) && (rMaxD * rMinD >= 0)) return true;
  if ((rMaxD * rMinD < 0)) return true;
  return false;
}

bool AxialHitQuadTreeProcessor::checkExtremum(QuadTree* node, const CDCWireHit* wireHit) const
{
  const double& l = wireHit->getRefDriftLength();
  const Vector2D& pos2D = wireHit->getRefPos2D() - m_localOrigin;
  double r2 = pos2D.normSquared() - l * l;

  // get left and right borders of the node
  long thetaMin = node->getXMin();
  long thetaMax = node->getXMax();

  const Vector2D& thetaVecMin = m_cosSinLookupTable->at(thetaMin);
  const Vector2D& thetaVecMax = m_cosSinLookupTable->at(thetaMax);

  if (not pos2D.isBetween(thetaVecMin, thetaVecMax)) return false;

  // compute sinograms at the position
  double r = pos2D.norm();
  float rRight = r - l;
  float rLeft = r + l;

  // get top and bottom borders of the node
  float rMin = node->getYMin() * r2 / 2;
  float rMax = node->getYMax() * r2 / 2;

  bool crossesRight = (rMin - rRight) * (rMax - rRight) < 0;
  bool crossesLeft = (rMin - rLeft) * (rMax - rLeft) < 0;
  return crossesRight or crossesLeft;
}

void AxialHitQuadTreeProcessor::drawNode()
{
  static int nevent(0);

  TCanvas* canv = new TCanvas("canv", "legendre transform", 0, 0, 1200, 600);
  canv->cd(1);
  TGraph* dummyGraph = new TGraph();
  dummyGraph->SetPoint(1, -3.1415, 0);
  dummyGraph->SetPoint(2, 3.1415, 0);
  dummyGraph->Draw("AP");
  dummyGraph->GetXaxis()->SetTitle("#theta");
  dummyGraph->GetYaxis()->SetTitle("#rho");
  dummyGraph->GetXaxis()->SetRangeUser(-3.1415, 3.1415);
  dummyGraph->GetYaxis()->SetRangeUser(0, 0.15);

  for (Item* item : m_quadTree->getItems()) {
    const CDCWireHit* wireHit = item->getPointer();
    const double& l = wireHit->getRefDriftLength();
    const Vector2D& pos2D = wireHit->getRefPos2D() - m_localOrigin;

    TF1* funct1 = new TF1("funct", "2*[0]*cos(x)/((1-sin(x))*[1]) ", -3.1415, 3.1415);
    funct1->SetLineWidth(1);
    double r2 = pos2D.normSquared() - l * l;
    double x = pos2D.x();

    funct1->SetParameters(x, r2);
    funct1->Draw("CSAME");
  }
  canv->Print(Form("legendreHits_%i.root", nevent));
  canv->Print(Form("legendreHits_%i.eps", nevent));
  canv->Print(Form("legendreHits_%i.png", nevent));

  nevent++;
}
