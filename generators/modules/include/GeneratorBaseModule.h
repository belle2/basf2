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

  /** The base module for generator modules, which sets the generator information as EventExtraInfo.
   */
  class GeneratorBaseModule : public Module {
  public:
    /**
     * Constructor.
     */
    GeneratorBaseModule()
    {
      //Generator common parameters
      addParam("eventType", m_eventType, "Event type", std::string(""));
    };

    /** Initialize the module */
    void initialize() override
    {
      m_eventExtraInfo.isRequired();

      generatorInitialize();
    };

    /** Initialize the module. To be defined in each generator module. */
    virtual void generatorInitialize() = 0;

    /** Method is called for each event. */
    void event() override
    {
      generatorEvent();

      if (not m_eventExtraInfo.isValid())
        m_eventExtraInfo.create();

      m_eventExtraInfo->addEventTypeIfNotSet(getEventType());

      for (auto [name, val] : m_generatorInfoMap)
        m_eventExtraInfo->addExtraInfo(name, val);
    };

    /** Method is called for each event. To be defined in each generator module.*/
    virtual void generatorEvent() = 0;

    /** Set the generator information  */
    void setGeneratorInfoMap(std::unordered_map<std::string, double> generatorInfoMap)
    {
      m_generatorInfoMap = generatorInfoMap;
    };

    /** Return m_eventType. */
    virtual std::string getEventType() const
    {
      return m_eventType;
    }

  protected:
    /** pointer to EventExtraInfo  */
    StoreObjPtr<EventExtraInfo> m_eventExtraInfo;

    /** Event type */
    std::string m_eventType;
    /** Generator information to be set as extraInfo */
    std::unordered_map<std::string, double> m_generatorInfoMap = {};

  };

} // end namespace Belle2

