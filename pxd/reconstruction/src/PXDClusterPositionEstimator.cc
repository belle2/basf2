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

void Belle2::PXD::PXDClusterPositionEstimator::initialize()
{
  m_shapeIndexFromDB = unique_ptr<Belle2::DBObjPtr<Belle2::PXDClusterShapeIndexPar>>(new
                       Belle2::DBObjPtr<Belle2::PXDClusterShapeIndexPar>());
  m_positionEstimatorFromDB = unique_ptr<Belle2::DBObjPtr<Belle2::PXDClusterPositionEstimatorPar>>
                              (new Belle2::DBObjPtr<Belle2::PXDClusterPositionEstimatorPar>());

  if ((*m_shapeIndexFromDB).isValid() && (*m_positionEstimatorFromDB).isValid()) {
    setShapeIndexFromDB();
    (*m_shapeIndexFromDB).addCallback(this, &Belle2::PXD::PXDClusterPositionEstimator::setShapeIndexFromDB);

    setPositionEstimatorFromDB();
    (*m_positionEstimatorFromDB).addCallback(this, &Belle2::PXD::PXDClusterPositionEstimator::setPositionEstimatorFromDB);
  }
}

void Belle2::PXD::PXDClusterPositionEstimator::setShapeIndexFromDB()
{
  m_shapeIndexPar = **m_shapeIndexFromDB;
}

void Belle2::PXD::PXDClusterPositionEstimator::setPositionEstimatorFromDB()
{
  m_positionEstimatorPar = **m_positionEstimatorFromDB;
}

Belle2::PXD::PXDClusterPositionEstimator& Belle2::PXD::PXDClusterPositionEstimator::getInstance()
{
  static std::unique_ptr<Belle2::PXD::PXDClusterPositionEstimator> instance(new Belle2::PXD::PXDClusterPositionEstimator());
  return *instance;
}


const Belle2::PXDClusterOffsetPar* Belle2::PXD::PXDClusterPositionEstimator::getClusterOffset(const Belle2::PXDCluster& cluster,
    double tu,
    double tv) const
{
  double thetaU = TMath::ATan2(tu, 1.0) * 180.0 / M_PI;
  double thetaV = TMath::ATan2(tv, 1.0) * 180.0 / M_PI;
  int sector_index = getSectorIndex(thetaU, thetaV);

  int clusterkind = cluster.getKind();
  int shape_index = cluster.getSectorShapeIndices().at(sector_index);
  float eta = cluster.getSectorEtaValues().at(sector_index);
  return m_positionEstimatorPar.getOffset(shape_index, eta, thetaU, thetaV, clusterkind);
}


float Belle2::PXD::PXDClusterPositionEstimator::getShapeLikelyhood(const Belle2::PXDCluster& cluster, double tu, double tv) const
{
  double thetaU = TMath::ATan2(tu, 1.0) * 180.0 / M_PI;
  double thetaV = TMath::ATan2(tv, 1.0) * 180.0 / M_PI;
  int clusterkind = cluster.getKind();
  int sector_index = getSectorIndex(thetaU, thetaV);
  int shape_index = cluster.getSectorShapeIndices().at(sector_index);
  return m_positionEstimatorPar.getShapeLikelyhood(shape_index, thetaU, thetaV, clusterkind);
}


float Belle2::PXD::PXDClusterPositionEstimator::computeEta(const std::set<Belle2::PXD::Pixel>& pixels, int vStart, int vSize,
                                                           double thetaU, double thetaV) const
{
  const Belle2::PXD::Pixel& headPixel = getHeadPixel(pixels, vStart, vSize, thetaU, thetaV);
  const Belle2::PXD::Pixel& tailPixel = getTailPixel(pixels, vStart, vSize, thetaU, thetaV);
  float eta = 0;
  if (headPixel.getIndex() != tailPixel.getIndex()) {
    eta = tailPixel.getCharge();
    eta /= (tailPixel.getCharge() + headPixel.getCharge());
  } else {
    eta = tailPixel.getCharge();
  }
  return eta;
}

const Belle2::PXD::Pixel& Belle2::PXD::PXDClusterPositionEstimator::getHeadPixel(const std::set<Belle2::PXD::Pixel>& pixels,
    int vStart, int vSize, double thetaU,
    double thetaV) const
{
  if (thetaV >= 0) {
    if (thetaU >= 0) {
      return getLastPixelWithVOffset(pixels, vStart, vSize - 1);
    } else {
      return getFirstPixelWithVOffset(pixels, vStart, vSize - 1);
    }
  } else {
    if (thetaU >= 0) {
      return getLastPixelWithVOffset(pixels, vStart, 0);
    } else {
      return getFirstPixelWithVOffset(pixels, vStart, 0);
    }
  }
}

