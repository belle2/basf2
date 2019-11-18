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
    explicit SVDEventInfo(SVDModeByte mode = SVDModeByte(), SVDTriggerType type = SVDTriggerType()):
      m_modeByte(mode.getID()), m_triggerType(type.getType()) {m_ModeByteMatch = true; m_TriggerTypeMatch = true; m_Xtalk = false;}

    /** Destructor. */
    ~SVDEventInfo() {}

    /** Match TriggerType setter
     *  Sets the flag telling us if the SVDTriggerType object is the same for each FADCs in the event
     */
    void setMatchTriggerType(bool triggermatch) {m_TriggerTypeMatch = triggermatch;}

    /** Match ModeByte setter
     *  Sets the flag telling us if the SVDModeByte object is the same for each FADCs in the event
     */
    void setMatchModeByte(bool modematch) {m_ModeByteMatch = modematch;}

    /** SVDModeByte setter
     *  Sets the SVDModeByte data in the EventInfo object
     */
    void setModeByte(SVDModeByte mode) {m_modeByte = mode.getID();}

    /** SVDTriggerType setter
     *  Sets the type of SVDTrigger in the EventInfo object
     */
    void setTriggerType(SVDTriggerType type) {m_triggerType = type.getType();}

    /** x-Talk setter
     *  Sets the x-talk info in the EventInfo object
     */
    void setCrossTalk(bool xtalk) {m_Xtalk = xtalk;}

    /** Match TriggerType getter
     *  Gets the flag telling us if the SVDTriggerType object is the same for each FADCs in the event
     */
    bool getMatchTriggerType() {return m_TriggerTypeMatch;}

    /** MatchModeByte getter
     *  Gets the flag telling us if the SVDModeByte object is the same for each FADCs in the event
     */
    bool getMatchModeByte() {return m_ModeByteMatch;}

    /** SVDModeByte getter
     *  Gets the SVDModeByte info for the event
     */
    SVDModeByte getModeByte() const
    { return m_modeByte; }

    /** SVDTriggerType getter
     *  Gets the type of SVDTrigger for the event
     */
    SVDTriggerType getTriggerType() const
    { return SVDTriggerType(m_triggerType); }

    /** X-talk info getter
     *  Gets the X-talk info for the event
     */
    bool isCrossTalkEvent() { return m_Xtalk; }


    /** Display main parameters in this object */
    std::string toString() const
    {
      SVDModeByte thisMode(m_modeByte);
      SVDTriggerType thisType(m_triggerType);

      bool thisModeMatch(m_ModeByteMatch);
      bool thisTriggerMatch(m_TriggerTypeMatch);
      bool thisXtalk(m_Xtalk);

      std::ostringstream os;

      os << " Full SVDModeByte: " << (unsigned int)thisMode << std::endl;
      os << " Triggerbin: " << (unsigned int) thisMode.getTriggerBin() << std::endl;
      os << "RunType: " << (unsigned int)thisMode.getRunType() << ", EventType: " << (unsigned int) thisMode.getEventType() <<
         ", DAQMode:  " << (unsigned int) thisMode.getDAQMode() << std::endl;
      os << " ModeByte Match: " << thisModeMatch << std::endl;
      os << " Trigger Type: " << (unsigned int)thisType.getType() << std::endl;
      os << " TriggerType Match: " << thisTriggerMatch << std::endl;
      os << " Cross Talk: " << (thisXtalk ? "true" : "false") << std::endl;
      return os.str();
    }

  private:

    SVDModeByte::baseType m_modeByte = SVDModeByte::c_DefaultID; /**< SVDModeByte object (runType + eventType + DAQmode + TriggerBin) */
    SVDTriggerType::baseType m_triggerType;    /**< information on the Trigger type */
    bool m_ModeByteMatch; /**< flag telling if the SVDModeByte object is the same for each FADCs in the event */
    bool m_TriggerTypeMatch; /**< flag telling if the SVDTriggerType object is the same for each FADCs in the event */
    bool m_Xtalk = false;    /**< information on the x-talk */

    ClassDef(SVDEventInfo, 1);

  }; //class
} // namespace Belle2




