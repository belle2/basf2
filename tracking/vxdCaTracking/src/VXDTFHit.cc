/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/VXDTFHit.h"
#include "../include/FullSecID.h"
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Tracking;


/** operator overload **/
bool VXDTFHit::operator==(const VXDTFHit& b) const
{
  B2FATAL("somebody is using the '=='-operator of VXDTFHit, although it does no valid comparison!");
  if (getVxdID() != b.getVxdID()) { return false; }   /// ensures that hits are from the same sensor

  if (this->getDetectorType() == Const::PXD) {   // PXD
    return getClusterIndexUV() == b.getClusterIndexUV();
  } else if (this->getDetectorType() == Const::SVD) {   // SVD
    return (getClusterIndexU() == b.getClusterIndexU() or getClusterIndexV() == b.getClusterIndexV());  // returns True if one of the cases are True
  }
  return getTimeStamp() == b.getTimeStamp();
}

bool VXDTFHit::operator<(const VXDTFHit& b) const
{
  B2FATAL("somebody is using the '=='-operator of VXDTFHit, although it does no valid comparison!");
  return getTimeStamp() < b.getTimeStamp();
}

bool VXDTFHit::operator>(const VXDTFHit& b) const
{
  B2FATAL("somebody is using the '=='-operator of VXDTFHit, although it does no valid comparison!");
  return getTimeStamp() > b.getTimeStamp();
}


std::string VXDTFHit::getSectorString() { return FullSecID(m_papaSector).getFullSecString(); }