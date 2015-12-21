/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/logging/Logger.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

#include <map>
#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Mixin for filters that use Monte Carlo information.
    template<class AFilter>
    class MCSymmetricFilterMixin : public AFilter {

    private:
      /// Type of the super class
      typedef AFilter Super;

    public:
      /// Constructor
      explicit MCSymmetricFilterMixin(bool allowReverse = true) :
        Super(),
        m_param_allowReverse(allowReverse)
      {
      }

      /// Initialize the before event processing.
      virtual void initialize() override
      {
        if (needsTruthInformation()) {
          CDCMCManager::getInstance().requireTruthInformation();
        }
      }

      /// Signal the beginning of a new event
      virtual void beginEvent() override
      {
        if (needsTruthInformation()) {
          CDCMCManager::getInstance().fill();
        }
      }

      /** Set the parameter with key to value.
       *
       *  Parameters are:
       *  symmetric -  Accept the object if the reverse version of the object
       *               is correct preserving the progagation reversal symmetry.
       *               Allowed values "true", "false". Default is "true".
       */
      virtual
      void setParameter(const std::string& key, const std::string& value) override
      {
        if (key == "symmetric") {
          if (value == "true") {
            setAllowReverse(true);
            B2INFO("Filter received parameter '" << key << "' " << value);
          } else if (value == "false") {
            setAllowReverse(false);
            B2INFO("Filter received parameter '" << key << "' " << value);
          } else {
            Super::setParameter(key, value);
          }
        } else {
          Super::setParameter(key, value);
        }
      }

      /** Returns a map of keys to descriptions describing the individual parameters of the filter.
       */
      virtual
      std::map<std::string, std::string> getParameterDescription() override
      {
        std::map<std::string, std::string> des = Super::getParameterDescription();
        des["symmetric"] =
          "Indication whether the reverse to the truth is accepted"
          "preserving the progagation reversal symmetry."
          "Allowed values 'true', 'false'. Default is 'true'.";
        return des;
      }

      /// Indicates that the filter requires Monte Carlo information.
      virtual bool needsTruthInformation() override final
      { return true; }

    public:
      /// Setter for the allow reverse parameter
      virtual void setAllowReverse(bool allowReverse)
      { m_param_allowReverse = allowReverse; }

      /// Getter for the allow reverse parameter
      bool getAllowReverse() const
      { return m_param_allowReverse; }

    private:
      /// Switch to indicate if the reversed version of the object shall also be accepted (default is true).
      bool m_param_allowReverse;

    }; // end class MCFilterMixin

  } //end namespace TrackFindingCDC
} //end namespace Belle2
