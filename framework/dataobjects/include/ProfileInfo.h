/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Andreas Moll                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef PROFILEINFO_H
#define PROFILEINFO_H

#include <TObject.h>

namespace Belle2 {
  /** Store execution time and memory usage.
   *
   *  Class for execution time and memory usage information.
   */
  class ProfileInfo : public TObject {
  public:

    /** Constructor.
     *
     *  Initilises meta data of event with Nulls.
     */
    ProfileInfo(unsigned long memory = 0, double timeInSec = 0)
      : m_memory(memory), m_timeInSec(timeInSec) {}

    /** Obtain the current profile values.
     */
    void set(double timeOffset = 0);

    /** Memory size getter.
     *
     *  @return The memory size in kB.
     */
    unsigned long getMemory() const {
      return m_memory;
    }

    /** Time getter.
     *
     *  @return The time in seconds.
     */
    double getTimeInSec() const {
      return m_timeInSec;
    }

  private:

    /** Memory size in kB.
     */
    unsigned long m_memory;

    /** Time used by this process in seconds.
     */
    double m_timeInSec;

    ClassDef(ProfileInfo, 1); /**< Store execution time and memory usage. */

  }; //class
} // namespace Belle2
#endif
