/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/legendre/quadtree/AxialHitQuadTreeProcessor.h>

#include <tracking/trackingUtilities/eventdata/hits/CDCWireHit.h>

#include <array>
#include <vector>

#include <TF1.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TAxis.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace TrackingUtilities;

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
  , m_precisionFunction(precisionFunction)
  , m_localOrigin(0.0, 0.0)
  , m_cosSinLookupTable(&getCosSinLookupTable())
{
  m_twoSidedPhaseSpace = m_quadTree->getYMin() * m_quadTree->getYMax() < 0;
}

AxialHitQuadTreeProcessor::AxialHitQuadTreeProcessor(const Vector2D& localOrigin,
                                                     const YSpan& curvSpan,
                                                     const LookupTable<Vector2D>* cosSinLookupTable)
  : QuadTreeProcessor(0, 0, { {0, cosSinLookupTable->getNPoints() - 1}, curvSpan})
, m_localOrigin(localOrigin)
, m_cosSinLookupTable(cosSinLookupTable)
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

  const double& driftLength = wireHit->getRefDriftLength();
  const Vector2D& pos2D = wireHit->getRefPos2D() - m_localOrigin;
  double r2 = pos2D.normSquared() - driftLength * driftLength;

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

  // compute Legendre curves at the left and right borders of the node
  float rHitMinRight = rHitMin - driftLength;
  float rHitMaxRight = rHitMax - driftLength;

  float rHitMinLeft = rHitMin + driftLength;
  float rHitMaxLeft = rHitMax + driftLength;

  // Compute distance from the Legendre curves to bottom and top borders of the node
  distRight[0][0] = rMin - rHitMinRight;
  distRight[0][1] = rMin - rHitMaxRight;
  distRight[1][0] = rMax - rHitMinRight;
  distRight[1][1] = rMax - rHitMaxRight;

  distLeft[0][0] = rMin - rHitMinLeft;
  distLeft[0][1] = rMin - rHitMaxLeft;
  distLeft[1][0] = rMax - rHitMinLeft;
  distLeft[1][1] = rMax - rHitMaxLeft;

  // Compare distance signs from Legendre curves to the node
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

