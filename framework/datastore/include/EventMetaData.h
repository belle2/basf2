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

    /** Constructor.
     *
     *  Initilises meta data of event with Nulls.
     */
    EventMetaData(const unsigned long& event = 0, const unsigned long& run = 0, const unsigned long& experiment = 0)
        : m_event(event), m_run(run), m_experiment(experiment) {}

    /** Destructor. */
    ~EventMetaData() {}

    /** Event Setter.
     *
     *  @param event The current event number.
     */
    void setEvent(const unsigned long& event) {
      m_event = event;
    }

    /** Run Setter.
     *
     *  @param run The number of the current run.
     */
    void setRun(const unsigned long& run) {
      m_run = run;
    }

    /** Experiment Setter.
     *
     *  @param experiment The number of the current experiment.
     */
    void setExperiment(const unsigned long& experiment) {
      m_experiment = experiment;
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

    /** ROOT Macro to make EventMetaData a ROOT class.
     */
    ClassDef(EventMetaData, 1);

  }; //class
} // namespace Belle2
#endif // EVENTMETADATA
