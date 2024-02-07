/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

    /** Set the event type information. It is supposed to be set by the GeneratorBaseModule. */
    void setEventType(std::string eventType)
    {
      m_eventType = eventType;
    };

    /** Get the event type information. */
    std::string getEventType() const
    {
      return m_eventType;
    };


  private:
    std::map<std::string, float> eventExtraInfo; /**< map variable names to values. */

    std::string m_eventType = ""; /**< even type information, e.g. mixed, ccbar. */

    ClassDef(EventExtraInfo, 3); /**< Class to store event extra info. */
    // v3. Add m_eventType
  };

} // end namespace Belle2

