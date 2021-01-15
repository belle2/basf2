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

  /** template class for the APV Histograms*/
  template < class H > // H is an histogram
  class SVDAPVHistograms: public TObject {

  public:
    /** Default constructor*/
    SVDAPVHistograms():
      SVDAPVHistograms(H()) {};

    /** Use @param templateAPV to initialize all the histograms*/
    explicit SVDAPVHistograms(const H& templateAPV);

    /** clean everything in the destructor */
    ~SVDAPVHistograms() { clean(); };

    /** This enumeration assure the same semantic of the
    isU methods defined by Peter Kv.*/
    enum E_side { VIndex = 0 , UIndex = 1 };

    /** get a reference to the histogram for @param vxdID side @param view and @param apv
     * please, use the enumeration SVDAPVHistograms::Vindex and
     * SVDAPVHistograms::UIndex */
    H* getHistogram(const VxdID& vxdID, int view, int apv)
    {
      H* returnValue = m_defaultHistogram;
      try {
        auto layer = m_histograms.at(vxdID.getLayerNumber());
        auto ladder = layer.at(vxdID.getLadderNumber());
        auto sensor = ladder.at(vxdID.getSensorNumber());
        auto theView = sensor.at(view);
        returnValue = theView.at(apv);
      } catch (...) {
        B2WARNING("Unexpected VxdID/view/apv. VxdID: " << (std::string)(vxdID)
                  << "  view : " << view
                  << "  apv : " << apv);

        returnValue = m_defaultHistogram;
      }

      return returnValue;
    }

    // variable number of arguments for TH1 TH2...
    /** fill the histogram for
     * @param vxdID side
     * @param view
     * @param apv
     * @param args value to be filled*/
    template< class ... Types>
    void fill(const VxdID& vxdID, int view, int apv, Types ... args)
    {
      getHistogram(vxdID, view, apv)->Fill(args...);
    }

    // variable number of arguments for TH1 TH2...
    /** fill the histogram for
     * @param vxdID side
     * @param isU
     * @param apv
     * @param args value to be filled*/
    template< class ... Types>
    void fill(const VxdID& vxdID, bool isU, int apv, Types ... args)
    {
      int view = isU ? UIndex : VIndex;
      getHistogram(vxdID, view, apv)->Fill(args...);
    }

    /** replaces layer ladder sensor view and apv with the current numbers*/
    void customizeString(std::string& base, const VxdID& vxdID, bool isU, int user_apv)
    {
      std::string layer  = std::to_string(vxdID.getLayerNumber());
      std::string ladder = std::to_string(vxdID.getLadderNumber());
      std::string sensor = std::to_string(vxdID.getSensorNumber());
      std::string apv = std::to_string(user_apv);
      std::string view = isU ? "U" : "V" ;
      base = std::regex_replace(base, std::regex("[@]layer") , layer);
      base = std::regex_replace(base, std::regex("[@]ladder"), ladder);
      base = std::regex_replace(base, std::regex("[@]sensor"), sensor);
      base = std::regex_replace(base, std::regex("[@]view")  , view);
      std::string side = isU ? "P" : "N" ;
      base = std::regex_replace(base, std::regex("[@]side")  , side);
      base = std::regex_replace(base, std::regex("[@]apv")  , apv);
    }

    /** delete pointers*/
    void clean()
    {

      for (auto layer : m_histograms)
        for (auto ladder : layer)
          for (auto sensor : ladder)
            for (auto view : sensor)
              for (auto apv : view)
                delete apv;
    }




  private:
    /** A t_SVDView is a vector of H that will have length = # APV chips.
     * Index 0 for the V side, index 1 for the U side
     * Please, please, pleaseeeee use SVDAPVHistograms<...>::UIndex
     * and SVDAPVHistograms<...>::VIndex instead of  1 and 0 for better
     * code readibility
     */
    typedef std::vector< H* > t_Views; /**< a vector of H, length = # APV chips*/

    typedef std::vector< t_Views > t_SVDSensor; /**< a vector of vector of H, length = 2 */

    /** A t_SVDLadder is a vector of t_SVDSensors */
    typedef std::vector< t_SVDSensor > t_SVDLadder; /**< a vector of vector of vector of H, length = # svd sensors */

    /** A t_SVDLayer is a vector of t_SVDLadders*/
    typedef std::vector< t_SVDLadder > t_SVDLayer; /**< a vector of vector of vector of vector of H, length = # ladders*/

    /** The t_SVD is a vector of t_SVDLayers*/
    typedef std::vector< t_SVDLayer > t_SVD; /**< a vector of vector of vector of vector of vector of H, length = # layers*/

    t_SVD m_histograms; /**< the vector of vector ... that contains all histograms */
    H* m_defaultHistogram; /**< the default histogram */

    void customize(H& histogram, VxdID vxdID, int view, int apv); /**< customize the histogram with the sensor, view and APV numbers*/

    ClassDef(SVDAPVHistograms , 1); /**< needed by root*/
  };

  /** constructor, builds all histograms and customize them*/
  template <class H>
  SVDAPVHistograms<H>::SVDAPVHistograms(const H& templateAPV)
  {
    m_defaultHistogram = new H(templateAPV);

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
            int nAPV = 6;
            unsigned int viewNumber = 3;

            if (m_histograms[layerNumber][ladderNumber][view].size() < viewNumber)
              m_histograms[layerNumber][ladderNumber][sensorNumber].resize(viewNumber);
            m_histograms[layerNumber][ladderNumber][sensorNumber][view].resize(nAPV);
            for (int apv = 0; apv < nAPV; apv ++) {

              H h = templateAPV;
              customize(h , sensor, view, apv);
              m_histograms[layerNumber][ladderNumber][sensorNumber][view][apv] = new H(h);
            }
          }
        }
      }
    }
  }

  /** customize the histogram with the sensor, view and APV numbers*/
  template < class H >
  void SVDAPVHistograms<H>::customize(H& histogram, VxdID vxdID, int view, int apv)
  {
    bool isU = view == UIndex;
    std::string name = histogram.GetName();
    customizeString(name, vxdID, isU, apv);
    histogram.SetName(name.c_str());

    std::string title = histogram.GetTitle();
    customizeString(title, vxdID, isU, apv);
    histogram.SetTitle(title.c_str());

    std::string xAxis = histogram.GetXaxis()->GetTitle();
    customizeString(xAxis, vxdID, isU, apv);
    histogram.SetXTitle(xAxis.c_str());

    std::string yAxis = histogram.GetYaxis()->GetTitle();
    customizeString(yAxis, vxdID, isU, apv);
    histogram.SetYTitle(yAxis.c_str());

    std::string zAxis = histogram.GetZaxis()->GetTitle();
    customizeString(zAxis, vxdID, isU, apv);
    histogram.SetZTitle(zAxis.c_str());

  }
}
