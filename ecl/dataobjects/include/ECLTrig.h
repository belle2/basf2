/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *               Vasily Shebalin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLTRIG_H
#define ECLTRIG_H

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /*! Class to store ECLTrig, still need to be study
   * relation to ECLHit
   * filled in ecl/modules/eclDigitizer/src/ECLDigitizerModule.cc
   * and ecl/modules/eclUnpacker/ECLUnpacker.cc
   */

  class ECLTrig : public RelationsObject {
  public:
    /** default constructor for ROOT */
    ECLTrig()
    {
      m_TrigId = 0;    /**< Trig ID == crate ID */
      m_TimeTrig = 0; /**< Trig Time */
      m_TrigTag = 0;
    }


    /*! Set  TrigID
     */
    void setTrigId(int TrigId) { m_TrigId = TrigId; }
    /*! Set Trig Time (crate Id)
     */
    void setTrigTag(int TrigTag) { m_TrigTag = TrigTag; }
    /*! Set Triger Tag (crate Id)
     */
    void setTimeTrig(double TimeTrig) { m_TimeTrig = TimeTrig; }
    /*! Get Trig ID
     * @return trig ID
     */
    int getTrigId() const { return m_TrigId; }

    /*! Get Trigger tag
     * Trigger tag word width is 16 bit (bits 0-15).
     * Bit 16 is used as error flag.
     * If it is 1 then trigger tags from different shapers are not equal
     * @return Trigger tag
     */
    int getTrigTag() const { return m_TrigTag & 0xFFFF; }

    /*! Get trigger tag quality flag.
     * @return 0 if trigger tags from different shapers are equal. 1 otherwise.
     */
    int getTrigTagQualityFlag() const { return (m_TrigTag & 0x10000) >> 16; }

    /*! Get Trig Time
     * @return Trig Time
     */
    double getTimeTrig() const { return m_TimeTrig; }

  private:
    int m_TrigId;      /**< Trig ID */
    double m_TimeTrig; /**< Trig Time */
    int m_TrigTag;


//    ClassDef(ECLTrig, 1);/**< ClassDef */
//    ClassDef(ECLTrig, 2); // CellId -> TrigId
    ClassDef(ECLTrig, 3); // TrigTag added

  };
} // end namespace Belle2

#endif
