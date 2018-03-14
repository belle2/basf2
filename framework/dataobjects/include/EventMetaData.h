/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Andreas Moll, Thomas Kuhr                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>

#include <string>

namespace Belle2 {
  /** Store event, run, and experiment numbers. (see member variable doc for detailed description)
   *
   *  An object of this type is registered by e.g. EventInfoSetter for newly generated data,
   *  and updated with proper values for each event. Alternatively it can also be read from file.
   *
   *  This object is used for controlling the event-flow, e.g. via changing run-numbers
   *  or setEndOfData() (see also EventProcessor).
   *
   *  See BELLE2-NOTE-TE-2015-028: Event, File, and Dataset Metadata for a
   *  detailed definition. Available at: https://docs.belle2.org/record/287?ln=en
   *
   *  @sa EventInfoSetterModule, EventInfoPrinterModule
   */
  class EventMetaData : public TObject {
  public:

    /** Constructor. */
    explicit EventMetaData(unsigned int event = 1, int run = 0, int experiment = 0);

    /** Destructor. */
    ~EventMetaData() {}

    /** bit-flag format of m_error_flag  */
    enum EventErrorFlag {
      c_B2LinkPacketCRCError = 0x1, /**< Belle2link CRC error  is detected in the event */
      c_B2LinkEventCRCError = 0x2, /**< HSLB_COPPER CRC error  is detected in the event */
    };

    /** Event Setter.
     *
     *  @param event The current event number.
     */
    void setEvent(unsigned int event) { m_event = event; }

    /** Run Setter.
     *
     *  @param run The number of the current run.
     */
    void setRun(int run) { m_run = run; }

    /** Sub run Setter.
     *
     *  @param subrun The number of the current sub run.
     */
    void setSubrun(int subrun) { m_subrun = subrun; }

    /** Experiment Setter.
     *
     *  @param experiment The number of the current experiment.
     */
    void setExperiment(int experiment) { m_experiment = experiment; }

    /** Production Setter.
     *
     *  @param production The integer identifier of the production.
     */
    void setProduction(int production) { m_production = production; }

    /** Time Setter.
     *
     *  @param time The time when the event was recorded.
     */
    void setTime(unsigned long long int time) { m_time = time; }

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

    /** Parent LFN Setter.
     *
     *  @param parent The LFN of the current parent file.
     */
    void setParentLfn(const std::string& parent) { m_parentLfn = parent; }

    /** Generated Weight Setter.
     *
     *  @param weight The weight produced by the event generator
     */
    void setGeneratedWeight(double weight) { m_generatedWeight = weight; }

    /** Set Error Flag
     *
     *   @param errorFlag a bit flag for an error event.
     */
    void setErrorFlag(unsigned int errorFlag) { m_errorFlag = errorFlag; }

    /** Add Error Flag
     *
     *   @param errorFlag a bit flag for an error event.
     */
    void addErrorFlag(EventErrorFlag errorFlag) { m_errorFlag |= errorFlag; }

    /** Get error flag
     *
     *  @return Error flag for this event.
     */
    unsigned int getErrorFlag() const { return m_errorFlag; }

    /** Event Getter.
     *
     *  @return The number of the current event.
     */
    unsigned int getEvent() const { return m_event; }

    /** Run Getter.
     *
     *  @return The number of the current run.
     */
    int getRun() const { return m_run; }

    /** Sub run Getter.
     *
     *  @return Sub-run number, increases indicate recovery from DAQ-internal trouble without
     *          change to detector constants. Not supposed to be used by offline analysis.
     */
    int getSubrun() const { return m_subrun; }

    /** Experiment Getter.
     *
     *  @return The number of the current experiment.
     */
    int getExperiment() const { return m_experiment; }

    /** Production Getter.
     *
     *  @return The number of the current production.
     */
    int getProduction() const { return m_production; }

    /** Time Getter.
     *
     *  @return The time when the event was recorded.
     */
    unsigned long long int getTime() const { return m_time; }

    /** Return LFN of the current parent file, or an empty string if not set.
     */
    const std::string& getParentLfn() const { return m_parentLfn; }

    /** Generated Weight Getter.
     *
     *  @return The weight produced by the event generator
     */
    double getGeneratedWeight() const { return m_generatedWeight; }

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
    unsigned int m_event; /**< Event number ('normal' data has values > 0).  */
    int m_run; /**< Run number (usually > 0, run-independent MC has run == 0).  */
    int m_subrun; /**< Sub-run number, increases indicate recovery from DAQ-internal trouble without change to detector constants. Not supposed to be used by offline analysis.  */
    int m_experiment; /**< Experiment number. (valid values: [0, 1023], run-independent MC has exp == 0)  */

    int m_production; /**< Unique identifier of the production of the event.  */
    unsigned long long int m_time; /**< Time in ns since epoch (1970-01-01).  */
    std::string m_parentLfn;  /**< LFN of the parent file */
    double m_generatedWeight; /**< Generated weight.  */
    unsigned int m_errorFlag;  /**< Indicator of error conditions during data taking, ORed combination of EventErrorFlag values.  */

    ClassDef(EventMetaData, 4); /**< Store event number, run number, and experiment number. */
  }; //class
} // namespace Belle2
