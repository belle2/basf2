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
   *  For simulation there is the SVDEventInfoSetter module to set the parameters
   */
  class SVDEventInfo : public TObject {
  public:

    /** Constructor. */
    explicit SVDEventInfo(SVDModeByte mode = SVDModeByte(), SVDTriggerType type = SVDTriggerType()):
      m_modeByte(mode.getID()), m_triggerType(type.getType()) {m_ModeByteMatch = true; m_TriggerTypeMatch = true; m_Xtalk = false; m_relativeTimeShift = 0;}

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

    /** relative 3/6 shift
     *  Sets the relative shift in latency in data taken in 3/6 samples
     *  int between 0 and 15 (shift is 0*7.9 .... 15*7.9 in ns)
     */
    void setRelativeShift(int relativeShift) {m_relativeTimeShift = relativeShift;}

    /** nAPVsamples setter
     * Sets the number of acquired samples (3 or 6) found in the raw data
     */
    void setNSamples(int nAPVsamples) {m_nAPVsamples = nAPVsamples;}

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

    /** returns the time shift between SVD reference
     *  and FTSW (Trigger) reference:
     *  time in FTSW reference = time in SVD reference + time shift
     *  for clusters, additional information of the first frame is
     *  used to improve the precision
     */

    /** returns the number of samples, 6, 3 or 1 */
    int getNSamples() const
    { return m_nAPVsamples; }

    float getSVD2FTSWTimeShift(int firstFrame) const
    { return 4000. / 509 * (3 - SVDModeByte(m_modeByte).getTriggerBin() + 4 * firstFrame); }

    /** getRelativeShiftInNs
     * returns the relative shift of the latency, in ns, in data taken in 3/6 samples.
     * A number between 0 and 15 (in xml) -> shift is 0*7.9 .... 15*7.9 in ns.
     * It returns the correct value in ns only if data have been collected in 3-sample DAQmode,
     * otherwise it returns 0
     */

    float getRelativeShiftInNs() const
    {
      if (m_nAPVsamples == 3)
        return m_relativeTimeShift * 4000. / 509.;

      return 0;
    }

    /** getRelativeShift
     * returns the relative shift in data taken in 3/6 samples
     * A number between 0 and 15 as written in the xml file
     */
    int getRelativeShift() const
    {return m_relativeTimeShift;}

    /** getTimeInFTSWReference(double time_in_SVD)
     * it takes the time in SVD reference (in either 3 or 6-sample DAQ mode)
     * and the firstFrame and provides the time in the FTWS reference
     * In the SVD reference t=0 is the time of the first sample, regardless if the event is acquired in 3- o 6-sample DAQ mode

     */
    double getTimeInFTSWReference(double time, int firstFrame) const
    {
      double time_in_FTSW = time + getRelativeShiftInNs() + getSVD2FTSWTimeShift(firstFrame);
      return time_in_FTSW;
    }


    /** getTimeInSVDReference
     * it takes time in FTSW reference and provides
     * the time the SVD reference for that event.
     * In the SVD reference t=0 is the time of the first sample, regardless if the event is acquired in 3- o 6-sample DAQ mode
     */
    double getTimeInSVDReference(double time_in_FTSW, int firstFrame) const
    {
      double time = time_in_FTSW - getRelativeShiftInNs() - getSVD2FTSWTimeShift(firstFrame);
      return time;
    }

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
      bool thisRelativeShift(m_relativeTimeShift);

      std::ostringstream os;

      os << " Full SVDModeByte: " << (unsigned int)thisMode << std::endl;
      os << " Triggerbin: " << (unsigned int) thisMode.getTriggerBin() << std::endl;
      os << "RunType: " << (unsigned int)thisMode.getRunType() << ", EventType: " << (unsigned int) thisMode.getEventType() <<
         ", DAQMode:  " << (unsigned int) thisMode.getDAQMode() << std::endl;
      os << " ModeByte Match: " << thisModeMatch << std::endl;
      os << " Trigger Type: " << (unsigned int)thisType.getType() << std::endl;
      os << " TriggerType Match: " << thisTriggerMatch << std::endl;
      os << " Cross Talk: " << (thisXtalk ? "true" : "false") << std::endl;
      os << " Relative Shift 3/6: " << (unsigned int)thisRelativeShift << std::endl;
      return os.str();
    }

  private:

    SVDModeByte::baseType m_modeByte = SVDModeByte::c_DefaultID; /**< SVDModeByte object (runType + eventType + DAQmode + TriggerBin) */
    SVDTriggerType::baseType m_triggerType;    /**< information on the Trigger type */
    bool m_ModeByteMatch; /**< flag telling if the SVDModeByte object is the same for each FADCs in the event */
    bool m_TriggerTypeMatch; /**< flag telling if the SVDTriggerType object is the same for each FADCs in the event */
    bool m_Xtalk = false;    /**< information on the x-talk */
    int m_relativeTimeShift = 0; /**< relative shift in units of APV-clock/4 between 3- and 6-sample acquired events */
    int m_nAPVsamples = 0; /**< number of acquired samples */

    /**class def needed by root*/
    ClassDef(SVDEventInfo, 2);

  }; //class
} // namespace Belle2




