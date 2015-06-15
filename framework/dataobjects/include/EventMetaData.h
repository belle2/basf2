/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Andreas Moll                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef EVENTMETADATA_H
#define EVENTMETADATA_H

#include <TObject.h>

namespace Belle2 {
  /** Store event number, run number, and experiment number.
   *
   *  Modules controlling the event-flow (e.g. EventInfoSetter, RootInput) have to create an object of this type!
   *
   *  @sa EventInfoSetterModule, EventInfoPrinterModule
   *  @author <a href="mailto:martin.heck@kit.edu?subject=EventMetaData">Martin Heck</a>
   */
  class EventMetaData : public TObject {
  public:

    /** Constructor. */
    explicit EventMetaData(unsigned long event = 0, unsigned long run = 0, unsigned long experiment = 0);

    /** Destructor. */
    ~EventMetaData() {}

    /** bit-flag format of m_error_flag  */
    enum EventErrorFlag {
      c_B2LinkCRCError = 0x1, /**< Belle2link CRC error  is detected in the event */
      c_HLTError       = 0x2  /**< Error is returned from HLT modules. */
    };

    /** Event Setter.
     *
     *  @param event The current event number.
     */
    void setEvent(unsigned long event)
    {
      m_event = event;
    }

    /** Run Setter.
     *
     *  @param run The number of the current run.
     */
    void setRun(unsigned long run)
    {
      m_run = run;
    }

    /** Sub run Setter.
     *
     *  @param subrun The number of the current sub run.
     */
    void setSubrun(unsigned long subrun)
    {
      m_subrun = subrun;
    }

    /** Experiment Setter.
     *
     *  @param experiment The number of the current experiment.
     */
    void setExperiment(unsigned long experiment)
    {
      m_experiment = experiment;
    }

    /** Time Setter.
     *
     *  @param time The time when the event was recorded.
     */
    void setTime(unsigned long long int time)
    {
      m_time = time;
    }

    /** Marks the end of the data processing.
     *
     * Can be used by any module to safely halt event processing.
     * After this is set on StoreObjPtr<EventMetaData> and your event() function returns,
     * no further modules in the path will be called and normal cleanup will
     * be done (i.e. endRun(), terminate()).
     *
     * Using this will produce a warning that your module stopped execution early,
     * so you might want to add an explanatory message of your own.
     */
    void setEndOfData();

    /** is end-of-data set? (see setEndOfData()). */
    bool isEndOfData() const;

    /** Parent Index Setter.
     *
     *  @param index The index of the current parent file.
     */
    void setParentIndex(int index)
    {
      m_parent_index = index;
    }

    /** Generated Weight Setter.
     *
     *  @param weight The weight produced by the event generator
     */
    void setGeneratedWeight(double weight)
    {
      m_generated_weight = weight;
    }

    /** Set Error Flag
     *
     *   @param error_flag a bit flag for an error event.
     */
    void setErrorFlag(unsigned int error_flag)
    {
      m_error_flag = error_flag;
    }

    /** Add Error Flag
     *
     *   @param error_flag a bit flag for an error event.
     */
    void addErrorFlag(EventErrorFlag event_error_flag)
    {
      m_error_flag |= event_error_flag;
    }

    /** Get error flag
     *
     *  @return Error flag for this event.
     */
    unsigned int getErrorFlag() const
    {
      return m_error_flag;
    }

    /** Event Getter.
     *
     *  @return The number of the current event.
     */
    unsigned long getEvent() const
    {
      return m_event;
    }

    /** Run Getter.
     *
     *  @return The number of the current run.
     */
    unsigned long getRun() const
    {
      return m_run;
    }

    /** Sub run Getter.
     *
     *  @return Sub-run number, increases indicate recovery from DAQ-internal trouble without
     *          change to detector constants. Not supposed to be used by offline analysis.
     */
    unsigned long getSubrun() const
    {
      return m_subrun;
    }

    /** Experiment Getter.
     *
     *  @return The number of the current experiment.
     */
    unsigned long getExperiment() const
    {
      return m_experiment;
    }

    /** Time Getter.
     *
     *  @return The time when the event was recorded.
     */
    unsigned long long int getTime() const
    {
      return m_time;
    }

    /** Parent Index Getter.
     *
     *  @return The index of the current parent file, or UINT_MAX if not set.
     */
    int getParentIndex() const
    {
      return m_parent_index;
    }

    /** Generated Weight Getter.
     *
     *  @return The weight produced by the event generator
     */
    double getGeneratedWeight() const
    {
      return m_generated_weight;
    }

    /** Comparison Operator.
     *
     *  @param eventMetaData The event meta data to compare with.
     *  @return True if event, run, and experiment numbers are the same
     */
    bool operator== (const EventMetaData& eventMetaData) const;

    /** Comparison Operator.
     *
     *  @param eventMetaData The event meta data to compare with.
     *  @return True if event, run, or experiment numbers are different
     */
    bool operator!= (const EventMetaData& eventMetaData) const { return !(*this == eventMetaData); }


  private:

    unsigned long m_event; /**< Event number.  */

    unsigned long m_run; /**< Run number.  */

    unsigned long
    m_subrun; /**< Sub-run number, increases indicate recovery from DAQ-internal trouble without change to detector constants. Not supposed to be used by offline analysis.  */

    unsigned long m_experiment; /**< Experiment number.  */

    unsigned long long int m_time; /**< Time in ns since epoch (1970-01-01).  */

    int m_parent_index; /**< Index of parent file.  */

    double m_generated_weight; /**< Generated weight.  */

    unsigned int m_error_flag;  /**< Error flag.  */

    ClassDef(EventMetaData, 3); /**< Store event number, run number, and experiment number. */
  }; //class
} // namespace Belle2
#endif // EVENTMETADATA
