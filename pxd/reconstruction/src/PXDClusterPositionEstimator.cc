/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <pxd/reconstruction/PXDClusterPositionEstimator.h>
#include <vxd/dataobjects/VxdID.h>
#include <pxd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <cmath>



using namespace std;

//namespace Belle2 {
//  namespace PXD {

Belle2::PXD::PXDClusterPositionEstimator& Belle2::PXD::PXDClusterPositionEstimator::getInstance()
{
  static std::unique_ptr<PXDClusterPositionEstimator> instance(new Belle2::PXD::PXDClusterPositionEstimator());
  return *instance;
}


Belle2::PXDClusterOffsetPar Belle2::PXD::PXDClusterPositionEstimator::getClusterOffset(const Belle2::PXDCluster& cluster, double tu,
    double tv) const
{
  // No correction if no data
  if (!m_isInitialized) return Belle2::PXDClusterOffsetPar();

  double thetaU = TMath::ATan2(tu, 1.0) * 180.0 / M_PI;
  double thetaV = TMath::ATan2(tv, 1.0) * 180.0 / M_PI;
  int clusterkind = getClusterkind(cluster);
  int uStart = cluster.getUStart();
  int vStart = cluster.getVStart();
  int vSize = cluster.getVSize();

  std::set<Belle2::PXD::Pixel> pixels;
  for (int i = 0; i < cluster.getSize(); i++) {
    const Belle2::PXDDigit* const storeDigit = cluster.getRelationsTo<Belle2::PXDDigit>("PXDDigits")[i];
    pixels.insert(Pixel(storeDigit, i));
  }

  float eta = computeEta(pixels, vStart, vSize, thetaU, thetaV);
  auto shape_name = getShortName(pixels, uStart, vStart, vSize, thetaU, thetaV);
  int shape_index = m_shapeIndexPar.getShapeIndex(shape_name);

  if (m_positionEstimatorPar.hasOffset(shape_index, eta, thetaU, thetaV, clusterkind)) {
    return m_positionEstimatorPar.getOffset(shape_index, eta, thetaU, thetaV, clusterkind);
  }
  return Belle2::PXDClusterOffsetPar();
}

float Belle2::PXD::PXDClusterPositionEstimator::getShapeLikelyhood(const Belle2::PXDCluster& cluster, double tu, double tv) const
{
  double thetaU = TMath::ATan2(tu, 1.0) * 180.0 / M_PI;
  double thetaV = TMath::ATan2(tv, 1.0) * 180.0 / M_PI;
  int clusterkind = getClusterkind(cluster);
  int uStart = cluster.getUStart();
  int vStart = cluster.getVStart();
  int vSize = cluster.getVSize();

  std::set<Belle2::PXD::Pixel> pixels;
  for (int i = 0; i < cluster.getSize(); i++) {
    const Belle2::PXDDigit* const storeDigit = cluster.getRelationsTo<Belle2::PXDDigit>("PXDDigits")[i];
    pixels.insert(Pixel(storeDigit, i));
  }

  auto shape_name = getShortName(pixels, uStart, vStart, vSize, thetaU, thetaV);
  int shape_index = m_shapeIndexPar.getShapeIndex(shape_name);
  return m_positionEstimatorPar.getShapeLikelyhood(shape_index, thetaU, thetaV, clusterkind);
}

float Belle2::PXD::PXDClusterPositionEstimator::computeEta(const std::set<Belle2::PXD::Pixel>& pixels, int vStart, int vSize,
                                                           double thetaU, double thetaV) const
{
  auto headPixel = getHeadPixel(pixels, vStart, vSize, thetaU, thetaV);
  auto tailPixel = getTailPixel(pixels, vStart, vSize, thetaU, thetaV);
  float eta = 0;
  if (headPixel.getV() != tailPixel.getV() or headPixel.getU() != tailPixel.getU())
    eta = (double)tailPixel.getCharge() / ((double)tailPixel.getCharge() + (double)headPixel.getCharge());
  else
    eta = tailPixel.getCharge();
  return eta;
}

const Belle2::PXD::Pixel& Belle2::PXD::PXDClusterPositionEstimator::getHeadPixel(const std::set<Belle2::PXD::Pixel>& pixels,
    int vStart, int vSize, double thetaU,
    double thetaV) const
{
  if (thetaV >= 0) {
    if (thetaU >= 0)
      return getLastPixelWithVOffset(pixels, vStart, vSize - 1);   //size - 1;
    else
      return getFirstPixelWithVOffset(pixels, vStart, vSize - 1); //    get_indices_at_v(cluster, vmax)[0];
  } else {
    if (thetaU >= 0)
      return getLastPixelWithVOffset(pixels, vStart, 0);    //get_indices_at_v(cluster, 0)[-1];
    else
      return getFirstPixelWithVOffset(pixels, vStart, 0);   // get_indices_at_v(cluster, 0)[0];
  }
}

