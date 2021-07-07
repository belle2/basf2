/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <string>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {

    /// Mixin for filters that use Monte Carlo information.
    template<class AFilter>
    class MCSymmetric : public AFilter {

    private:
      /// Type of the super class
      using Super = AFilter;

    public:
      /// Constructor
      explicit MCSymmetric(bool allowReverse = true);

      /// Default destructor
      ~MCSymmetric();

      /// Expose the set of parameters of the filter to the module parameter list.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

      /// Initialize the before event processing.
      void initialize() override;

      /// Signal the beginning of a new event
      void beginEvent() override;

      /// Indicates that the filter requires Monte Carlo information.
      bool needsTruthInformation() final;

    public:
      /// Setter for the allow reverse parameter
      virtual void setAllowReverse(bool allowReverse);

      /// Getter for the allow reverse parameter
      bool getAllowReverse() const;

    private:
      /// Switch to indicate if the reversed version of the object shall also be accepted (default is true).
      bool m_param_allowReverse;
    };
  }
}
