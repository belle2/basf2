/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <map>

#include <TObject.h>

namespace Belle2 {

  /**
   * Class to stores ExtraInfo of the whole event.
   * Used by TMVAExpert to save expert output for a event-level training
   */
  class EventExtraInfo : public TObject {

  public:
    /** Return given value if set.
     *
     * throws std::out_of_range if variable is not set.
     */
    float getExtraInfo(const std::string& name) const;

    /** Return wether the extra info with the given name is set. */
    bool hasExtraInfo(const std::string& name) const;

    /** Sets the user-defined data of given name to the given value.
     *
     * throws std::out_of_range if variable is already set.
     */
    void addExtraInfo(const std::string& name, float value);

  private:
    std::map<std::string, float> eventExtraInfo; /**< map variable names to values. */

    ClassDef(EventExtraInfo, 2); /**< Class to store event extra info. */
  };

} // end namespace Belle2

