/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <vxd/dataobjects/VxdID.h>
#include <string>
#include <regex>
#include <TH2F.h>
#include <TProfile.h>

#include <TH2Poly.h>
#include <TMath.h>

namespace Belle2 {

  /** class to summarize SVD quantities per sensor and side*/
  class SVDSummaryPlots: public TObject {

  public:
    /** Default constructor*/
    SVDSummaryPlots():
      SVDSummaryPlots("", "") {}; /**< this is the default constructor */

    /**
     * @param name histogram name
     * @param title histogram title
     */
    SVDSummaryPlots(TString name, TString title)
    {
      m_defaultHistogram = new TH2F(name.Data(), title.Data(),
                                    16, 0.5, 16.5,
                                    19, 0, 19);

      for (int view = VIndex ; view < UIndex + 1; view++) {
        std::string hname = name.Data();
        bool isU = (view == UIndex);
        customizeString(hname, isU);
        m_histos[view] = new TH2F(hname.c_str(), title.Data(),
                                  16, 0.5, 16.5,
                                  19, 0, 19);
        customize(*m_histos[view], view);
        m_Title[view] = m_histos[view]->GetTitle();
      }
    }


    /** clean everything in the destructor */
    ~SVDSummaryPlots() { clean(); };

    /** This enumeration assure the same semantic of the
    isU methods defined by Peter Kv.*/
    enum E_side { VIndex = 0, UIndex = 1 };

    /** get a reference to the histogram for @param view
     * please, use the enumeration SVDSummaryPlots::Vindex and
     * SVDSummaryPlots::UIndex */
    TH2F* getHistogram(int view)
    {
      TH2F* returnValue = m_defaultHistogram;
      try {
        returnValue = m_histos[view];
      } catch (...) {
        B2WARNING("Unexpected view: " << view);

        returnValue = m_defaultHistogram;
      }

      return returnValue;
    }

    /** get the value contained in the corresponding bin, given VxdID and view*/
    float getValue(const VxdID& vxdID, int view)
    {
      int layer = vxdID.getLayerNumber();
      int ladder = vxdID.getLadderNumber();
      int sensor = vxdID.getSensorNumber();

      return getValue(layer, ladder, sensor, view);
    }

    /** get the value contained in the corresponding bin, given
     * @param layer
     * @param ladder
     * @param sensor
     * @param view
     */
    float getValue(int layer, int ladder, int sensor, int view)
    {
      int bin = m_histos[view]->FindBin(ladder, findBinY(layer, sensor));
      return getHistogram(view)->GetBinContent(bin);
    }

    /** fill the histogram for
     * @param layer
     * @param ladder
     * @param sensor
     * @param view with
     * @param value
     */
    void fill(int layer, int ladder, int sensor, int view, float value)
    {
      getHistogram(view)->Fill(ladder, findBinY(layer, sensor), value);
    }

    /** fill the histogram for
     * @param vxdID
     * @param view with @param value*/
    void fill(const VxdID& vxdID, int view, float value)
    {
      int layer = vxdID.getLayerNumber();
      int ladder = vxdID.getLadderNumber();
      int sensor = vxdID.getSensorNumber();

      fill(layer, ladder, sensor, view, value);
    }

    /** set run ids in title
     * @param runID
    */
    void setRunID(const TString& runID)
    {
      for (int view = VIndex ; view < UIndex + 1; view++) {
        // add blank if needed before adding runID
        if (!m_Title[view].EndsWith(" "))
          m_Title[view].Append(" ");

        getHistogram(view)->SetTitle(m_Title[view] + runID);
      }
    }

    /** Reset histograms
     */
    void reset()
    {
      for (int view = VIndex ; view < UIndex + 1; view++)
        getHistogram(view)->Reset();
    }

    /** set histograms stat
     * @param stats
     */
    void setStats(bool stats = true)
    {
      for (int view = VIndex ; view < UIndex + 1; view++)
        getHistogram(view)->SetStats(stats);
    }

