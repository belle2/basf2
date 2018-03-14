#ifndef SOFTWARETRIGGERHLTDQMMODULE_H
#define SOFTWARETRIGGERHLTDQMMODULE_H
//+
// File : SoftwareTriggerHLTDQMModule.h
// Description : Module to monitor raw data accumulating histos
//
// Author : Chunhua LI, the University of Melbourne
//          Thomas Hauth
// Date :  4 - March - 2015
//-

#include <string>
#include <vector>
#include <map>

#include <TH1F.h>

#include <framework/core/HistoModule.h>


namespace Belle2 {
  namespace SoftwareTrigger {

    /*! A class definition of an input module for Sequential ROOT I/O */

    class SoftwareTriggerHLTDQMModule : public HistoModule {

      // Public functions
    public:

      //! Constructor / Destructor
      SoftwareTriggerHLTDQMModule();
      virtual ~SoftwareTriggerHLTDQMModule() = default;

      //! Module functions to be called from main process
      virtual void initialize() override;

      //! Module functions to be called from event process
      virtual void event() override;

      //! Histogram definition
      virtual void defineHisto() override;

      // Data members

    private:

      /// contains a list of sw trigger variables that should be plotted
      std::vector<std::string> m_param_triggerVariables;

      /// Base identifier for all variables and cut results reported
      std::string m_param_baseIdentifier = "hlt";

      /// List of cut results to include in the final dqm plots
      std::vector<std::string> m_param_cutIdentifiers;

      /// Directory to put the generated histograms
      std::string m_param_histogramDirectoryName = "softwaretrigger";

      /// contains the histogram holding the final sw trigger decision when
      /// all individual cuts are combined
      TH1F* m_totalResultHistogram;

      /// contains the histograms holding the sw trigger variables plots
      std::map<std::string, TH1F*> m_triggerVariablesHistogram;

      /// contains the histograms holnding the results of the cuts
      std::map<std::string, TH1F*> m_cutResultsHistogram;

    };

  } // end namespace SoftwareTrigger
} // end namespace Belle2

#endif // SOFTWARETRIGGERDQMMODULE_H
