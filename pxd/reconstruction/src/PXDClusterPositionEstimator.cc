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
#include <set>


using namespace std;

//namespace Belle2 {
//  namespace PXD {

Belle2::PXD::PXDClusterPositionEstimator& Belle2::PXD::PXDClusterPositionEstimator::getInstance()
{
  static std::unique_ptr<PXDClusterPositionEstimator> instance(new Belle2::PXD::PXDClusterPositionEstimator());
  return *instance;
}

Belle2::PXDCluster& Belle2::PXD::PXDClusterPositionEstimator::correctCluster(Belle2::PXDCluster& cluster, double tu,
    double tv) const
{
  // No correction if no data
  if (!m_isInitialized) return cluster;

  double thetaU = TMath::ATan2(tu, 1.0) * 180.0 / M_PI;
  double thetaV = TMath::ATan2(tv, 1.0) * 180.0 / M_PI;
  int clusterkind = getClusterkind(cluster);
  float eta = computeEta(cluster, thetaU, thetaV);
  auto shape_name = getShortName(cluster, thetaU, thetaV);
  int shape_index = m_shapeIndexPar.getShapeIndex(shape_name);

  if (m_positionEstimatorPar.hasOffset(shape_index, eta, thetaU, thetaV, clusterkind)) {
    auto offset = m_positionEstimatorPar.getOffset(shape_index, eta, thetaU, thetaV, clusterkind);

    const Belle2::VxdID& sensorID = cluster.getSensorID();
    const Belle2::PXD::SensorInfo& Info = dynamic_cast<const Belle2::PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    double posU = Info.getUCellPosition(cluster.getUStart());
    double posV = Info.getVCellPosition(cluster.getVStart());

    cluster.setU(posU + offset.getU());
    cluster.setV(posV + offset.getV());
    cluster.setUSigma(std::sqrt(offset.getUSigma2()));
    cluster.setVSigma(std::sqrt(offset.getVSigma2()));
    cluster.setRho(offset.getUVCovariance() / std::sqrt(offset.getUSigma2()) / std::sqrt(offset.getVSigma2()));
  }
  return cluster;
}

float Belle2::PXD::PXDClusterPositionEstimator::getShapeLikelyhood(Belle2::PXDCluster& cluster, double tu, double tv) const
{
  double thetaU = TMath::ATan2(tu, 1.0) * 180.0 / M_PI;
  double thetaV = TMath::ATan2(tv, 1.0) * 180.0 / M_PI;
  auto shape_name = getShortName(cluster, thetaU, thetaV);
  int shape_index = m_shapeIndexPar.getShapeIndex(shape_name);
  int clusterkind = getClusterkind(cluster);
  return m_positionEstimatorPar.getShapeLikelyhood(shape_index, thetaU, thetaV, clusterkind);
}

float Belle2::PXD::PXDClusterPositionEstimator::computeEta(const Belle2::PXDCluster& cluster, double thetaU, double thetaV) const
{
  auto headDigit = getHeadDigit(cluster, thetaU, thetaV);
  auto tailDigit = getTailDigit(cluster, thetaU, thetaV);
  float eta = 0;
  if (headDigit.getVCellID() != tailDigit.getVCellID() or headDigit.getUCellID() != tailDigit.getVCellID())
    eta = (float)tailDigit.getCharge() / ((float)tailDigit.getCharge() + (float)headDigit.getCharge());
  else
    eta = (float) tailDigit.getCharge();
  return eta;
}

const Belle2::PXDDigit& Belle2::PXD::PXDClusterPositionEstimator::getHeadDigit(const Belle2::PXDCluster& cluster, float thetaU,
    float thetaV) const
{
  auto vMax = cluster.getVSize() - 1;

  if (thetaV >= 0)
    if (thetaU >= 0)
      return getLastDigitWithVOffset(cluster, vMax);     //size - 1;
    else
      return getFirstDigitWithVOffset(cluster, vMax); //    get_indices_at_v(cluster, vmax)[0];
  else if (thetaU >= 0)
    return getLastDigitWithVOffset(cluster, 0);    //get_indices_at_v(cluster, 0)[-1];
  else
    return getFirstDigitWithVOffset(cluster, 0);   // get_indices_at_v(cluster, 0)[0];
}