    /** fill the histogram for @param vxdID side @param isU with @param value*/
    void fill(const VxdID& vxdID, bool isU, float value)
    {
      int view = isU ? UIndex : VIndex;

      int layer = vxdID.getLayerNumber();
      int ladder = vxdID.getLadderNumber();
      int sensor = vxdID.getSensorNumber();

      fill(layer, ladder, sensor, view, value);
    }

    /** replaces layer ladder sensor view and apv with the current numbers*/
    void customizeString(std::string& base, bool isU)
    {
      std::string view = isU ? "U" : "V" ;
      base = std::regex_replace(base, std::regex("[@]view"), view);
      std::string side = isU ? "P" : "N" ;
      base = std::regex_replace(base, std::regex("[@]side"), side);
    }

    /** Create the TH2Poly version of the plot
     */
    TH2Poly* getPoly(int view, int min = -1111) //-1111 set the minimum dependig on the content
    {
      TH2F* histogram = getHistogram(view);
      TString name = histogram->GetName();
      TString title = histogram->GetTitle();

      double xLow = -210.;
      double xHigh = 210.;
      double yLow = -210.;
      double yHigh = 210.;

      if (m_polyHistos[view] == NULL) {
        m_polyHistos[view] = new TH2Poly(name + "_poly", title, xLow, xHigh, yLow, yHigh);
        generateHistogramBins(m_polyHistos[view]);
        m_polyHistos[view]->GetXaxis()->SetTitle("x");
        m_polyHistos[view]->GetXaxis()->SetLabelSize(0);
        m_polyHistos[view]->GetXaxis()->SetTickLength(0);
        m_polyHistos[view]->GetXaxis()->SetTitleOffset(0.5);

        m_polyHistos[view]->GetYaxis()->SetTitle("y");
        m_polyHistos[view]->GetYaxis()->SetLabelSize(0);
        m_polyHistos[view]->GetYaxis()->SetTickLength(0);
        m_polyHistos[view]->GetYaxis()->SetTitleOffset(0.5);
      }

      m_polyHistos[view]->SetName(name + "poly");
      m_polyHistos[view]->SetTitle(title);

      m_polyHistos[view]->SetMinimum(min);

      const double nLadders[4] = {7, 10, 12, 16}; // per layer
      const double nSensors[4] = {2, 3, 4, 5}; // per ladder

      for (int layer = 0; layer < 4; layer++) {
        for (int ladder = 1; ladder < nLadders[layer] + 1; ladder++) {
          for (int sensor = 1; sensor < nSensors[layer] + 1; sensor++) {
            double value = getValue(layer + 3, ladder, sensor, view);
            int bin = findBinPoly(layer + 3, ladder, sensor);
            m_polyHistos[view]->SetBinContent(bin, value);
          }
        }
      }

      return m_polyHistos[view];
    }

    /** delete pointers*/
    void clean()
    {
      delete m_histos[0];
      delete m_histos[1];
      delete m_defaultHistogram;
    }

  private:

    /** find the Y bin given the layer and sensor number */
    Int_t findBinY(Int_t layer, Int_t sensor)
    {

      if (layer == 3)
        return sensor; //2
      if (layer == 4)
        return 2 + 1 + sensor; //6
      if (layer == 5)
        return 6 + 1 + sensor; // 11
      if (layer == 6)
        return 11 + 1 + sensor; // 17
      else
        return -1;
    }

    TH2F* m_histos[2]; /**< vector containing the U and V histograms*/

    TH2Poly* m_polyHistos[2] = {nullptr}; /**< vector containing the U and V poly-histograms*/

    TH2F* m_defaultHistogram = nullptr; /**< default histograms*/

    TString m_Title[2];    /**< Base title */

