/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jarek Wiechczynski                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <string>
#include <svd/dataobjects/SVDModeByte.h>
#include <svd/dataobjects/SVDTriggerType.h>


namespace Belle2 {
  /** Stores SVDModeByte object with Trigger time, DAQ mode, Run type & Event type!
   *  Also - the information of any inconsistencies of these variables within one event are stored
   *
   *  For data this object will be filled by the SVDUnpacker once per event
   *
   *  For simulation there is a SVDEventInfoSetter module to set the parameters
   */
  class SVDEventInfo : public TObject {
  public:


    /** Constructor. */
    explicit SVDEventInfo(bool modeByteMatch = true, SVDModeByte mode = SVDModeByte()):
      m_ModeByteMatch(modeByteMatch), m_mode(mode.getID()) {}

    /** Destructor. */
    ~SVDEventInfo() {}



    /** MatchModeByte setter
     *  Sets the flag telling us if the SVDModeByte object is the same for each FADCs in the event
     */
    void setMatchModeByte(bool modematch) {m_ModeByteMatch = modematch;}

    /** SVDModeByte setter
     *  Sets the SVDModeByte data in the EventInfo object
     */
    void setModeByte(SVDModeByte mode) {m_mode = mode.getID();}

    /** SVDTriggerType setter
     *  Sets the type of SVDTrigger in the EventInfo object
     */
    void setTriggerType(SVDTriggerType type) {m_trigger = type.getType();}

    /** x-Talk setter
     *  Sets the x-talk info in the EventInfo object
     */
    void setCrossTalk(bool xtalk) {m_Xtalk = xtalk;}




    /** MatchModeByte getter
     *  Gets the flag telling us if the SVDModeByte object is the same for each FADCs in the event
     */
    bool getMatchModeByte() {return m_ModeByteMatch;}

    /** SVDModeByte getter
     *  Gets the SVDModeByte info for the event
     */
    SVDModeByte getModeByte() const
    { return m_mode; }

    /** SVDTriggerType getter
     *  Gets the type of SVDTrigger for the event
     */
    SVDTriggerType getTriggerType() const
    { return m_trigger; }

    /** X-talk info getter
     *  Gets the X-talk info for the event
     */
    bool isCrossTalkEvent() { return m_Xtalk; }




    /** Display main parameters in this object */
    std::string toString() const
    {
      SVDModeByte thisMode(m_mode);
      SVDTriggerType thisType(m_trigger);

      bool thisModeMatch(m_ModeByteMatch);
      bool thisXtalk(m_Xtalk);

      std::ostringstream os;

      os << " Full SVDModeByte: " << (unsigned int)thisMode << std::endl;
      os << " Triggerbin: " << (unsigned int) thisMode.getTriggerBin() << std::endl;
      os << "RunType: " << (unsigned int)thisMode.getRunType() << ", EventType: " << (unsigned int) thisMode.getEventType() <<
         ", DAQMode:  " << (unsigned int) thisMode.getDAQMode() << std::endl;
      os << " ModeByte Match: " << thisModeMatch << std::endl;
      os << " Trigger Type: " << (unsigned int)thisType.getType() << std::endl;
      os << " Cross Talk: " << (thisXtalk ? "true" : "false") << std::endl;
      return os.str();
    }

  private:
    bool m_ModeByteMatch; /**< flag telling if the SVDModeByte object is the same for each FADCs in the event */
    SVDModeByte::baseType m_mode = SVDModeByte::c_DefaultID; /**< SVDModeByte object (runType + eventType + DAQmode + TriggerBin) */
    bool m_Xtalk;    /**< information on the x-talk */
    SVDTriggerType::baseType m_trigger;    /**< information on the Trigger type */

    ClassDef(SVDEventInfo, 1);

  }; //class
} // namespace Belle2




