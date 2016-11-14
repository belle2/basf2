/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Petr Katrenko                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMRAWPACKER_H
#define EKLMRAWPACKER_H

/* C++ headers. */
#include <map>

/* Belle2 headers. */
#include <framework/core/Module.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <rawdata/dataobjects/RawKLM.h>

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class EKLMRawPackerModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    EKLMRawPackerModule();
    virtual ~EKLMRawPackerModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void event();


  private:
    //! check data contents
    //    virtual void checkData(RawDataBlock* raw_datablk, unsigned int* eve_copper_0);

    //    StoreArray<RawFTSW> raw_ftswarray;

    //! use default elect id, if not found in mapping file
    bool m_useDefaultElectId = true;

    //! # of events in a run
    int max_nevt;

    //! Number of events
    int n_basf2evt;

    //! Event Meta Data
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    //! RawKLM array
    StoreArray<RawKLM> rawklmarray;

    /**
     * Creation of raw data.
     * @param[in]  endcap Endcap number.
     * @param[in]  layer  Layer number.
     * @param[in]  sector Sector number.
     * @param[in]  plane  Plane number.
     * @param[in]  strip  Strip number.
     * @param[in]  charge Charge.
     * @param[in]  ctime  Time.
     * @param[out] bword1 First word.
     * @param[out] bword2 Second word.
     * @param[out] bword3 Third word.
     * @param[out] bword4 Fourth word.
     */
    void formatData(int endcap, int layer, int sector, int plane, int strip,
                    int charge, float ctime, unsigned short& bword1,
                    unsigned short& bword2, unsigned short& bword3,
                    unsigned short& bword4);

    //! to map logical coordinates to hardware coordinates
    std::map<int, int> m_ModuleIdToelectId;
    //! fill m_ModuleIdToelectId from xml file
    void loadMap();
  };

} // end namespace Belle2

#endif // MODULEHELLO_H