const Belle2::PXD::Pixel& Belle2::PXD::PXDClusterPositionEstimator::getTailPixel(const std::set<Belle2::PXD::Pixel>& pixels,
    int vStart, int vSize, double thetaU,
    double thetaV) const
{
  if (thetaV >= 0) {
    if (thetaU >= 0)
      return getFirstPixelWithVOffset(pixels, vStart, 0); //0;
    else
      return getLastPixelWithVOffset(pixels, vStart, 0); //get_indices_at_v(cluster, 0)[-1];
  } else {
    if (thetaU >= 0)
      return getFirstPixelWithVOffset(pixels, vStart, vSize - 1); //get_indices_at_v(cluster, vmax)[0];
    else
      return getLastPixelWithVOffset(pixels, vStart, vSize - 1); //get_indices_at_v(cluster, vmax)[-1];
  }
}

const Belle2::PXD::Pixel& Belle2::PXD::PXDClusterPositionEstimator::getLastPixelWithVOffset(const std::set<Belle2::PXD::Pixel>&
    pixels,
    int vStart, int vOffset) const
{
  for (auto pxit = pixels.cbegin(); pxit != pixels.cend(); pxit++) {
    int v = (*pxit).getV() - vStart;
    if (vOffset < v) {
      if (pxit == pixels.cbegin()) {
        B2FATAL("Accesing invalid pixel.");
      } else {
        pxit--;
        return *pxit;
      }
    }
  }
  if (pixels.empty())
    B2FATAL("Found cluster with empty pixel set. ");

  auto pxit = pixels.cend()--;
  return *pxit;
}

const Belle2::PXD::Pixel& Belle2::PXD::PXDClusterPositionEstimator::getFirstPixelWithVOffset(
  const std::set<Belle2::PXD::Pixel>& pixels,
  int vStart, int vOffset) const
{
  for (const Belle2::PXD::Pixel& px : pixels) {
    int v = px.getV() - vStart;
    if (vOffset == v) {
      return px;
    }
  }
  if (pixels.empty())
    B2FATAL("Found cluster with empty pixel set. ");
  return *pixels.cbegin();
}

const std::string Belle2::PXD::PXDClusterPositionEstimator::getShortName(const std::set<Belle2::PXD::Pixel>& pixels, int uStart,
    int vStart, int vSize, double thetaU,
    double thetaV) const
{
  auto headPixel = getHeadPixel(pixels, vStart, vSize, thetaU, thetaV);
  auto tailPixel = getTailPixel(pixels, vStart, vSize, thetaU, thetaV);
  std::string name = "S";

  name += "D" + std::to_string(tailPixel.getV() - vStart) + '.' + std::to_string(tailPixel.getU() - uStart);

  if (headPixel.getV() != tailPixel.getV() or headPixel.getU() != tailPixel.getU())
    name += "D" + std::to_string(headPixel.getV() - vStart) + '.' + std::to_string(headPixel.getU() - uStart);
  return name;
}

const std::string Belle2::PXD::PXDClusterPositionEstimator::getFullName(const std::set<Belle2::PXD::Pixel>& pixels, int uStart,
    int vStart) const
{
  std::string name = "F";
  for (const Belle2::PXD::Pixel& px : pixels) {
    name += "D" + std::to_string(px.getV() - vStart) + '.' + std::to_string(px.getU() - uStart);
  }
  return name;
}

int Belle2::PXD::PXDClusterPositionEstimator::getClusterkind(const Belle2::PXDCluster& cluster) const
{
  std::set<int> pixelkinds;
  bool uEdge = false;
  bool vEdge = false;

  Belle2::VxdID sensorID = cluster.getSensorID();
  const Belle2::PXD::SensorInfo& Info = dynamic_cast<const Belle2::PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));

  for (const Belle2::PXDDigit& digit : cluster.getRelationsTo<Belle2::PXDDigit>("PXDDigits")) {
    int pixelkind = Info.getPixelKindNew(sensorID, digit.getVCellID());
    pixelkinds.insert(pixelkind);

    // Cluster at v sensor edge
    if (digit.getVCellID() <= 0 or digit.getVCellID() >= 767)
      vEdge = true;
    // Cluster at u sensor edge
    if (digit.getUCellID() <= 0 or digit.getUCellID() >= 249)
      uEdge = true;
  }

  // In most cases, clusterkind is just pixelkind of first digit
  int clusterkind = *pixelkinds.begin();

  // Clusters with different pixelkinds or edge digits are special
  // TODO: At the moment, clusterkind >3 will not be corrected
  if (pixelkinds.size() >  1 || uEdge || vEdge)
    clusterkind = 4;

  return clusterkind;
}

//  }
//}
