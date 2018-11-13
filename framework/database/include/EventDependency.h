/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/database/IntraRunDependency.h>
#include <vector>

class TClass;

namespace Belle2 {

  /**
   * Class for handling changing conditions as a function of event number.
   */
  class EventDependency: public IntraRunDependency {
  public:

    /**
     * Constructor for event depenent conditions.
     * @param object   the first valid object in the run.
     * @param owner    flag that indicates whether the EventDependency takes ownership of the payload objects or not.
     */
    explicit EventDependency(TObject* object = 0, bool owner = true): IntraRunDependency(object, owner) {};

    /**
     * Add an object to the intra run dependency.
     * Note that the EventDependency object takes ownership of the added object by default.
     * @param event    the event number from which on the given conditions object is valid.
     * @param object   the object which is valid starting from the given event number.
     */
    void add(unsigned int event, TObject* object);

  protected:
    /**
     * Get the index of the object that is valid for the given event.
     * @param event   meta data of the event for which we want to have the conditions.
     * @return        array index of the object valid for the given event.
     */
    virtual int getIndex(const EventMetaData& event) const override;

  private:
    /** Vector of event number boundaries. */
    std::vector<unsigned int> m_eventNumbers;

    ClassDefOverride(EventDependency, 1);  /**< class for event number dependent conditions. */
  };
}
