/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/database/IntraRunDependency.h>
#include <vector>

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

    /**
     * Get a vector with event number boundaries
     * In case of no intra-run dependence, the vector is empty
     * In general for n payloads there are n-1 boundaries
     */
    std::vector<unsigned int> getEventNumbers() const { return m_eventNumbers;}

    /**
     * Get the stored object according to indx
     * @param indx index which can have values from 0 to getEventNumbers() - 1
     */
    TObject* getObjectByIndex(int indx) const {return m_objects.At(indx);}

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
