/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Eugenio Paoloni, Giulia Casarosa                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 * WARNING! Have the potential to cause addiction, but not always...      *
 **************************************************************************/

#pragma once
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <string>
#include <regex>

namespace Belle2 {

  /** template class for SVd histograms*/
  template < class H > // H is an histogram
  class SVDHistograms: public TObject {

  public:
    /** Default constructor*/
    SVDHistograms():
      SVDHistograms(H(), H(), H() , H()) {}; /**< the class is built with a default histogram for L3 and L456, U and V sides*/

    /** Use @param templateHisto to initialize all the histograms*/
    explicit SVDHistograms(const H& templateHisto):
      SVDHistograms(templateHisto, templateHisto,
                    templateHisto, templateHisto)
    {};

    /** Use templates to initialize all the histograms
     * @param templateU3 for the layer 3 U side
     * @param templateV3 for the layer 3 V side
     * @param templateU456 for the layer 4 5 6 U side
     * @param templateV456 for the layer 4 5 6 V side
     */
    SVDHistograms(const H& templateU3, const H& templateV3,
                  const H& templateU456, const H& templateV456);

    /** clean everything in the destructor */
    ~SVDHistograms() { clean(); };

    /** This enumeration assure the same semantic of the
    isU methods defined by Peter Kv.*/
    enum E_side { VIndex = 0 , UIndex = 1 };

    /** get a reference to the histogram for @param vxdID side @param view
     * please, use the enumeration SVDHistograms::Vindex and
     * SVDHistograms::UIndex */
    H* getHistogram(const VxdID& vxdID, int view)
    {
      H* returnValue = m_defaultHistogram;
      try {
        auto layer = m_histograms.at(vxdID.getLayerNumber());
        auto ladder = layer.at(vxdID.getLadderNumber());
        auto sensor = ladder.at(vxdID.getSensorNumber());
        returnValue = sensor.at(view);
      } catch (...) {
        B2WARNING("Unexpected VxdID /view. VxdID: " << (std::string)(vxdID)
                  << "  view : " << view);

        returnValue = m_defaultHistogram;
      }

      return returnValue;
    }

    // variable number of arguments for TH1 TH2...
    /** fill the histogram for @param vxdID side @param view with @param args*/
    template< class ... Types>
    void fill(const VxdID& vxdID, int view, Types ... args)
    {
      getHistogram(vxdID, view)->Fill(args...);
    }

    // variable number of arguments for TH1 TH2...
    /** fill the histogram for @param vxdID side @param isU with @param args*/
    template< class ... Types>
    void fill(const VxdID& vxdID, bool isU, Types ... args)
    {
      int view = isU ? UIndex : VIndex;
      getHistogram(vxdID, view)->Fill(args...);
    }

    /** replaces layer ladder sensor view and apv with the current numbers*/
    void customizeString(std::string& base, const VxdID& vxdID, bool isU)
    {
      std::string layer  = std::to_string(vxdID.getLayerNumber());
      std::string ladder = std::to_string(vxdID.getLadderNumber());
      std::string sensor = std::to_string(vxdID.getSensorNumber());
      std::string view = isU ? "U" : "V" ;
      base = std::regex_replace(base, std::regex("[@]layer") , layer);
      base = std::regex_replace(base, std::regex("[@]ladder"), ladder);
      base = std::regex_replace(base, std::regex("[@]sensor"), sensor);
      base = std::regex_replace(base, std::regex("[@]view")  , view);
      std::string side = isU ? "P" : "N" ;
      base = std::regex_replace(base, std::regex("[@]side")  , side);
    }

    /** delete pointers*/
    void clean()
    {

      for (auto layer : m_histograms)
        for (auto ladder : layer)
          for (auto sensor : ladder)
            for (auto view : sensor)
              delete view;
    }




  private:
    // A t_SVDSensor is a vector of H that will have length 2.
    // Index 0 for the V side, index 1 for the U side
    // Please, please, pleaseeeee use SVDHistograms<...>::UIndex
    // and SVDHistograms<...>::VIndex instead of  1 and 0 for better
    // code readibility
    typedef std::vector< H* > t_SVDSensor;  /**< a vector of H, length = 2 */

    // A t_SVDLadder is a vector of t_SVDSensors
    typedef std::vector< t_SVDSensor > t_SVDLadder; /**< a vector of vector of H, length = # svd sensors */

    // A t_SVDLayer is a vector of t_SVDLAdders
    typedef std::vector< t_SVDLadder > t_SVDLayer; /**< a vector of vector of vector of H, length = # ladders*/

    // The t_SVD is a vector of t_SVDLayers
    typedef std::vector< t_SVDLayer > t_SVD; /**< a vector of vector of vector of vector of H, length = # layers*/

    t_SVD m_histograms; /**< the vector of vector ... that contains all histograms */
    H* m_defaultHistogram; /**< the default histogram */


    void customize(H& histogram, VxdID vxdID, int view); /**< customize the histogram with the sensor, view*/

    ClassDef(SVDHistograms , 1); /**< needed by root*/
  };

  /** constructor, builds all histograms and customize them*/
  template <class H>
  SVDHistograms<H>::SVDHistograms(const H& templateU3, const H& templateV3,
                                  const H& templateU456, const H& templateV456)
  {
    m_defaultHistogram = new H(templateU3);

    VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();
    for (auto layer : geoCache.getLayers(VXD::SensorInfoBase::SVD)) {
      unsigned int layerNumber = layer.getLayerNumber();
      if (m_histograms.size() <= layerNumber)
        m_histograms.resize(layerNumber + 1);

      for (auto ladder : geoCache.getLadders(layer)) {
        unsigned int ladderNumber = ladder.getLadderNumber();
        if (m_histograms[layerNumber].size() <= ladderNumber)
          m_histograms[layerNumber].resize(ladderNumber + 1);

        for (Belle2::VxdID sensor :  geoCache.getSensors(ladder)) {
          unsigned int sensorNumber = sensor.getSensorNumber();
          if (m_histograms[layerNumber][ladderNumber].size() <= sensorNumber)
            m_histograms[layerNumber][ladderNumber].resize(sensorNumber + 1);
          m_histograms[layerNumber][ladderNumber][sensorNumber].resize(2);

          for (int view = VIndex ; view < UIndex + 1; view++) {
            H h = layerNumber == 3 && view == UIndex ? templateU3 :
                  layerNumber == 3 && view == VIndex ? templateV3 :
                  view == UIndex ? templateU456 : templateV456 ;
            customize(h , sensor, view);
            m_histograms[layerNumber][ladderNumber][sensorNumber][view] = new H(h);
          }
        }
      }
    }
  }

  /** customize the histogram with the sensor and view */
  template < class H >
  void SVDHistograms<H>::customize(H& histogram, VxdID vxdID, int view)
  {
    bool isU = view == UIndex;
    std::string name = histogram.GetName();
    customizeString(name, vxdID, isU);
    histogram.SetName(name.c_str());

    std::string title = histogram.GetTitle();
    customizeString(title, vxdID, isU);
    histogram.SetTitle(title.c_str());

    std::string xAxis = histogram.GetXaxis()->GetTitle();
    customizeString(xAxis, vxdID, isU);
    histogram.SetXTitle(xAxis.c_str());

    std::string yAxis = histogram.GetYaxis()->GetTitle();
    customizeString(yAxis, vxdID, isU);
    histogram.SetYTitle(yAxis.c_str());

    std::string zAxis = histogram.GetZaxis()->GetTitle();
    customizeString(zAxis, vxdID, isU);
    histogram.SetZTitle(zAxis.c_str());

  }
}
