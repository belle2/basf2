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

