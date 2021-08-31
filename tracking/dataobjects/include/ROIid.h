/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <vxd/dataobjects/VxdID.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDRawHit.h>
#include <svd/dataobjects/SVDShaperDigit.h>

namespace Belle2 {

  /** ROIid stores the U and V ids and the sensor id of the Region Of Interest.
   */
  class ROIid : public RelationsObject {

  public:

    /** Default constructor.
     */
    ROIid();

    /** Constructor providing the ROI information */
    ROIid(int minUid, int maxUid, int minVid, int maxVid, VxdID sensorID) :
      m_minUid(minUid), m_maxUid(maxUid), m_minVid(minVid), m_maxVid(maxVid), m_sensorID(sensorID) {}


    /** Destructor.
     */
    virtual ~ROIid();

    //some accessors
    int getMinUid() const { return m_minUid; } /**< return the minimum U id of the ROI*/
    int getMaxUid() const { return m_maxUid; } /**< return the maximum U id of the ROI*/
    int getMinVid() const { return m_minVid; } /**< return the minimum V id of the ROI*/
    int getMaxVid() const { return m_maxVid; } /**< return the maximum V id of the ROI*/
    VxdID getSensorID() const { return m_sensorID; } /**< return the sensor ID of the ROI*/

    void setMinUid(double user_minUid) { m_minUid = user_minUid; } /**< set the minimum U id of the ROI*/
    void setMaxUid(double user_maxUid) { m_maxUid = user_maxUid; } /**< set the maximum U id of the ROI*/
    void setMinVid(double user_minVid) { m_minVid = user_minVid; } /**< set the minimum V id of the ROI*/
    void setMaxVid(double user_maxVid) { m_maxVid = user_maxVid; } /**< set the maximum V id of the ROI*/
    void setSensorID(VxdID user_sensorID) { m_sensorID = user_sensorID;} /**< set the sensor ID of the ROI*/

    bool Contains(const Belle2::PXDRawHit& thePXDRawHit) const; /**< true if the ROI contains the thePXDRawHit*/
    bool Contains(const Belle2::PXDDigit& thePXDDigit) const; /**< true if the ROI contains the thePXDDigit*/
    bool Contains(const Belle2::SVDShaperDigit& theSVDDigit) const; /**< true if the ROI contains the theSVDDigit*/

  private:

    int m_minUid; /**< u ID of the bottom left pixel of the ROI */
    int m_maxUid; /**< u ID of the top right pixel of the ROI */
    int m_minVid; /**< v ID of the bottom left pixel of the ROI */
    int m_maxVid; /**< v ID of the top right pixel of the ROI */
    VxdID m_sensorID; /**< sensor ID */

    //! Needed to make the ROOT object storable
    ClassDef(ROIid, 2)
  };
}
