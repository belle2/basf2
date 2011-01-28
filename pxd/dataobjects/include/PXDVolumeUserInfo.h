/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDVOLUMEUSERINFO_H_
#define PXDVOLUMEUSERINFO_H_

#include <geometry/dataobjects/VolumeUserInfoBase.h>

#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  /**
    * PXDVolumeUserInfo - Additional information for a PXD sensitive volume.
    *
    * This class holds additional information for PXD sensitive volumes, which are used
    * later to set this information for a PXDSimHit.
    */
  class PXDVolumeUserInfo : public VolumeUserInfoBase {

  public:

    /** Empty constructor, required for ROOT IO.*/
    PXDVolumeUserInfo(): VolumeUserInfoBase(),
        m_layerID(-1),    /* Layer ID. */
        m_ladderID(-1),   /* Ladder ID. */
        m_sensorID(-1) {  /* Sensor ID. */
      /*! Does nothing */
    }

    /** Full constructor.
     * @param layerID ID of the layer.
     * @param ladderID ID of the ladder.
     * @param sensorID ID of the sensor.
     */
    PXDVolumeUserInfo(
      int layerID,
      int ladderID,
      int sensorID): VolumeUserInfoBase(),
        m_layerID(layerID),
        m_ladderID(ladderID),
        m_sensorID(sensorID) {
      /* Does nothing. */
    }

    /** Destructor */
    ~PXDVolumeUserInfo() {
      /* Does nothing. */
    }

    /** Sets the layer ID.
     * @param layerID The layer ID of the sensitive volume.
     */
    void setLayerID(int layerID) { m_layerID = layerID; }

    /** The method to set LadderID.*/
    void setLadderID(int ladderID) { m_ladderID = ladderID; }

    /** The method to set SensorID.*/
    void setSensorID(int sensorID) { m_sensorID = sensorID; }

    /** The method to get layer id.*/
    int getLayerID() const { return m_layerID; }

    /** The method to get ladder id.*/
    int getLadderID() const { return m_ladderID; }

    /** The method to get sensor id.*/
    int getSensorID() const { return m_sensorID; }

    /** Assignment operator.*/
    PXDVolumeUserInfo& operator=(const PXDVolumeUserInfo& other);

  private:

    int m_layerID;           /**< Layer number. */
    int m_ladderID;          /**< Ladder number. */
    int m_sensorID;          /**< Sensor number. */

    ClassDef(PXDVolumeUserInfo, 1)

  };

} // end namespace Belle2

#endif /* PXDVOLUMEUSERINFO_H_ */
