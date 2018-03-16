/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Andreas Moll                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

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
     *  Initializes meta data of event with Nulls.
     */
    ProfileInfo(unsigned long virtualMemory = 0, unsigned long rssMemory = 0, double timeInSec = 0)
      : m_virtualMemory(virtualMemory), m_rssMemory(rssMemory), m_timeInSec(timeInSec) {}

    /** Obtain the current profile values.
     */
    void set(double timeOffset = 0);

    /** Virtual Memory size getter.
     *
     *  @return The virtual memory size in kB.
     */
    unsigned long getVirtualMemory() const
    {
      return m_virtualMemory;
    }

    /** Rss Memory size getter.
     *
     *  @return The Rss memory size in kB.
     */
    unsigned long getRssMemory() const
    {
      return m_rssMemory;
    }

    /** Time getter.
     *
     *  @return The time in seconds.
     */
    double getTimeInSec() const
    {
      return m_timeInSec;
    }

  private:

    /** Virtual Memory size in kB.
     */
    unsigned long m_virtualMemory;

    /** Resident Memory size in kB.
     */
    unsigned long m_rssMemory;

    /** Time used by this process in seconds.
     */
    double m_timeInSec;

    ClassDef(ProfileInfo, 2); /**< Store execution time and memory usage. */

  }; //class
} // namespace Belle2
