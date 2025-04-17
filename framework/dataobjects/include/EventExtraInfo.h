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


    /** Return given value if set.
     *
     * throws std::out_of_range if variable is not set.
     */
    std::string getExtraStringInfo(const std::string& name) const;

    /** Return whether the extra string info with the given name is set. */
    bool hasExtraStringInfo(const std::string& name) const;


    /** Removes extra string info from event */
    void removeExtraStringInfo();

    /** Sets the user-defined data of given name to the given value.
     *
     * throws std::out_of_range if variable is already set.
     */
    void addExtraStringInfo(const std::string& name, const std::string& value);

    /** Sets the user-defined data of given name to the given value.
     * Does not throw anything if the value is already set.
     * Overrides existing values
     */
    void setExtraStringInfo(const std::string& name, const std::string& value);

    /** Grab the names in this event extra string info (for printing etc) */
    std::vector<std::string> getStringInfoNames() const;

    /** Add the event type information if it is not already set.
     * Helper function for the GeneratorBaseModule. */
    void addEventTypeIfNotSet(const std::string& eventType)
    {
      /**
       * Event type may already be set if there are multiple generator calls
       * with subsequent selection of generated events by another module.
       */
      if (!hasExtraStringInfo("eventType"))
        addExtraStringInfo(std::string("eventType"), eventType);
    };

    /** Get the event type information.
     * Helper function for the VariablesToNtupleModule*/
    std::string getEventType() const;

  private:
    std::map<std::string, float> eventExtraInfo; /**< map variable names to values. */

    std::map<std::string, std::string> eventExtraStringInfo; /**< map variable names to values for string data type. */

    ClassDef(EventExtraInfo, 3); /**< Class to store event extra info. */
    // v3. Add m_eventType
  };

} // end namespace Belle2

