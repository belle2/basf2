/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <vtx/reconstruction/VTXClusterPositionEstimator.h>
#include <vxd/dataobjects/VxdID.h>
#include <vtx/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <cmath>
#include <numeric>

using namespace std;



void Belle2::VTX::VTXClusterPositionEstimator::initialize()
{
  m_shapeIndexFromDB = unique_ptr<Belle2::DBObjPtr<Belle2::VTXClusterShapeIndexPar>>(new
                       Belle2::DBObjPtr<Belle2::VTXClusterShapeIndexPar>());
  m_positionEstimatorFromDB = unique_ptr<Belle2::DBObjPtr<Belle2::VTXClusterPositionEstimatorPar>>
                              (new Belle2::DBObjPtr<Belle2::VTXClusterPositionEstimatorPar>());

  if ((*m_shapeIndexFromDB).isValid() && (*m_positionEstimatorFromDB).isValid()) {
    setShapeIndexFromDB();
    (*m_shapeIndexFromDB).addCallback(this, &Belle2::VTX::VTXClusterPositionEstimator::setShapeIndexFromDB);

    setPositionEstimatorFromDB();
    (*m_positionEstimatorFromDB).addCallback(this, &Belle2::VTX::VTXClusterPositionEstimator::setPositionEstimatorFromDB);
  }
}

void Belle2::VTX::VTXClusterPositionEstimator::setShapeIndexFromDB()
{
  m_shapeIndexPar = **m_shapeIndexFromDB;
}

void Belle2::VTX::VTXClusterPositionEstimator::setPositionEstimatorFromDB()
{
  m_positionEstimatorPar = **m_positionEstimatorFromDB;
}

Belle2::VTX::VTXClusterPositionEstimator& Belle2::VTX::VTXClusterPositionEstimator::getInstance()
{
  static std::unique_ptr<Belle2::VTX::VTXClusterPositionEstimator> instance(new Belle2::VTX::VTXClusterPositionEstimator());
  return *instance;
}


