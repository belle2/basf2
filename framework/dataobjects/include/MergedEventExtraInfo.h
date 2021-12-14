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
   * TODO: This could be a temporary solution if someone has a better idea....
   * I would probably move the EventExtraInfo from analysis to framework - it is anyways used all over our software
   * The whole DataStore merging business takes place in the framework package.
   * We want to be able to skip the merging of events if their charge is not consistent.
   * For that reason we have to somehow save this information on a 'framework' level
   * but the information about the charge is usually saved on the analysis level (EventExtraInfo).
   */
  class MergedEventExtraInfo : public TObject {

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
    std::map<std::string, float> m_mergedEventExtraInfo; /**< map variable names to values. */

    ClassDef(MergedEventExtraInfo, 1); /**< Class to store event extra info. */
  };

} // end namespace Belle2

