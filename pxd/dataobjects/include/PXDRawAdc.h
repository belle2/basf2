/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDRAWADC_H
#define PXDRAWADC_H


#include <vxd/dataobjects/VxdID.h>
#include <framework/datastore/RelationsObject.h>

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
    PXDRawAdc(VxdID sensorID, void* data, bool pedestal_flag):
      m_sensorID(sensorID) , m_adcs() {
      unsigned char* d = (unsigned char*)data;
      m_dhp_header = ((unsigned short*)data)[2];
      d += 8; // Skip DHH and DHP header, data is 64kb large (+ 8 bytes)
      if (pedestal_flag) {
        for (unsigned int i = 0; i < sizeof(m_adcs); i++) {
          m_adcs[i] = d[2 * i + 1]; // Check if endianess is correctly done... TODO , seems so
        }
      } else {
        // memcpy(m_adcs,d,sizeof(m_adcs));// check if we need to swap endianess TODO , we do :-(
        for (unsigned int i = 0; i < sizeof(m_adcs); i++) {
          m_adcs[i] = d[i ^ 0x1]; // swap endianess --- again ---
        }
      }
    };

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
      return m_adcs;
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
    unsigned char m_adcs[64 * 1024]; /**< Raw ADC data as it is memmory dumped by the DHP */

    // ~PXDRawAdc();

    ClassDef(PXDRawAdc, 2)
  };


} //Belle2 namespace
#endif
