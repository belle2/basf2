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

namespace Belle2 {

  /** ROIpayload
   * @TODO: Better explanation, Is there a reason to inherit from TObject and not Relationsobject here?
   */

  class ROIpayload : public TObject {

  public:
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

  private:
    //! Needed to make the ROOT object storable
    ClassDef(ROIpayload, 1)
  };
}
