/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *

 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once


#include <framework/logging/Logger.h>

#include <list>

#include <utility>      // std::pair
#include <functional> // std::function, std::find

namespace Belle2 {

  /** A small class to store a state which can be observed.
   *
   * Same class used as publisher and subscriber.
   */
  template< typename StateType >
  class ObservableState {
  protected:


    /** contains the state to be observed */
    StateType m_state;


    /** stores observers.
     *
     * for each entry:
     * .first: unique identifier to be able to remove observer afterwards.
     * .second: contain update function of observer/subscriber listening to this observableState */
    std::list< std::pair<unsigned int, std::function<void(StateType)> > > m_observers;


    /** unique counter of collected observers */
    unsigned int m_nObservers;
  public:

    /** ************************* CONSTRUCTORS ************************* */


    /** standard constructor - state not initialized */
    ObservableState() :
      m_nObservers(0) {}


    /** constructor initializing state via given state */
    ObservableState(StateType aState) :
      m_state(aState),
      m_nObservers(0) {}


    /** ************************* OPERATORS ************************* */

    // TODO assignment: type conversion ObservableState <-> StateType, comparison & equality

    /** ************************* MEMBER FUNCTIONS ************************* */


/// getter


    /** simple getter, returns current state */
    StateType get() const { return m_state; }


    /** returns reference to current observers of this state */
    std::list< std::pair<unsigned int, std::function<void(StateType)>>>& getObservers() { return m_observers; }


/// setter


    /** informs subscribers if state has changed */
    void set(StateType newState)
    {
      if (newState == m_state) { return; }

      m_state = newState;

      for (auto& anObserver : m_observers) {
        anObserver.second(newState);
      }
    }


    /** adds and synchronizes new observer for this state.
     *
     * return value is assigned identifier for observing function passed!
     */
    unsigned int addObserver(typename std::function<void(StateType)> newObserver)
    {
      m_nObservers++;
      newObserver(get());
      m_observers.push_back({m_nObservers, newObserver});
      return m_nObservers;
    }


    /** removes given observer if given identifier has been found */
    void removeObserver(unsigned int identifier)
    {
      m_observers.remove_if(
        [&](const std::pair<unsigned int, std::function<void(StateType)>>& entry) -> bool { return entry.first == identifier; }
      );
    }


    /** returns number of listening observers at the moment */
    unsigned int getNObservers() const { return m_observers.size(); }


    /** returns total number of observers ever added */
    unsigned int getNObserversTotal() const { return m_nObservers; }
  };

} // end namespace Belle2
