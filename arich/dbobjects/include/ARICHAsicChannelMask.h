/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Rok Pestotnik, Manca Mrvar                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <vector>

namespace Belle2 {
  /**
   *  ARICH Channel Mask
   */
  class ARICHAsicChannelMask: public TObject {
  public:

    /**
     * Various constants
     */
    enum {c_Size = 18, /**< storage size in bytes 4*36 channels / 8 bits */
         };

    /**
     * Default constructor
     */
    ARICHAsicChannelMask()
    {
      for (unsigned i = 0; i < c_Size; i++)
        m_mask[i] = 0;
    };

    /**
     * Constructor
     */
    explicit ARICHAsicChannelMask(unsigned char* mask)
    {
      for (unsigned i = 0; i < c_Size; i++)
        m_mask[i] = mask[i];
    };

    /**
     * Destructor
     */
    ~ARICHAsicChannelMask() {};


  private:
    unsigned char m_mask[c_Size];       /**< FPGA bitfile */

    ClassDef(ARICHAsicChannelMask, 1);  /**< ClassDef */

  };
} // end namespace Belle2