const Belle2::VTXClusterOffsetPar* Belle2::VTX::VTXClusterPositionEstimator::getClusterOffset(const Belle2::VTXCluster& cluster,
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


float Belle2::VTX::VTXClusterPositionEstimator::getShapeLikelyhood(const Belle2::VTXCluster& cluster, double tu, double tv) const
{
  double thetaU = TMath::ATan2(tu, 1.0) * 180.0 / M_PI;
  double thetaV = TMath::ATan2(tv, 1.0) * 180.0 / M_PI;
  int clusterkind = cluster.getKind();
  int sector_index = getSectorIndex(thetaU, thetaV);
  int shape_index = cluster.getSectorShapeIndices().at(sector_index);
  return m_positionEstimatorPar.getShapeLikelyhood(shape_index, thetaU, thetaV, clusterkind);
}


float Belle2::VTX::VTXClusterPositionEstimator::computeEta(const std::set<Belle2::VTX::Pixel>& pixels, int vStart, int vSize,
                                                           double thetaU, double thetaV) const
{
  const Belle2::VTX::Pixel& headPixel = getHeadPixel(pixels, vStart, vSize, thetaU, thetaV);
  const Belle2::VTX::Pixel& tailPixel = getTailPixel(pixels, vStart, vSize, thetaU, thetaV);
  float eta = 0;
  if (headPixel.getIndex() != tailPixel.getIndex()) {
    eta = tailPixel.getCharge();
    eta /= (tailPixel.getCharge() + headPixel.getCharge());
  } else {
    eta = tailPixel.getCharge();
  }
  return eta;
}

const Belle2::VTX::Pixel& Belle2::VTX::VTXClusterPositionEstimator::getHeadPixel(const std::set<Belle2::VTX::Pixel>& pixels,
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

const Belle2::VTX::Pixel& Belle2::VTX::VTXClusterPositionEstimator::getTailPixel(const std::set<Belle2::VTX::Pixel>& pixels,
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

const Belle2::VTX::Pixel& Belle2::VTX::VTXClusterPositionEstimator::getLastPixelWithVOffset(const std::set<Belle2::VTX::Pixel>&
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

const Belle2::VTX::Pixel& Belle2::VTX::VTXClusterPositionEstimator::getFirstPixelWithVOffset(
  const std::set<Belle2::VTX::Pixel>& pixels,
  int vStart, int vOffset) const
{
  for (const Belle2::VTX::Pixel& px : pixels) {
    int v = px.getV() - vStart;
    if (vOffset == v) {
      return px;
    }
  }
  if (pixels.empty())
    B2FATAL("Found cluster with empty pixel set. ");

  return *pixels.cbegin();
}

const std::string Belle2::VTX::VTXClusterPositionEstimator::getShortName(const std::set<Belle2::VTX::Pixel>& pixels, int uStart,
    int vStart, int vSize, double thetaU,
    double thetaV) const
{
  const Belle2::VTX::Pixel& headPixel = getHeadPixel(pixels, vStart, vSize, thetaU, thetaV);
  const Belle2::VTX::Pixel& tailPixel = getTailPixel(pixels, vStart, vSize, thetaU, thetaV);
  std::string name = "S";

  name += "D" + std::to_string(tailPixel.getV() - vStart) + '.' + std::to_string(tailPixel.getU() - uStart);

  if (headPixel.getIndex() != tailPixel.getIndex()) {
    name += "D" + std::to_string(headPixel.getV() - vStart) + '.' + std::to_string(headPixel.getU() - uStart);
  }
  return name;
}

int Belle2::VTX::VTXClusterPositionEstimator::computeShapeIndex(const std::set<Belle2::VTX::Pixel>& pixels, int uStart, int vStart,
    int vSize, double thetaU,
    double thetaV) const
{
  // Compute shape name
  auto shape_name = getShortName(pixels, uStart, vStart, vSize, thetaU, thetaV);
  // Return shape index
  return m_shapeIndexPar.getShapeIndex(shape_name);
}


const std::string Belle2::VTX::VTXClusterPositionEstimator::getMirroredShortName(const std::set<Belle2::VTX::Pixel>& pixels,
    int uStart,
    int vStart, int vSize, double thetaU,
    double thetaV) const
{
  const Belle2::VTX::Pixel& headPixel = getHeadPixel(pixels, vStart, vSize, thetaU, thetaV);
  const Belle2::VTX::Pixel& tailPixel = getTailPixel(pixels, vStart, vSize, thetaU, thetaV);
  int vmax = vSize - 1;

  std::string name = "S";
  name += "D" + std::to_string(vmax - tailPixel.getV() + vStart) + '.' + std::to_string(tailPixel.getU() - uStart);

  if (headPixel.getIndex() != tailPixel.getIndex()) {
    name += "D" + std::to_string(vmax - headPixel.getV() + vStart) + '.' + std::to_string(headPixel.getU() - uStart);
  }
  return name;
}

const std::string Belle2::VTX::VTXClusterPositionEstimator::getFullName(const std::set<Belle2::VTX::Pixel>& pixels, int uStart,
    int vStart) const
{
  return std::accumulate(pixels.begin(), pixels.end(), std::string("F"),
  [uStart, vStart](auto name, auto px) {
    return name + "D" + std::to_string(px.getV() - vStart) + "." + std::to_string(px.getU() - uStart);
  });
}

int Belle2::VTX::VTXClusterPositionEstimator::getClusterkind(const Belle2::VTXCluster& cluster) const
{
  Belle2::VxdID sensorID = cluster.getSensorID();
  const Belle2::VTX::SensorInfo& Info = dynamic_cast<const Belle2::VTX::SensorInfo&>(Belle2::VXD::GeoCache::get(sensorID));

  std::set<int> pixelkinds;
  for (const Belle2::VTXDigit& digit : cluster.getRelationsTo<Belle2::VTXDigit>("VTXDigits")) {
    int pixelkind = Info.getPixelKindNew(sensorID, digit.getVCellID());
    pixelkinds.insert(pixelkind);
  }

  // In most cases, clusterkind is just pixelkind of first digit
  int clusterkind = *pixelkinds.begin();

  // Clusters with different pixelkinds are special
  // TODO: At the moment, clusterkind >3 will not be corrected
  if (pixelkinds.size() >  1)
    clusterkind = 4;

  return clusterkind;
}

int Belle2::VTX::VTXClusterPositionEstimator::getClusterkind(const std::vector<Belle2::VTX::Pixel>& pixels,
    const Belle2::VxdID& sensorID) const
{
  const Belle2::VTX::SensorInfo& Info = dynamic_cast<const Belle2::VTX::SensorInfo&>(Belle2::VXD::GeoCache::get(sensorID));

  std::set<int> pixelkinds;
  for (const Belle2::VTX::Pixel& pix : pixels) {
    int pixelkind = Info.getPixelKindNew(sensorID, pix.getV());
    pixelkinds.insert(pixelkind);
  }

  // In most cases, clusterkind is just pixelkind of first digit
  int clusterkind = *pixelkinds.begin();

  // Clusters with different pixelkinds  are special
  // TODO: At the moment, clusterkind >3 will not be corrected
  if (pixelkinds.size() > 1)
    clusterkind = 4;

  return clusterkind;
}

int Belle2::VTX::VTXClusterPositionEstimator::getSectorIndex(double thetaU, double thetaV) const
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