void AxialHitQuadTreeProcessor::insertItemsInNodes(const std::vector<QuadTree*>& nodes,
                                                   const std::vector<Item*>& items)
{
  const size_t nNodes = nodes.size();
  if (nNodes == 0 or items.empty()) return;

  // Collect the geometry of the nodes and group them by their theta span.
  // The nodes handed here are either the four children of one node or the nodes of the seed
  // level, which both cover only a handful of distinct theta spans.
  m_thetaSpanCaches.clear();
  m_nodeCaches.resize(nNodes);

  for (size_t iNode = 0; iNode < nNodes; ++iNode) {
    QuadTree* node = nodes[iNode];
    NodeCache& nodeCache = m_nodeCaches[iNode];
    nodeCache.yMin = node->getYMin();
    nodeCache.yMax = node->getYMax();
    nodeCache.needsDerivativeCheck = node->getLevel() <= 4 and m_twoSidedPhaseSpace and
                                     nodeCache.yMin > -c_curlCurv and nodeCache.yMax < c_curlCurv;

    const long xMin = node->getXMin();
    const long xMax = node->getXMax();

    size_t iThetaSpan = 0;
    for (; iThetaSpan < m_thetaSpanCaches.size(); ++iThetaSpan) {
      if (m_thetaSpanCaches[iThetaSpan].xMin == xMin and m_thetaSpanCaches[iThetaSpan].xMax == xMax) break;
    }
    if (iThetaSpan == m_thetaSpanCaches.size()) {
      ThetaSpanCache thetaSpanCache;
      thetaSpanCache.xMin = xMin;
      thetaSpanCache.xMax = xMax;
      thetaSpanCache.thetaVecMin = &m_cosSinLookupTable->at(xMin);
      thetaSpanCache.thetaVecMax = &m_cosSinLookupTable->at(xMax);
      m_thetaSpanCaches.push_back(thetaSpanCache);
    }
    nodeCache.iThetaSpan = iThetaSpan;
  }

  const size_t nThetaSpans = m_thetaSpanCaches.size();

  for (Item* item : items) {
    if (item->isUsed()) continue;

    const CDCWireHit* wireHit = item->getPointer();

    // Quantities that only depend on the hit
    const double driftLength = wireHit->getRefDriftLength();
    const Vector2D pos2D = wireHit->getRefPos2D() - m_localOrigin;
    const double r2 = pos2D.normSquared() - driftLength * driftLength;

    // Quantities that only depend on the hit and the theta span of a node
    for (size_t iThetaSpan = 0; iThetaSpan < nThetaSpans; ++iThetaSpan) {
      ThetaSpanCache& thetaSpanCache = m_thetaSpanCaches[iThetaSpan];
      const Vector2D& thetaVecMin = *thetaSpanCache.thetaVecMin;
      const Vector2D& thetaVecMax = *thetaSpanCache.thetaVecMax;

      const float rHitMin = thetaVecMin.dot(pos2D);
      const float rHitMax = thetaVecMax.dot(pos2D);

      thetaSpanCache.rHitMinRight = rHitMin - driftLength;
      thetaSpanCache.rHitMaxRight = rHitMax - driftLength;
      thetaSpanCache.rHitMinLeft = rHitMin + driftLength;
      thetaSpanCache.rHitMaxLeft = rHitMax + driftLength;

      const float rHitMinExtr = thetaVecMin.cross(pos2D);
      const float rHitMaxExtr = thetaVecMax.cross(pos2D);
      thetaSpanCache.rHitMinExtr = rHitMinExtr;
      thetaSpanCache.rHitMaxExtr = rHitMaxExtr;

      // Same decision as checkDerivative()
      thetaSpanCache.derivativeOk = ((rHitMinExtr > 0) and (rHitMaxExtr * rHitMinExtr >= 0)) or
                                    (rHitMaxExtr * rHitMinExtr < 0);

      thetaSpanCache.hasExtremum = rHitMinExtr * rHitMaxExtr < 0.;
      if (thetaSpanCache.hasExtremum) {
        thetaSpanCache.extremumIsBetween = pos2D.isBetween(thetaVecMin, thetaVecMax);
      }
    }

    // Legendre curves at the extremum - only needed if some theta span contains the extremum
    bool extremumComputed = false;
    float rRight = 0;
    float rLeft = 0;

    for (size_t iNode = 0; iNode < nNodes; ++iNode) {
      const NodeCache& nodeCache = m_nodeCaches[iNode];
      const ThetaSpanCache& thetaSpanCache = m_thetaSpanCaches[nodeCache.iThetaSpan];

      // Check whether the hit lies in the forward direction
      if (nodeCache.needsDerivativeCheck and not thetaSpanCache.derivativeOk) continue;

      // get top and bottom borders of the node
      const float rMin = nodeCache.yMin * r2 / 2;
      const float rMax = nodeCache.yMax * r2 / 2;

      // Compare distance signs from the Legendre curves to the node
      // Check right
      if (not sameSign(rMin - thetaSpanCache.rHitMinRight,
                       rMin - thetaSpanCache.rHitMaxRight,
                       rMax - thetaSpanCache.rHitMinRight,
                       rMax - thetaSpanCache.rHitMaxRight)) {
        nodes[iNode]->insertItem(item);
        continue;
      }

      // Check left
      if (not sameSign(rMin - thetaSpanCache.rHitMinLeft,
                       rMin - thetaSpanCache.rHitMaxLeft,
                       rMax - thetaSpanCache.rHitMinLeft,
                       rMax - thetaSpanCache.rHitMaxLeft)) {
        nodes[iNode]->insertItem(item);
        continue;
      }

      // Check the extremum
      if (not thetaSpanCache.hasExtremum) continue;
      if (not thetaSpanCache.extremumIsBetween) continue;

      if (not extremumComputed) {
        const double r = pos2D.norm();
        rRight = r - driftLength;
        rLeft = r + driftLength;
        extremumComputed = true;
      }

      const bool crossesRight = (rMin - rRight) * (rMax - rRight) < 0;
      const bool crossesLeft = (rMin - rLeft) * (rMax - rLeft) < 0;
      if (crossesRight or crossesLeft) {
        nodes[iNode]->insertItem(item);
      }
    }
  }
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
  const double& driftLength = wireHit->getRefDriftLength();
  const Vector2D& pos2D = wireHit->getRefPos2D() - m_localOrigin;
  double r2 = pos2D.normSquared() - driftLength * driftLength;

  // get left and right borders of the node
  long thetaMin = node->getXMin();
  long thetaMax = node->getXMax();

  const Vector2D& thetaVecMin = m_cosSinLookupTable->at(thetaMin);
  const Vector2D& thetaVecMax = m_cosSinLookupTable->at(thetaMax);

  if (not pos2D.isBetween(thetaVecMin, thetaVecMax)) return false;

  // compute Legendre curves at the position
  double r = pos2D.norm();
  float rRight = r - driftLength;
  float rLeft = r + driftLength;

  // get top and bottom borders of the node
  float rMin = node->getYMin() * r2 / 2;
  float rMax = node->getYMax() * r2 / 2;

  bool crossesRight = (rMin - rRight) * (rMax - rRight) < 0;
  bool crossesLeft = (rMin - rLeft) * (rMax - rLeft) < 0;
  return crossesRight or crossesLeft;
}