    /** customize the histogram with the sensor, view*/
    void customize(TH2F& histogram, int view)
    {

      const int nY = 19;
      TString Ylabels[nY] = {"", "L3.x.1", "L3.x.2",
                             " ", "L4.x.1", "L4.x.2", "L4.x.3",
                             "  ", "L5.x.1", "L5.x.2", "L5.x.3", "L5.x.4",
                             "   ", "L6.x.1", "L6.x.2", "L6.x.3", "L6.x.4", "L6.x.5", "    "
                            };

      histogram.SetMarkerSize(1.1);
      histogram.GetXaxis()->SetTitle("ladder number");
      histogram.GetXaxis()->SetLabelSize(0.04);
      for (unsigned short i = 0; i < nY; i++)
        histogram.GetYaxis()->SetBinLabel(i + 1, Ylabels[i].Data());

      bool isU = view == UIndex;
      std::string title = histogram.GetTitle();
      customizeString(title, isU);
      histogram.SetTitle(title.c_str());

      std::string xAxis = histogram.GetXaxis()->GetTitle();
      customizeString(xAxis, isU);
      histogram.SetXTitle(xAxis.c_str());

      std::string yAxis = histogram.GetYaxis()->GetTitle();
      customizeString(yAxis, isU);
      histogram.SetYTitle(yAxis.c_str());

      std::string zAxis = histogram.GetZaxis()->GetTitle();
      customizeString(zAxis, isU);
      histogram.SetZTitle(zAxis.c_str());

    }

    /** generate histogram bins */
    void generateHistogramBins(TH2Poly* histogram)
    {
      // parameters
      const double rLayer[4] = {40, 70, 110, 160}; // arbitrary units. True radii are {39, 80, 104, 135}; // mm
      const double nLadders[4] = {7, 10, 12, 16}; // per layer
      const double nSensors[4] = {2, 3, 4, 5}; // per ladder
      const double inclination[4] = {-17, -5, -13, -12}; // de.g. Layer inclination with respect to the x-axis
      const double delta[4] = {9, 8, 8, 8}; // arbitrary units. Width of the bins for each sensor

      double pi = TMath::Pi();

      for (int layer = 0; layer < 4; layer ++) {
        for (int ladder = 1; ladder <= nLadders[layer]; ladder++) {
          for (int sensor = 1; sensor <= nSensors[layer]; sensor++) {
            double r = rLayer[layer] + (delta[layer]) * (sensor - 1); //position of the layer + position of the sensor
            double phi = 2 * pi / nLadders[layer]; // angle for each sensor (step)
            double dphiThisPoint = (ladder - 1) * phi - phi / 2 + inclination[layer] * pi /
                                   180.; // position of the edge of the sensor (ladder-1 to start from zero)
            double dphiNextPoint = dphiThisPoint + phi; // position of the next edge of the sensor
            // bin coordinates
            double xr0 = r * TMath::Cos(dphiThisPoint);
            double xr1 = (r + delta[layer]) * TMath::Cos(dphiThisPoint);
            double xr2 = (r + delta[layer]) * TMath::Cos(dphiNextPoint);
            double xr3 = r * TMath::Cos(dphiNextPoint);
            double yr0 = r * TMath::Sin(dphiThisPoint);
            double yr1 = (r + delta[layer]) * TMath::Sin(dphiThisPoint);
            double yr2 = (r + delta[layer]) * TMath::Sin(dphiNextPoint);
            double yr3 = r * TMath::Sin(dphiNextPoint);
            // add bin to the histogram
            double xbin[4] = {xr0, xr1, xr2, xr3};
            double ybin[4] = {yr0, yr1, yr2, yr3};
            histogram->AddBin(4, xbin, ybin);
            // resetting bin coordinates
            for (int k = 0; k < 4; k++) {
              xbin[k] = 0.;
              ybin[k] = 0.;
            }
          }
        }
      }
    }

    /** find the poly-bin given the layer, ladder and sensor number */
    Int_t findBinPoly(Int_t layer, Int_t ladder, Int_t sensor)
    {
      const int initBinPerLayer[4] = {1, 15, 45, 93};

      const double nLadders[4] = {7, 10, 12, 16}; // per layer
      const double nSensors[4] = {2, 3, 4, 5}; // per ladder

      int bin = initBinPerLayer[layer - 3];

      for (int ld = 1; ld <= nLadders[layer - 3]; ld++) {
        bool binFound = false;
        for (int s = 1; s <= nSensors[layer - 3]; s++) {
          if (ld == ladder && s == sensor) binFound = true;
          if (binFound) break;
          bin += 1;
        }
        if (binFound) break;
      }
      return bin;
    }

    ClassDef(SVDSummaryPlots, 3);  /**< needed by root */
  };


}
