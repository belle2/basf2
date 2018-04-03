#ifndef STATISTICSTIMINGHLTDQMMODULE_H
#define STATISTICSTIMINGHLTDQMMODULE_H

#include <string>
#include <vector>
#include <map>

#include <TH1F.h>

#include <framework/core/HistoModule.h>

namespace Belle2 {
  namespace SoftwareTrigger {

    /*! A class definition of an input module for Sequential ROOT I/O */

    class StatisticsTimingHLTDQMModule : public HistoModule {

      // Public functions
    public:

      //! Constructor / Destructor
      StatisticsTimingHLTDQMModule();
      virtual ~StatisticsTimingHLTDQMModule();

      //! Module functions to be called from main process
      virtual void initialize();

      //! Module functions to be called from event process
      virtual void event();

      //! Histogram definition
      virtual void defineHisto();


    private:
      /// Directory to put the generated histograms
      std::string m_param_histogramDirectoryName;
      /**mean process time of event*/
      TH1F* h_MeanTime;
      /**mean memory of event*/
      TH1F* h_MeanMem;
      /**process time of event*/
      TH1F* h_EvtTime;
      /**mean process time of modules with large time consumption*/
      TH1F* h_MeanTime_TopModule;
      /**process time of module per event*/
      std::vector<TH1F*> h_ModuleTime;
      /**total process time of modules*/
      std::map<std::string, double> m_sumtime_module;
      /**modules in fastreco*/
      std::vector<std::string> m_fastreco_modules;
      /**modules in physicstrigger*/
      std::vector<std::string> m_physfilter_modules;
      /**modules with large time consumption*/
      std::vector<std::string> m_name_topmodule;
      /**four subgroups: unpacker, fast_reco, physcistrigger, flag samples*/
      const int m_nsubhist = 4;
    };

  } // end namespace SoftwareTrigger
} // end namespace Belle2

#endif // SOFTWARETRIGGERCOSMICRAYDQMMODULE_H