const Belle2::PXD::Pixel& Belle2::PXD::PXDClusterPositionEstimator::getTailPixel(const std::set<Belle2::PXD::Pixel>& pixels,
    int vStart, int vSize, double thetaU,
    double thetaV) const
{
  if (thetaV >= 0) {
    if (thetaU >= 0) {
      return getFirstPixelWithVOffset(pixels, vStart, 0);
    } else {
      return getLastPixelWithVOffset(pixels, vStart, 0);
    }
  } else {
    if (thetaU >= 0) {
      return getFirstPixelWithVOffset(pixels, vStart, vSize - 1);
    } else {
      return getLastPixelWithVOffset(pixels, vStart, vSize - 1);
    }
  }
}

const Belle2::PXD::Pixel& Belle2::PXD::PXDClusterPositionEstimator::getLastPixelWithVOffset(const std::set<Belle2::PXD::Pixel>&
    pixels,
    int vStart, int vOffset) const
{
  for (auto pxit = pixels.cbegin(); pxit != pixels.cend(); ++pxit) {
    int v = pxit->getV() - vStart;
    if (vOffset < v) {
      if (pxit == pixels.cbegin()) {
      } else {
        pxit--;
        return *pxit;
      }
    }
  }
  if (pixels.empty())
    B2FATAL("Found cluster with empty pixel set. ");

  auto pxit = --pixels.cend();
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
  const Belle2::PXD::Pixel& headPixel = getHeadPixel(pixels, vStart, vSize, thetaU, thetaV);
  const Belle2::PXD::Pixel& tailPixel = getTailPixel(pixels, vStart, vSize, thetaU, thetaV);
  std::string name = "S";

  name += "D" + std::to_string(tailPixel.getV() - vStart) + '.' + std::to_string(tailPixel.getU() - uStart);

  if (headPixel.getIndex() != tailPixel.getIndex()) {
    name += "D" + std::to_string(headPixel.getV() - vStart) + '.' + std::to_string(headPixel.getU() - uStart);
  }
  return name;
}

int Belle2::PXD::PXDClusterPositionEstimator::computeShapeIndex(const std::set<Belle2::PXD::Pixel>& pixels, int uStart, int vStart,
    int vSize, double thetaU,
    double thetaV) const
{
  // Compute shape name
  auto shape_name = getShortName(pixels, uStart, vStart, vSize, thetaU, thetaV);
  // Return shape index
  return m_shapeIndexPar.getShapeIndex(shape_name);
}


const std::string Belle2::PXD::PXDClusterPositionEstimator::getMirroredShortName(const std::set<Belle2::PXD::Pixel>& pixels,
    int uStart,
    int vStart, int vSize, double thetaU,
    double thetaV) const
{
  const Belle2::PXD::Pixel& headPixel = getHeadPixel(pixels, vStart, vSize, thetaU, thetaV);
  const Belle2::PXD::Pixel& tailPixel = getTailPixel(pixels, vStart, vSize, thetaU, thetaV);
  int vmax = vSize - 1;

  std::string name = "S";
  name += "D" + std::to_string(vmax - tailPixel.getV() + vStart) + '.' + std::to_string(tailPixel.getU() - uStart);

  if (headPixel.getIndex() != tailPixel.getIndex()) {
    name += "D" + std::to_string(vmax - headPixel.getV() + vStart) + '.' + std::to_string(headPixel.getU() - uStart);
  }
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
  const Belle2::PXD::SensorInfo& Info = dynamic_cast<const Belle2::PXD::SensorInfo&>(Belle2::VXD::GeoCache::get(sensorID));

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

int Belle2::PXD::PXDClusterPositionEstimator::getClusterkind(const std::vector<Belle2::PXD::Pixel>& pixels,
    const Belle2::VxdID& sensorID) const
{
  std::set<int> pixelkinds;
  bool uEdge = false;
  bool vEdge = false;

  const Belle2::PXD::SensorInfo& Info = dynamic_cast<const Belle2::PXD::SensorInfo&>(Belle2::VXD::GeoCache::get(sensorID));

  for (const Belle2::PXD::Pixel& pix : pixels) {
    int pixelkind = Info.getPixelKindNew(sensorID, pix.getV());
    pixelkinds.insert(pixelkind);

    // Cluster at v sensor edge
    if (pix.getV() <= 0 or pix.getV() >= 767)
      vEdge = true;
    // Cluster at u sensor edge
    if (pix.getU() <= 0 or pix.getU() >= 249)
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

int Belle2::PXD::PXDClusterPositionEstimator::getSectorIndex(double thetaU, double thetaV) const
{
  int sectorIndex = 0;
  if (thetaU >= 0) {
    if (thetaV >= 0) {
      sectorIndex = 0;
    } else {
      sectorIndex = 3;
    }
  } else {
    if (thetaV >= 0) {
      sectorIndex = 1;
    } else {
      sectorIndex = 2;
    }
  }
  return sectorIndex;
}



//  }
//}