const Belle2::PXDDigit& Belle2::PXD::PXDClusterPositionEstimator::getTailDigit(const Belle2::PXDCluster& cluster, float thetaU,
    float thetaV) const
{
  auto vMax = cluster.getVSize() - 1;

  if (thetaV >= 0)
    if (thetaU >= 0)
      return getFirstDigitWithVOffset(cluster, 0); //0;
    else
      return getLastDigitWithVOffset(cluster, 0); //get_indices_at_v(cluster, 0)[-1];
  else if (thetaU >= 0)
    return getFirstDigitWithVOffset(cluster, vMax);  //get_indices_at_v(cluster, vmax)[0];
  else
    return getLastDigitWithVOffset(cluster, vMax); //get_indices_at_v(cluster, vmax)[-1];

}

const Belle2::PXDDigit& Belle2::PXD::PXDClusterPositionEstimator::getLastDigitWithVOffset(const Belle2::PXDCluster& cluster,
    int vOffset) const
{
  B2INFO("inside getLastDigit");
  auto vMax = cluster.getVSize() - 1;
  auto vMin = cluster.getVStart();

  if (vOffset > vMax)
    B2FATAL("Request digit from cluster with invalid offset.");

  B2INFO("vOffset " << vOffset << " for cluster size " << cluster.getSize() << "  and charge " << cluster.getCharge());
  int index = 0;
  for (auto digit : cluster.getRelationsTo<Belle2::PXDDigit>()) {
    int v = digit.getVCellID() - vMin;
    B2INFO("index " << index << " current v " << v);
    if (vOffset < v)
      B2INFO("match at " << index - 1);
    return *cluster.getRelationsTo<Belle2::PXDDigit>()[index - 1];
    index++;
  }
  B2INFO("match at last " << index);
  return *cluster.getRelationsTo<PXDDigit>()[index];
}

const Belle2::PXDDigit& Belle2::PXD::PXDClusterPositionEstimator::getFirstDigitWithVOffset(const Belle2::PXDCluster& cluster,
    int vOffset) const
{
  auto vMax = cluster.getVSize() - 1;
  auto vMin = cluster.getVStart();

  if (vOffset > vMax)
    B2FATAL("Request digit from cluster with invalid offset.");

  for (const Belle2::PXDDigit& digit : cluster.getRelationsTo<PXDDigit>()) {
    int v = digit.getVCellID() - vMin;
    if (vOffset == v) {
      return digit;
    }
  }
  // There is no reason to ever get here
  return *cluster.getRelationsTo<PXDDigit>().begin();
}

const std::string Belle2::PXD::PXDClusterPositionEstimator::getShortName(const Belle2::PXDCluster& cluster, float thetaU,
    float thetaV) const
{
  auto headDigit = getHeadDigit(cluster, thetaU, thetaV);
  auto tailDigit = getTailDigit(cluster, thetaU, thetaV);
  auto vMin = cluster.getVStart();
  auto uMin = cluster.getUStart();
  std::string name = "S";

  name += "D" + std::to_string(tailDigit.getVCellID() - vMin) + '.' + std::to_string(tailDigit.getUCellID() - uMin);

  if (headDigit.getVCellID() != tailDigit.getVCellID() or headDigit.getUCellID() != tailDigit.getVCellID())
    name += "D" + std::to_string(headDigit.getVCellID() - vMin) + '.' + std::to_string(headDigit.getUCellID() - uMin);

  return name;
}

const std::string Belle2::PXD::PXDClusterPositionEstimator::getFullName(const Belle2::PXDCluster& cluster) const
{
  auto vMin = cluster.getVStart();
  auto uMin = cluster.getUStart();
  std::string name = "F";

  for (const Belle2::PXDDigit& digit : cluster.getRelationsTo<PXDDigit>()) {
    name += "D" + std::to_string(digit.getVCellID() - vMin) + '.' + std::to_string(digit.getUCellID() - uMin);
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

  for (const Belle2::PXDDigit& digit : cluster.getRelationsTo<PXDDigit>()) {
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
    clusterkind += 4;

  return clusterkind;
}

//  }
//}
