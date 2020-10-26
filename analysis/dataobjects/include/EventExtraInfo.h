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

    /** Return whether the extra info with the given name is set. */
    bool hasExtraInfo(const std::string& name) const;


    /** Removes extra info from event */
    void removeExtraInfo();

    /** Sets the user-defined data of given name to the given value.
     *
     * throws std::out_of_range if variable is already set.
     */
    void addExtraInfo(const std::string& name, float value);

    /** Sets the user-defined data of given name to the given value.
     * Does not throw anything if the value is already set.
     * Overrides existing values
     */
    void setExtraInfo(const std::string& name, float value);

    /** Return a short summary of this object's contents in HTML format. */
    std::string getInfoHTML() const;

    /** Grab the names in this event extra info (for printing etc) */
    std::vector<std::string> getNames() const;

  private:
    std::map<std::string, float> eventExtraInfo; /**< map variable names to values. */

    ClassDef(EventExtraInfo, 2); /**< Class to store event extra info. */
  };

} // end namespace Belle2

