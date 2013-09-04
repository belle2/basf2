/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef ROIID_H
#define ROIID_H

#include <framework/datastore/RelationsObject.h>
#include <pxd/geometry/SensorInfo.h>
#include <pxd/dataobjects/PXDDigit.h>

namespace Belle2 {
  /** ROIid stores the U and V ids and the sensor id of the Region Of Interest.
   *
   *
   */

  class ROIid : public RelationsObject {

  public:

    /** default constructor.
     *
     */
    ROIid();


    /**
     * destructor.
     */
    virtual ~ROIid();


    //some accessors
    int getMinUid() const { return m_minUid; }
    int getMaxUid() const { return m_maxUid; }
    int getMinVid() const { return m_minVid; }
    int getMaxVid() const { return m_maxVid; }
    VxdID getSensorID() const { return m_sensorID; }

    void setMinUid(double user_minUid) { m_minUid = user_minUid; }
    void setMaxUid(double user_maxUid) { m_maxUid = user_maxUid; }
    void setMinVid(double user_minVid) { m_minVid = user_minVid; }
    void setMaxVid(double user_maxVid) { m_maxVid = user_maxVid; }
    void setSensorID(VxdID user_sensorID) { m_sensorID = user_sensorID;}

    bool Contains(const Belle2::PXDDigit& thePXDDigit) const;

  private:

    int m_minUid; /**< u ID of the bottom left pixel of the ROI */
    int m_maxUid; /**< u ID of the top right pixel of the ROI */
    int m_minVid; /**< v ID of the bottom left pixel of the ROI */
    int m_maxVid; /**< v ID of the top right pixel of the ROI */
    VxdID::baseType m_sensorID; /**< sensor ID */

    ClassDef(ROIid, 1)
  };
}

#endif  // ROIID_H  
