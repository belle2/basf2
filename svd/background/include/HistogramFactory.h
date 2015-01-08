/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 **************************************************************************/

#ifndef SVD_BG_HISTOGRAMFACTORY
#define SVD_BG_HISTOGRAMFACTORY

#include <TFile.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TH2F.h>
#include <THStack.h>
#include <set>
#include <map>
#include <string>

namespace Belle2 {
  namespace SVD {

    class HistogramFactory {
    public:
      /** BgValue class
       *  This is intended to unify ranges and titles across individual plots.
       */
      struct BgValue {
        /** Constructor takes name, unit and range for the quantity. */
        BgValue(
          std::string name, std::string unit, double rangeLow, double rangeHigh, int nBins):
          m_name(name), m_unit(unit), m_rangeLow(rangeLow), m_rangeHigh(rangeHigh), m_nBins(nBins) {
          m_axisLabel = name + " [" + unit + "]";
        }
        std::string m_name;     /**< full name of the quantity */
        std::string m_unit;     /**< unit for the quantity, such as "Gy/smy" */
        std::string m_axisLabel;/**< label for axis contains name quantity name and unit in [] */
        double m_rangeLow;      /**< low limit of value range */
        double m_rangeHigh;     /**< high limit of value range */
        int m_nBins;            /**< number of bins for value histogram(s) */
      }; // struct BgValue

      /** Constructor
       * takes no parameters.
       */
      HistogramFactory();
      /** Return axis label for the value indentified by short name.
       * The following are legal short names:
       * dose,
       * expo,
       * neutronFlux,
       * protonFlux,
       * pionFlux,
       * electronFlux,
       * photonFlux,
       * neutronFluxNIEL,
       * protonFluxNIEL,
       * pionFluxNIEL,
       * electronFluxNIEL,
       * NIELFlux,
       * firedU,
       * firedV,
       * occupancyU,
       * occupancyV
       * @param valueName short name of the value (see above)
       * @return string containing value name and [unit].
       */
      const std::string& getAxisLabel(const std::string& valueName);
      /** Return a plot title for the value
       * @param valueName short name of the value
       * @return full name of the quantity
       */
      const std::string& getTitle(const std::string& valueName);
      /** Return plot range for the value
       * @param valueName short name of the value
       * @return std::pair with low and high limits of the range.
       */
      std::pair<double, double> getRange(const std::string& valueName);
      /** Return number of bins for the value
       * @param valueName short name of the value
       * @return number of bins for the value
       */
      int getNBins(const std::string& valueName);
      /** Make a bar plot (bars = layers) for a background component
       * @param componentName _full_ name of the beam background component, as in component_colors
       * @param valueName _short_ name of the quantity to plot, as listed with getAxisLabel().
       * @return pointer to a TH1F histogram set to make a bar chart.
       */
      TH1F* MakeBarPlot(const std::string& componentName, const std::string& valueName);
      /** Plot stacked bar plot of a quantity for bar plots stored in a ROOT file.
       * This makes a complete TCanvas with legends, titles etc.
       * @param f source TFile object
       * @param componentNames set of full beam background component names
       * @param valueName _short_ name of quantity to plot, as listed with getValueLabel().
       * @return TCanvas with the stacked bar plot. Transfers ownership.
       */
      TCanvas* PlotStackedBars(TFile* f, const std::set<std::string>& componentNames, const std::string& valueName);
      /** Make a histogram of value valueName for a beam background component componentName
       * @param componentName _full_ name of the beam background component, as in component_colors
       * @param valueName _short_ name of the quantity to plot, as listed with getAxisLabel().
       * @return pointer to a TH1F histogram set to make a value histogram.
       */
      TH1F* MakeFluencePlot(const std::string& comp_name, const std::string& value_name, int layer);
    private:
      std::map<std::string, BgValue> m_valueDescriptions;
      std::map<std::string, int> component_colors = {
        {"Coulomb LER", 41},  // latte
        {"Coulomb HER", 42},
        {"Touschek LER", 45}, // reddish
        {"Touschek HER", 46},
        {"RBB LER", 29},      // greenish
        {"RBB HER", 30},
        {"twoPhoton", 40},      // blue
        {"SR LER", 25},      // grayish
        {"SR HER", 28}
      };
      const char* c_layerlabels[4] = {"3", "4", "5", "6"};
      const float c_barwidth = 0.4;

    }; // class HistogramFactory

  } // namespace SVD
} // namespace Belle2

#endif
