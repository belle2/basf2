/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <TObject.h>

namespace Belle2 {
  /**
   *  ARICH HAPD Sensor Mask: map of operational/nonoperational HAPDs : one bit per sensor
   */

  class ARICHHapdMask: public TObject {
  public:
    /**
     * Various constants
     */
    enum {c_Size = 54, /**< storage size in bytes 420 sensors / 8 bits */
         };

    /**
     * Default constructor
     */
    ARICHHapdMask()
    {
      for (unsigned i = 0; i < c_Size; i++)
        m_mask[i] = 0;
    };

    /**
     * Constructor
     */
    explicit ARICHHapdMask(unsigned char* mask)
    {
      for (unsigned i = 0; i < c_Size; i++)
        m_mask[i] = mask[i];
    };

    /**
     * Destructor
     */
    ~ARICHHapdMask() {};


  private:
    unsigned char m_mask[c_Size];       /**< mask 1 bit per sensor */

    ClassDef(ARICHHapdMask, 1);  /**< ClassDef */
  };
} // end namespace Belle2

