/**************************************************************************
 * Belle II detector background library                                   *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EVENTRANDOMIZERMODULE_H
#define EVENTRANDOMIZERMODULE_H

#include <framework/core/Module.h>

#include <TRandom.h>


namespace Belle2 {

  /**
   * Class RandomPermutation: provide a random permutation of integers 0..n-1.
   *
   * This class is a generator: when created with a given size n, it provides (via
   * RandomPermutation::Next) randomized integers from 0 to n-1.
   */
  class RandomPermutation {
  public:
    RandomPermutation(int n) : m_n(n), m_current(n), m_finished(false) {
      m_data = new int[n];
      for (int i = 0; i < n; ++i) m_data[i] = i;
    }
    ~RandomPermutation() { delete [] m_data; }
    int getNext() { // On end, must first return the last element and then -1 to terminate RootInput normally.
      if (m_finished) return -1;
      if (m_current < 0) {
        m_finished = true;
        return m_n - 1;
      }
      int iSelect = gRandom->Integer(m_current--);
      int result = m_data[m_current];
      if (iSelect < m_current) {
        result = m_data[iSelect];
        m_data[iSelect] = m_data[m_current];
      }
      return result;
    }
    bool isFinished() const { return m_finished; }

  private:
    int m_n;              /**< Size of the array.*/
    int* m_data;          /**< Array of indices.*/
    int m_current;        /**< Current position in the array of indices.*/
    bool m_finished;      /**< Indicates that the permuation has been exhausted.*/
  };

  /**
   * The EventRandomizer module.
   *
   * The module makes the framework read events from an input file in random order.
   * The module is intended for randomization of beam background simulation data
   * for the production of timed background readout frames.
   */
  class EventRandomizerModule : public Module {

  public:

    /** Constructor. */
    EventRandomizerModule();

    /** Destructor. */
    virtual ~EventRandomizerModule() {}

    /** Initializes the module. */
    virtual void initialize();

    /** Method is called for each event. */
    virtual void event();

    /** Terminates the module. */
    virtual void terminate();


  protected:

    //Variables
    RandomPermutation* m_selector;               /**< The class that says which record of the input to read next. */

  }; // class EventRandomizerModule

} // end namespace Belle2


#endif /* EVENTRANDOMIZERMODULE_H */
