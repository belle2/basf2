/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TRGCDCT3DConverterModule.h
// Section  :
// Owner    :
// Email    :
//---------------------------------------------------------------
// Description : TRGCDCT3DConverter Module
//---------------------------------------------------------------
// $Log$
//---------------------------------------------------------------

#ifndef TRGCDCT3DCONVERTER_H
#define TRGCDCT3DCONVERTER_H

#include <string>
#include <vector>

#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include "trg/cdc/dataobjects/TRGCDCT3DUnpackerStore.h"
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <framework/dataobjects/BinnedEventT0.h>
#include "boost/multi_array.hpp"

namespace Belle2 {

  class TRGCDCJSignal;
  class TRGCDCJLUT;
  class TRGCDCJSignalData;

  namespace TRGCDCT3DCONVERTERSPACE {

    /*! A module of TRGCDCT3DConverter */
    class TRGCDCT3DConverterModule : public Module {

    public:

      /** Constructor */
      TRGCDCT3DConverterModule();

      /** Destructor */
      virtual ~TRGCDCT3DConverterModule();

      /** Initilizes TRGCDCT3DConverterModule.*/
      virtual void initialize() override;

      /** Called event by event.*/
      virtual void event() override;

      /** Called when processing ended.*/
      virtual void terminate() override;

      /** Called when new run started.*/
      virtual void beginRun() override;

      /** Called when run ended*/
      virtual void endRun() override;

      /** returns version of TRGCDCT3DConverterModule.*/
      std::string version() const;

      /** converts firmwareResults of TS to a vector*/
      void storeTSFirmwareData(boost::multi_array<double, 4>& tsfInfo);

      /** filters tsfFirmwareInfo to tsfInfo*/
      void filterTSData(boost::multi_array<double, 4>& tsfFirmwareInfo,
                        boost::multi_array<double, 3>& tsfInfo);

      /** adds tsfInfo to TS data store*/
      void addTSDatastore(boost::multi_array<double, 3>& tsfInfo, int isSt = 1);

      /** converts firmwareResults of 2D to a vector*/
      void store2DFirmwareData(boost::multi_array<double, 3>& t2DFirmwareInfo,
                               boost::multi_array<double, 4>& axTsfFirmwareInfo);

      ///** filters t2DFirmwareInfo to t2DInfo*/
      //void filter2DData(boost::multi_array<double, 3>  & t2DFirmwareInfo, boost::multi_array<double, 4> & t2DTsfFirmwareInfo, boost::multi_array<double, 2> & t2DInfo, boost::multi_array<double, 3>  & t2DTsfInfo, std::vector<int> & t2DClks, std::vector<int> & t2DId);

      /** filters t2DFirmwareInfo to t2DInfo*/
      void filter2DData(boost::multi_array<double, 3>& t2DFirmwareInfo,
                        boost::multi_array<double, 4>& t2DTsfFirmwareInfo, boost::multi_array<double, 2>& t2DInfo,
                        boost::multi_array<double, 3>& t2DTsfInfo);

      /** adds t2DInfo to track data store*/
      void add2DDatastore(boost::multi_array<double, 2>& t2DInfo, boost::multi_array<double, 3>& t2DTsfInfo);

      /** converts firmwareResults of 3D to a vector*/
      void store3DFirmwareData(boost::multi_array<double, 3>& t3DFirmwareInfo);

      /** uses fast sim with debug*/
      void store3DFastSimData(boost::multi_array<double, 3>& t3DFirmwareInfo);

      /** uses firm sim with debug*/
      void store3DFirmSimData(boost::multi_array<double, 3>& t3DFirmwareInfo);

      /** filters t3DFirmwareInfo to t3DInfo*/
      void filter3DData(boost::multi_array<double, 3>& t3DFirmwareInfo, boost::multi_array<double, 2>& t3DInfo);

      /** adds t3DInfo to track data store*/
      void add3DDatastore(boost::multi_array<double, 2>& t3DInfo, bool doConvert = 1);

      /** Used for debugging 3D firmware */
      void debug3DFirmware();

      /** converts sl, iWire to continuous TS ID [0,2335]*/
      int toTSID(int iSL, int iWire);

      /** converts to signed value*/
      int toSigned(int value, int nBits);

      /** converts to 2D rho to 3D rho value,  obselete*/
//     int t2DRhoTot3DRho(int value, bool isSigned = 1);

      /** converts to 2D phi to 3D phi value*/
      int t2DPhiTot3DPhi(int phi, int rho);

    protected:
      /** Name of the StoreArray containing the input firmware results. */
      std::string m_firmwareResultCollectionName;
      /** Name of the StoreArray containing the input track segment hits. */
      std::string m_hitCollectionName;
      /** Flag for adding TS to datastore. */
      bool m_addTSToDatastore;
      /** Name of the StoreArray containing the input tracks from the 2D fitter. */
      std::string m_inputCollectionName;
      /** Flag for adding 2D finder to datastore. */
      bool m_add2DFinderToDatastore;
      /** Name of the StoreArray containing the resulting 3D tracks. */
      std::string m_outputCollectionName;
      /** Flag for adding 3D to datastore. */
      bool m_add3DToDatastore;
      /** Mode for fitting with TSIM. */
      unsigned m_fit3DWithTSIM;
      /** name of the event time StoreObjPtr */
      std::string m_EventTimeName;
      /** Flag for adding event time to datastore. */
      bool m_addEventTimeToDatastore;
      /** Switch printing detail information.  */
      unsigned m_isVerbose;



    private:
      /** list of firmware results */
      StoreArray<TRGCDCT3DUnpackerStore> m_firmwareResults;
      /** list of track segment hits */
      StoreArray<CDCTriggerSegmentHit> m_hits;
      /** list of 2D input tracks */
      StoreArray<CDCTriggerTrack> m_tracks2D;
      /** list of 3D output tracks*/
      StoreArray<CDCTriggerTrack> m_tracks3D;
      /** StoreObjPtr containing the event time */
      StoreObjPtr<BinnedEventT0> m_eventTime;

      /** TRGCDCJSignalData for VHDL generation (jbkim's class) */
      Belle2::TRGCDCJSignalData* m_commonData;
      /** TRGCDCJSignal for VHDL generation (jbkim's class) */
      std::map<std::string, TRGCDCJSignal> m_mSignalStorage;
      /** TRGCDCJLUT for VHDL generation (jbkim's class) */
      std::map<std::string, TRGCDCJLUT*> m_mLutStorage;

    };
  }
} // end namespace Belle2
#endif // TRGCDCT3DUNPACKER_H
