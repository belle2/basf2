/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventT0.h>

#include <TFile.h>
#include <TH1F.h>
#include <TH1D.h>
#include <TEfficiency.h>
#include <TDirectory.h>

namespace Belle2 {

  /**
   * This module to design collect the event t0 values base on different detectors and physics processes.
   */

  class EventT0ValidationModule : public Module {

  public:

    /** Default constructor */
    EventT0ValidationModule();

    /** Destructor */
    virtual ~EventT0ValidationModule();

    /** Initialize the module */
    virtual void initialize() override;

    /** This method is called for each run */
    virtual void beginRun() override;

    /** This method is called for each event. All processing of the event
     * takes place in this method. */
    virtual void event() override;

    /** This method is called for each run */
    virtual void endRun() override;

  private:

    StoreObjPtr<EventT0> m_eventT0 ;  /**< Store array for event t0 */

    /// Name of the output ROOT file
    std::string m_RootFileName = "EventT0Validation.root";

    /// Output ROOT file
    TFile* m_outputFile;

    /// ECL based EventT0 histograms
    TH1F* m_histECLEventT0{nullptr};
    /// SVD based EventT0 histograms
    TH1F* m_histSVDEventT0{nullptr};
    /// TOP based EventT0 histograms
    TH1F* m_histTOPEventT0{nullptr};
    /// CDC based EventT0 histograms
    TH1F* m_histCDCEventT0{nullptr};
    /// CDC hit based EventT0 histograms
    TH1F* m_histCDCHitBasedEventT0{nullptr};
    /// CDC chi2 based EventT0 histograms
    TH1F* m_histCDCChi2EventT0{nullptr};
    /// CDC grid EventT0 histograms
    TH1F* m_histCDCGridEventT0{nullptr};

    /// EventT0 algorithms for which to calculate fractions of abundance
    const char* c_eventT0Algorithms[11] = {"Any", "ECL", "SVD", "TOP", "CDC (all)", "CDC hit based (all)", "CDC hit based (active)",
                                           "CDC full grid #chi^{2} (all)", "CDC full grid #chi^{2} (active)", "CDC grid (all)", "CDC grid (active)"
                                          };
    /// Count of events with EventT0 from a given algorithm, numerator for efficiency calculation
    TH1D* m_histAlgorithmSourceCounts{nullptr};
    /// Counts of events with EventT0 from a given algorithm, denominator for efficiency calculation
    TH1D* m_histAlgorithmSourceCountsActive{nullptr};

    /// Set additional plot info: contact, description, shifter
    void setPlotMetaData(TH1* hist, const std::string& description, const std::string& check,
                         const std::string& contact, const std::string& shifter = "shifter");

    /// For simplicity, just set the contact once
    std::string m_contact = "christian.wessel@desy.de";
  };

} // Belle2 namespace
