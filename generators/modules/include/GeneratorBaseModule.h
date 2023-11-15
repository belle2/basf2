/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/dataobjects/EventExtraInfo.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Const.h>

#include <string>
#include <unordered_map>

namespace Belle2 {

  class Module;

  /** The base module for generator modules to set the generator information on the EventExtraInfo.
   */
  class GeneratorBaseModule : public Module {
  public:
    /**
     * Constructor.
     */
    GeneratorBaseModule() {};

    /** Method is called for each event. */
    void event() override
    {
      if (m_generatorName == "")
        B2FATAL("Generator name must be set!");

      generatorEvent();

      if (not m_eventExtraInfo.isValid())
        m_eventExtraInfo.create();

      m_eventExtraInfo->addExtraInfo(m_generatorName, 1.0);

      m_eventExtraInfo->addExtraInfo("eventType", getEventType());

      for (auto [name, val] : m_generatorInfoMap)
        m_eventExtraInfo->addExtraInfo(name, val);
    };

    /** Method is called for each event. To be defined in each generator module.*/
    virtual void generatorEvent() {};

    /** Set the generator name  */
    void setGeneratorName(std::string generatorName) {m_generatorName = generatorName;};

    /** Set the generator information  */
    void setGeneratorInfoMap(std::unordered_map<std::string, double> generatorInfoMap)
    {
      m_generatorInfoMap = generatorInfoMap;
    };

    /** Convert m_eventType from string to int */
    virtual double getEventType() const
    {
      return Const::doubleNaN;
    }

  protected:
    /** pointer to EventExtraInfo  */
    StoreObjPtr<EventExtraInfo> m_eventExtraInfo;

    /** Generator name to be added as an extraInfo with value of 1.0 */
    std::string m_generatorName = "";
    /** Generator information to be set on extraInfo */
    std::unordered_map<std::string, double> m_generatorInfoMap = {};
    /** Event type */
    std::string m_eventType = "";

  };

} // end namespace Belle2

