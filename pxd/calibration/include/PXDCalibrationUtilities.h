/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Qingyuan Liu                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <vector>
#include <TH1I.h>

namespace Belle2 {
  namespace PXD {

    /** Helper function to extract number of bins along u side and v side from counter histogram labels. */
    void getNumberOfBins(const std::shared_ptr<TH1I>& histo_ptr, unsigned short& nBinsU, unsigned short& nBinsV);

    /** Helper function to extract number of sensors from counter histogram labels. */
    unsigned short getNumberOfSensors(const std::shared_ptr<TH1I>& histo_ptr);

    /** Helper function to calculate a median from unsorted signal vector. The input vector gets sorted. */
    double CalculateMedian(std::vector<double>& signals);

    /** Helper function to calculate a median from 1D histogram. */
    double CalculateMedian(TH1* hist);

    /** Helper function to estimate MPV from 1D histogram. */
    double FitLandau(TH1* hist);

    /** Helper function to calculate MPV from a vector. The input vector gets sorted. */
    double FitLandau(std::vector<double>& signals);

  }// namespace PXD
} // namespace Belle2
