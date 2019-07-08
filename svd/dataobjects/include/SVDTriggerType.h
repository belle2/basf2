/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jarek Wiechczynski                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDTRIGGERTYPE_H
#define SVDTRIGGERTYPE_H

#include <cstdint>
#include <string>
#include <ostream>
#include <utility>

namespace Belle2 {

  class SVDTriggerType {

  public:
    /** The base integer type for SVDTriggerType */
    typedef uint8_t baseType;

    /** Constructor using the unique type.  */
    SVDTriggerType(baseType type = 0)
    {
      m_triggerID.type = type;
    }

    /** Set the unique type */
    void setType(baseType type) { m_triggerID.type = type; }

    /** Get the unique type */
    baseType getType() const { return m_triggerID.type; }

    /*
     * additional methods will come later...
     */

  private:
    union {
      /** Unique id */
      baseType type: 4;

      // 2 structs convenient for retrieving info (maybe one more to come....)
      struct {
        baseType bin1: 1;  /** LSB */
        baseType bin2: 1;
        baseType bin3: 1;
        baseType bin4: 1;  /** MSB */
      } parts1;
      struct {
        baseType bin12: 2;  /** first 2 bits */
        baseType bin34: 2;  /** last 2 bits */
      } parts2;

    } m_triggerID; /**< Union to store the ID and all components in one go. */
  };
} // namespace Belle2

#endif

