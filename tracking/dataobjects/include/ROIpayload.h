/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/dataobjects/ROIrawID.h>
#include "TObject.h"
#include <stdio.h>
#include <stdint.h>
#include <boost/spirit/home/support/detail/endian.hpp>

namespace Belle2 {

  /** ROIpayload
   * @TODO: Better explanation, Is there a reason to inherit from TObject and not Relationsobject here?
   */

  class ROIpayload : public TObject {
  public:
    typedef boost::spirit::endian::ubig32_t ubig32_t;

    /** Default constructor.
     */
    ROIpayload(int rois = 0);

    /**
     * destructor.
     */
    virtual ~ROIpayload() { delete[] m_rootdata; };

    int m_packetLengthByte; /**< packet length  in byte*/
    int m_length; /**< packet length*/
    int*  m_rootdata; //[m_length] /**< */

    int m_index; //! transient value /**< index*/
    uint32_t* m_data32; //! transient value /**< data32*/
    ROIrawID::baseType* m_data64; //! transient value /**< data64*/

    void setPayloadLength(int length); /**< set payload length*/
    void setPayloadLength(); /**< set payload length*/
    void setHeader(bool Accepted, bool SendAll, bool SendROIs);  /**< set header*/
    void setTriggerNumber(unsigned long int triggerNumber); /**< set trigger number*/

    //! set run/ subrun/exp number
    void setRunSubrunExpNumber(int run , int subrun , int exp); /**< set run, subrun and experiment number*/

    //    void addROIraw(ROIrawID roiraw);
    void addROIraw(unsigned long int roiraw); /**< add a ROIrawID */
    void setCRC(); /**< set CRC */

    int getPacketLengthByte() {return m_packetLengthByte;}; /**< get packet length in byte*/
    int getLength() {return m_length;}; /**< get length*/
    int* getRootdata() {return m_rootdata;}; //[m_length] /**< */

    void init(int length); /**< initializer*/

    /** Get the nr of ROIs.
     * @return Nr of ROIs.
     */
    int getNrROIs() const
    {
      return (m_length - 5) / 2; // only minus checksum
    }

    /** Return DHH ID of ROI j
     * @param j Index of ROI
     * @return DHH ID
     */
    int getDHHID(int j) const
    {
      if (j < 0 || j >= getNrROIs()) return -1;
      return (((ubig32_t*)m_rootdata)[4 + 2 * j] & 0x3F0) >> 4;  // & 0x3F0
    }

    /** Return Row 1 of ROI j
     * @param j Index of ROI
     * @return  Row 1
     */
    int getRow1(int j) const
    {
      if (j < 0 || j >= getNrROIs()) return -1;
      return ((((ubig32_t*)m_rootdata)[4 + 2 * j] & 0xF) << 6) | ((((ubig32_t*)m_rootdata)[4 + 2 * j + 1] & 0xFC000000) >> 26) ;
    }

    /** Return Row 2 of ROI j
     * @param j Index of ROI
     * @return Row 2
     */
    int getRow2(int j) const
    {
      if (j < 0 || j >= getNrROIs()) return -1;
      return (((ubig32_t*)m_rootdata)[4 + 2 * j + 1] & 0x3FF00) >> 8;
    }

    /** Return Col 1 of ROI j
     * @param j Index of ROI
     * @return Column 1
     */
    int getCol1(int j) const
    {
      if (j < 0 || j >= getNrROIs()) return -1;
      return (((ubig32_t*)m_rootdata)[4 + 2 * j + 1] & 0x03FC0000) >> 18;
    }

    /** Return Col 1 of ROI j
     * @param j Index of ROI
     * @return Column 2
     */
    int getCol2(int j) const
    {
      if (j < 0 || j >= getNrROIs()) return -1;
      return (((ubig32_t*)m_rootdata)[4 + 2 * j + 1]) & 0xFF;
    }

    /** Return Type (Datcon or HLT) of ROI j
     * @param j Index of ROI
     * @return Type of Roi
     */
    int getType(int j) const
    {
      if (j < 0 || j >= getNrROIs()) return -1;
      return (((ubig32_t*)m_rootdata)[4 + 2 * j] & 0x400) >> 10;
    }

  private:
    //! Needed to make the ROOT object storable
    ClassDef(ROIpayload, 1)
  };
}
