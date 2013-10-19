/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef ROIPAYLOAD_H
#define ROIPAYLOAD_H

#include <stdio.h>
#include "TObject.h"
#include <tracking/pxdDataReductionClasses/ROIrawID.h>
#include <stdint.h>

namespace Belle2 {

  /** ROIpayload
   *
   *
   */

  class ROIpayload : public TObject {

  public:

    /**
     * default constructor.
     */

    ROIpayload(int length = 0);

    /**
     * destructor.
     */
    virtual ~ROIpayload() { delete m_rootdata; };

    int m_packetLengthByte;
    int m_length;
    int*  m_rootdata; //[m_length]

    int m_index; //! transient value
    uint32_t* m_data32; //! transient value
    ROIrawID::baseType* m_data64; //! transient value

    void setPayloadLength(int length);
    void setHeader();
    void setTriggerNumber(unsigned long int triggerNumber);
    void addROIraw(ROIrawID roiraw);
    void setCRC();

    int getPacketLengthByte() {return m_packetLengthByte;};
    int getLength() {return m_length;};
    int* getRootdata() {return m_rootdata;}; //[m_length]

    void init(int length);

  private:


    //! Needed to make the ROOT object storable
    ClassDef(ROIpayload, 1)
  };
}

#endif  // ROIPAYLOAD_H  
