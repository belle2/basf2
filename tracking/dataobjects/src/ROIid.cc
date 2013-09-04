/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dataobjects/ROIid.h>


using namespace Belle2;

ROIid::ROIid()
{

}

ROIid::~ROIid()
{

}


bool
ROIid::Contains(const PXDDigit& thePXDDigit) const
{
  return (m_minUid <= thePXDDigit.getUCellID() &&
          m_maxUid >= thePXDDigit.getUCellID() &&
          m_minVid <= thePXDDigit.getVCellID() &&
          m_maxVid >= thePXDDigit.getVCellID() &&
          m_sensorID == thePXDDigit.getSensorID()
         );

}

ClassImp(ROIid);
