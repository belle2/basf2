/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef SVDTRIGGERTYPE_H
#define SVDTRIGGERTYPE_H

#include <cstdint>

namespace Belle2 {

  /**
   * Class to store Trigger Type information
   */
  class SVDTriggerType {

  public:
    /** The base integer type for SVDTriggerType */
    typedef uint8_t baseType;

    /** Constructor using the unique type.  */
    explicit SVDTriggerType(baseType type = 0)
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

    /**
       how to interpret the 4 bits:

       PID0 0x0 fine-timing trigger, tfine=0
       PID1 0x4 fine-timing trigger, tfine=1
       PID2 0x8 fine-timing trigger, tfine=2
       PID3 0xc fine-timing trigger, tfine=3
       RSV0 0x2 reserve 0
       RSV1 0x6 reserve 1
       RSV2 0xa reserve 2
       RSV3 0xe reserve 3
       ECL  0x1 ECL trigger, coarse-timing
       CDC  0x3 CDC trigger, coarse-timing
       DPHY 0x5 delayed physics trigger
       RAND 0x7 random trigger
       TEST 0x9 test trigger
       RSV4 0xb reserve 4
       RSV5 0xd reserve 5
       NONE 0xf not a trigger
     */

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

