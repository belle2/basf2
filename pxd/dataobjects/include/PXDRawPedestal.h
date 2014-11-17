/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//! *************************************************************
//! Important Remark:
//! Up to now the format for Raw frames as well as size etc
//! is not well defined. It will most likely change!
//! E.g. not the whole mem is dumped, but only a part of it.
//! *************************************************************

#ifndef PXDRAWPEDESTAL_H
#define PXDRAWPEDESTAL_H

#include <vxd/dataobjects/VxdID.h>
#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /** The PXD Raw Adc class
    * This class stores information about Raw Adc for Pedestals
    */
  class PXDRawPedestal : public RelationsObject {
  public:

    /** Default constructor for the ROOT IO. */
    PXDRawPedestal():
      m_sensorID(0), m_dhp_header(0), m_pedestals() {};


    /**
     * @param sensorID Sensor compact ID.
     * @param data raw data pointer
     */
    PXDRawPedestal(VxdID sensorID, void* data);

    /** Get the sensor ID.
     * @return ID of the sensor.
     */
    VxdID getSensorID() const {
      return m_sensorID;
    }

    /** Get static pointer to data.
     * @return pointer.
     */
    const unsigned char* getData() const {
      return m_pedestals;
    }

    /** Get Chip
     * @return chip id.
     */
    unsigned short getChip() const {
      return m_dhp_header & 0x3;
    }

  protected:
    unsigned short m_sensorID; /**< Compressed sensor identifier. actually a VxdID object */
    unsigned short m_dhp_header; /**< needed for Chip id */
    unsigned char m_pedestals[64 * 1024]; /**< Raw Pedestal data as it is memory dumped by the DHP */

    // ~PXDRawPedestal();

    ClassDef(PXDRawPedestal, 2)
  };


} //Belle2 namespace
#endif
