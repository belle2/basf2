/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/vtx/filters/results/BaseVTXResultFilter.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.dcl.h>

namespace Belle2 {
  /// Factory that can create appropriate cluster filters from associated names.
  class VTXResultFilterFactory : public TrackFindingCDC::FilterFactory<BaseVTXResultFilter> {

  private:
    /// Type of the base class
    using Super = TrackFindingCDC::FilterFactory<BaseVTXResultFilter>;

  public:
    /// Constructor forwarding the default filter name
    explicit VTXResultFilterFactory(const std::string& defaultFilterName = "all");

    /// Default destructor
    ~VTXResultFilterFactory();

    /// Getter for a short identifier for the factory
    std::string getIdentifier() const override;

    /// Getter for a descriptive purpose of the constructed filters
    std::string getFilterPurpose() const override;

    /// Getter for valid filter names and a description for each
    std::map<std::string, std::string> getValidFilterNamesAndDescriptions() const override;

    /// Create a filter with the given name.
    std::unique_ptr<BaseVTXResultFilter> create(const std::string& filterName) const override;
  };
}