void AxialHitQuadTreeProcessor::drawHits(std::vector<const CDCWireHit*> hits, unsigned int color) const
{
  static int nevent(0);

  TCanvas* canv = new TCanvas("canv", "legendre transform", 0, 0, 1200, 600);
  canv->cd(1);
  TGraph* dummyGraph = new TGraph();
  dummyGraph->SetPoint(1, -M_PI, 0);
  dummyGraph->SetPoint(2, M_PI, 0);
  dummyGraph->Draw("AP");
  dummyGraph->GetXaxis()->SetTitle("#theta");
  dummyGraph->GetYaxis()->SetTitle("#rho");
  dummyGraph->GetXaxis()->SetRangeUser(-M_PI, M_PI);
  dummyGraph->GetYaxis()->SetRangeUser(-0.02, 0.15);

  for (const CDCWireHit* wireHit : hits) {
    const double& driftLength = wireHit->getRefDriftLength();
    const Vector2D& pos2D = wireHit->getRefPos2D() - m_localOrigin;
    double x = pos2D.x();
    double y = pos2D.y();
    double r2 = pos2D.normSquared() - driftLength * driftLength;

    TF1* concaveHitLegendre = new TF1("concaveHitLegendre", "2*([0]/[3])*cos(x) + 2*([1]/[3])*sin(x) + 2*([2]/[3])", -M_PI, M_PI);
    TF1* convexHitLegendre = new TF1("convexHitLegendre", "2*([0]/[3])*cos(x) + 2*([1]/[3])*sin(x) - 2*([2]/[3])", -M_PI, M_PI);
    concaveHitLegendre->SetLineWidth(1);
    convexHitLegendre->SetLineWidth(1);
    concaveHitLegendre->SetLineColor(color);
    convexHitLegendre->SetLineColor(color);

    concaveHitLegendre->SetParameters(x, y, driftLength, r2);
    convexHitLegendre->SetParameters(x, y, driftLength, r2);
    concaveHitLegendre->Draw("CSAME");
    convexHitLegendre->Draw("CSAME");
  }
//   canv->Print(Form("legendreHits_%i.root", nevent));
//   canv->Print(Form("legendreHits_%i.eps", nevent));
  canv->Print(Form("legendreHits_%i.png", nevent));
  delete canv;

  nevent++;
}

void AxialHitQuadTreeProcessor::drawNode(QuadTree* node) const
{
  std::vector<const CDCWireHit*> hits;
  for (Item* item : node->getItems()) {
    const CDCWireHit* wireHit = item->getPointer();
    hits.push_back(wireHit);
  }
  drawHits(hits);
}
