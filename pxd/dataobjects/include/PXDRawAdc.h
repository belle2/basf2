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

#pragma once

#include <vxd/dataobjects/VxdID.h>
#include <framework/datastore/RelationsObject.h>
#include <vector>

namespace Belle2 {

  /** The PXD Raw Adc class
    * This class stores information about Raw Adc for Pedestals
    */
  class PXDRawAdc : public RelationsObject {
  public:

    /** Default constructor for the ROOT IO. */
    PXDRawAdc():
      m_sensorID(0), m_dhp_header(0), m_adcs() {};


    /**
     * @param sensorID Sensor compact ID.
     * @param data raw data pointer
     */
    PXDRawAdc(VxdID sensorID, void* data, int len);

    /** Get the sensor ID.
     * @return ID of the sensor.
     */
    VxdID getSensorID() const
    {
      return m_sensorID;
    }

    /** Get static pointer to data.
     * @return pointer.
     */
    const std::vector<uint8_t>& getData() const
    {
      return m_adcs;
    }

    /** Get Chip
     * @return chip id.
     */
    unsigned short getChip() const
    {
      return m_dhp_header & 0x3;
    }

  private:
    unsigned short m_sensorID; /**< Compressed sensor identifier. actually a VxdID object */
    unsigned short m_dhp_header; /**< needed for Chip id */
    std::vector <uint8_t> m_adcs; /**< Raw ADC data as it is memory dumped by the DHP */

    ClassDef(PXDRawAdc, 3)
  };


} //Belle2 namespace

