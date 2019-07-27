/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/pcore/EvtMessage.h>
#include <TH1.h>

#include <string>
#include <memory>
#include <map>

namespace Belle2 {
  /**
   * Utility to store received histograms (hierarchical tree structures) from clients (as an event message),
   * with a function to add multiple histogram trees together.
   * Can be converted back into a single event message and is used in the histogram
   * server for merging the received histogram messages.
   * Internally, the tree structure is stored as a mapping name -> TH1 (unique) pointer,
   * where folder structure is mapped via "/" in the name.
   */
  class HistogramTree {
  public:
    /// As this is a heavy object, make sure to not copy
    HistogramTree& operator=(const HistogramTree& rhs) = delete;
    /// Moving is allowed
    HistogramTree& operator=(HistogramTree&& rhs) = default;
    /// As this is a heavy object, make sure to not copy
    HistogramTree(const HistogramTree& rhs) = delete;
    /// Moving is allowed
    HistogramTree(HistogramTree&& rhs) = default;
    /// Default constructor needed during summation
    HistogramTree() = default;
    /// Constructor via a received event message by deserializing the histograms
    explicit HistogramTree(std::unique_ptr<Belle2::EvtMessage> msg);

    /// Add another histogramm tree instance by merging all histograms with the same name.
    void operator+=(const HistogramTree& rhs);

    /// Write out all stored histograms in the currently selected ROOT gDirectory
    void write() const;
    /// Clear all histograms in the internal map also deleting the pointers
    void clear();
    /// Debug function to print out the content as into messages.
    void printMe() const;
    /// Construct an EvtMessage by serializing the content of the internal histogram storage. Will not invalidate the histograms.
    std::unique_ptr<Belle2::EvtMessage> toMessage() const;
    /// Check if there are no stored histograms
    bool empty() const;

  private:
    /// Internal storage of the histograms in the form name -> unique TH1 pointer
    std::map<std::string, std::unique_ptr<TH1>> m_histograms;
  };
}