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
        TH2F h(*m_defaultHistogram);

        customize(h, view);
        m_histos[view] = new TH2F(h);
      }
    }


    /** clean everything in the destructor */
    ~SVDSummaryPlots() { clean(); };

    /** This enumeration assure the same semantic of the
    isU methods defined by Peter Kv.*/
    enum E_side { VIndex = 0 , UIndex = 1 };

    /** get a reference to the histogram for @param vxdID side @param view
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

      int bin = m_histos[view]->FindBin(ladder, findBinY(layer, sensor));
      return getHistogram(view)->GetBinContent(bin);
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

      getHistogram(view)->Fill(ladder, findBinY(layer, sensor), value);

    }

    /** fill the histogram for @param vxdID side @param isU with @param value*/
    void fill(const VxdID& vxdID, bool isU, float value)
    {
      int view = isU ? UIndex : VIndex;

      int layer = vxdID.getLayerNumber();
      int ladder = vxdID.getLadderNumber();
      int sensor = vxdID.getSensorNumber();

      getHistogram(view)->Fill(ladder, findBinY(layer, sensor), value);
    }

    /** replaces layer ladder sensor view and apv with the current numbers*/
    void customizeString(std::string& base, bool isU)
    {
      std::string view = isU ? "U" : "V" ;
      base = std::regex_replace(base, std::regex("[@]view")  , view);
      std::string side = isU ? "P" : "N" ;
      base = std::regex_replace(base, std::regex("[@]side")  , side);
    }

    /** delete pointers*/
    void clean()
    {
      delete m_histos[0];
      delete m_histos[1];
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

    TH2F* m_defaultHistogram; /**< default histograms*/

    /** customize the histogram with the sensor, view*/
    void customize(TH2F& histogram, int view)
    {

      const int nY = 19;
      TString Ylabels[nY] = {"", "L3.x.1", "L3.x.2",
                             "", "L4.x.1", "L4.x.2", "L4.x.3",
                             "", "L5.x.1", "L5.x.2", "L5.x.3", "L5.x.4",
                             "", "L6.x.1", "L6.x.2", "L6.x.3", "L6.x.4", "L6.x.5", ""
                            };

      histogram.SetMarkerSize(1.1);
      histogram.GetXaxis()->SetTitle("ladder number");
      histogram.GetXaxis()->SetLabelSize(0.04);
      for (unsigned short i = 0; i < nY; i++)
        histogram.GetYaxis()->SetBinLabel(i + 1, Ylabels[i].Data());

      bool isU = view == UIndex;
      std::string name = histogram.GetName();
      customizeString(name, isU);
      histogram.SetName(name.c_str());

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


    ClassDef(SVDSummaryPlots , 1); /**< needed by root */
  };


}
