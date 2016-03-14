/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/logging/Logger.h>

#include <map>
#include <vector>
#include <memory>

namespace Belle2 {

  /**
   * This is the base class for all MeasurementCreatorFactories used in the MeasurementCreatorModule.
   * It knows about all MeasurementCreators for a given HitType and can setup the creators from the
   * module parameters.
   * You have to implement the createMeasurementCreatorFromName function - all other functions
   * are already implemented in this base class.
   * See the CDCMeasurementCreatorFactory for an example.
   */

  template <class BaseMeasurementCreatorType>
  class MeasurementCreatorFactory {
  public:
    /** Typedef for convenience */
    typedef BaseMeasurementCreatorType CreatorType;

    /** Use the parameters given to the module and create the measurement creators from them. */
    void initialize()
    {
      for (const auto& creatorWithParameterDictionary : m_creatorsWithParametersDictionary) {
        const std::string& creatorName = creatorWithParameterDictionary.first;
        const std::map<std::string, std::string>& parameterDictionary = creatorWithParameterDictionary.second;

        B2DEBUG(100, "Creating measurement creator with name " << creatorName);

        BaseMeasurementCreatorType* creatorPointer = createMeasurementCreatorFromName(creatorName);
        if (creatorPointer == nullptr) {
          B2FATAL("Can not create a measurement creator with the name " << creatorName << ". Creator not known to the factory.");
        }
        m_measurementCreators.push_back(std::move(std::shared_ptr<BaseMeasurementCreatorType>(creatorPointer)));

        for (const auto& parameterWithValue : parameterDictionary) {
          const std::string& parameterName = parameterWithValue.first;
          const std::string& parameterValue = parameterWithValue.second;
          creatorPointer->setParameter(parameterName, parameterValue);
        }
      }
    }

    /** Overload this method to create the measurement creators by their name. */
    virtual BaseMeasurementCreatorType* createMeasurementCreatorFromName(const std::string& /*creatorName*/) const
    {
      return nullptr;
    }

    /** Return the creators to the module */
    const std::vector<std::shared_ptr<BaseMeasurementCreatorType>>& getCreators() const
    {
      return m_measurementCreators;
    }

    /** Return a reference to the parameters you can use in the module. */
    std::map<std::string, std::map<std::string, std::string>>& getParameters()
    {
      return m_creatorsWithParametersDictionary;
    }

    /** Set the parameters. */
    void setParameters(const std::map<std::string, std::map<std::string, std::string>>& creatorsWithParametersDictionary)
    {
      m_creatorsWithParametersDictionary = std::move(creatorsWithParametersDictionary);
    }

  private:
    /** A vector with the measurement creators. Is filled in initialize. */
    std::vector<std::shared_ptr<BaseMeasurementCreatorType>> m_measurementCreators;

    /** The map of dictionaries of the parameters. Fill it with the module parameters. */
    std::map<std::string, std::map<std::string, std::string>> m_creatorsWithParametersDictionary;
  };
}
