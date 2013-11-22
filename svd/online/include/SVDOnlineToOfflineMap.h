/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FADC_APV_MAPPER_H_
#define FADC_APV_MAPPER_H_

#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "svd/dataobjects/SVDDigit.h"
#include "svd/dataobjects/SVDDigit.h"

#include <string>
#include <vector>

using namespace std;
using boost::property_tree::ptree;

namespace Belle2 {
  /** This class implement the methods to map raw SVD hits to BASF2 SVD hits.
   * Raw SVD hits are identified by: FADC number, APV number, strip number
   * Basf2 SVD hits are identified by PXD Sensor-id, side, strip number
   */

  class SVDOnlineToOfflineMap {
  public:
    /** Constructor
     * @param xml_filename is the name of the xml file containing the map.
     */
    SVDOnlineToOfflineMap(const std::string& xml_filename);

    /** Return a pointer to a new SVDDigit whose VxdID, isU and cellID is set
     * @param FADC is FADC number from the SVDRawCopper data.
     * @param APV25 is the APV25 number from the SVDRawCopper data.
     * @param channel is the APV25 channel number from the SVDRawCopper data.
     * @return a pointer to the new SVDDigit owned by the caller whose Time,
     * Position and Charge=0
     */
    inline SVDDigit* NewDigit(const unsigned char FADC,
                              const unsigned char APV25,
                              const unsigned char channel);


  private:
    /** Default constructor is private since no one is allowed to use it
     */
    SVDOnlineToOfflineMap();

    /** Read from the ptree v in the xml file the layer nLayer
     */
    void ReadLayer(int nLayer,  ptree const& xml_layer);

    /** Read from the ptree xml_ladde the ladder nLadder in layer nLayer
     */
    void ReadLadder(int nLayer, int nLadder, ptree const& xml_ladder);

    /** Read from the ptree xml_sensor the sensor nSensor in ladder nLadder in layer nLayer
     */
    void ReadSensor(int nLayer, int nLadder, int nSensor, ptree const& xml_sensor);

    /** Read from the ptree xml_side the U-side, if isU, (the V-side otherwise) of the sensor
     *  nSensor in ladder nLadder in layer nLayer
     */
    void ReadSensorSide(int nLayer, int nLadder, int nSensor, bool isU, ptree const& xml_side);

    /** m_chipIsInTheDAQ[ FADC ] & ( 1<< APV25) tells us if the APV25 on FADC
     *  is in the map
     */
    vector< unsigned long long int >  m_chipIsInTheDAQ;

    /** m_VxdID[ FADC ][ APV25 ] gives the VxdId
     * of the sensor read by the APV25 attached to the FADC
     */
    vector< vector <VxdID> >          m_VxdID;


    /** m_isOnUside[ FADC ] & ( 1<< APV25) tells us if the APV25 on FADC
     *  reads the U side of the sensor
     */
    vector< unsigned long long int >  m_isOnUside;

    /** m_channel0Strip[ FADC ][ APV25 ] gives us the strip number
     *  attached to the channel0 of the APV25 attached to the FADC
     */
    vector< vector <unsigned short> > m_channel0Strip;

    /** m_parallel[ FADC ] & ( 1<< APV25) tells us if the APV25 channel
     *  number increases with the sensor strip number
     */
    vector< unsigned long long int >  m_parallel;


    /** add FADCn to the map
     */
    void addFADC(unsigned char  FADCn);

    /** add chipN on FADCn to the map
     */
    void addChip(unsigned char  chipN,
                 unsigned char  FADCn,
                 int nlayer, int nladder, int nsensor, bool isU,
                 unsigned short stripNumberCh0,
                 bool           isParallel);

    void addChip(unsigned char  chipN,
                 unsigned char  FADCn,
                 unsigned short stripNumberCh0,
                 bool           isParallel
                );


  };

  inline SVDDigit*
  SVDOnlineToOfflineMap::NewDigit(const unsigned char FADC,
                                  const unsigned char APV25,
                                  const unsigned char channel)
  {
    // Put some control here
    return new SVDDigit(m_VxdID [FADC][APV25] ,
                        (m_isOnUside [FADC] & (1 << APV25)) != 0 ,
                        m_channel0Strip [FADC][APV25] +
                        ((short) channel)
                        * (
                          (m_parallel[FADC] & (1 << APV25)) ?
                          1 : -1),
                        0., 0., 0.);
  }

}
#endif

