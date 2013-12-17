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
   *  Master modules have to create an object of this type!
   *
   *  @author <a href="mailto:martin.heck@kit.edu?subject=EventMetaData">Martin Heck</a>
   */
  class EventMetaData : public TObject {
  public:

    /** Constructor. */
    explicit EventMetaData(unsigned long event = 0, unsigned long run = 0, unsigned long experiment = 0);

    /** Destructor. */
    ~EventMetaData() {}

    /** Event Setter.
     *
     *  @param event The current event number.
     */
    void setEvent(unsigned long event) {
      m_event = event;
    }

    /** Run Setter.
     *
     *  @param run The number of the current run.
     */
    void setRun(unsigned long run) {
      m_run = run;
    }

    /** Experiment Setter.
     *
     *  @param experiment The number of the current experiment.
     */
    void setExperiment(unsigned long experiment) {
      m_experiment = experiment;
    }

    /** Marks the end of the data processing.
     *
     * Can be used by non-master modules to halt event processing.
     *
     * Sets the values for the experiment, run and event to
     * values indicating the end of data.
     */
    void setEndOfData();

    /** Parent Index Setter.
     *
     *  @param index The index of the current parent file.
     */
    void setParentIndex(int index) {
      m_parent_index = index;
    }

    /** Generated Weight Setter.
     *
     *  @param weight The weight produced by the event generator
     */
    void setGeneratedWeight(double weight) {
      m_generated_weight = weight;
    }

    /** Event Getter.
     *
     *  @return The number of the current event.
     */
    unsigned long getEvent() const {
      return m_event;
    }

    /** Run Getter.
     *
     *  @return The number of the current run.
     */
    unsigned long getRun() const {
      return m_run;
    }

    /** Experiment Getter.
     *
     *  @return The number of the current experiment.
     */
    unsigned long getExperiment() const {
      return m_experiment;
    }

    /** Parent Index Getter.
     *
     *  @return The index of the current parent file.
     */
    int getParentIndex() const {
      return m_parent_index;
    }

    /** Generated Weight Getter.
     *
     *  @return The weight produced by the event generator
     */
    double getGeneratedWeight() const {
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
    bool operator!= (const EventMetaData& eventMetaData) const;

  private:

    /** Event number.
     */
    unsigned long m_event;

    /** Run number.
     */
    unsigned long m_run;

    /** Experiment number.
     */
    unsigned long m_experiment;

    /** Index of parent file.
     */
    int m_parent_index;

    /** Generated weight.
     */
    double m_generated_weight;

    ClassDef(EventMetaData, 1); /**< Store event number, run number, and experiment number. */
  }; //class
} // namespace Belle2
#endif // EVENTMETADATA
