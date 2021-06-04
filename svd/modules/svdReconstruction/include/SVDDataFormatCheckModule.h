/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef SVDDATAFORMATCHECK_H
#define SVDDATAFORMATCHECK_H


#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <svd/dataobjects/SVDEventInfo.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDDAQDiagnostic.h>

#include <string>

namespace Belle2 {

  /** This module checks the format of the data that we are going to reconstruct
   * checking the SVDModeByte and the SVDDAQDiagnostic
   */

  class SVDDataFormatCheckModule : public Module {
  public:

    /** Constructor defining the parameters */
    SVDDataFormatCheckModule();

    virtual ~SVDDataFormatCheckModule();

    /** Initialize the SVDDataFormatCheck.*/
    virtual void initialize() override;

    /** Called when entering a new run.     */
    virtual void beginRun() override;

    /** This method is the core of the SVDDataFormatCheck. */
    virtual void event() override;

    /** This method is called if the current run ends. */
    virtual void endRun() override;

    /** This method is called at the end of the event processing.   */
    virtual void terminate() override;


  private:

    /** ShaperDigit store array*/
    StoreArray<SVDShaperDigit> m_storeShaper;
    /** Diagnostic store array*/
    StoreArray<SVDDAQDiagnostic> m_storeDAQ;
    /** event meta data store array */
    StoreObjPtr<EventMetaData> m_evtMetaData;

    /**storage for SVDEventInfo object */
    StoreObjPtr<SVDEventInfo> m_storeSVDEvtInfo;

    /** experiment number */
    int m_expNumber = 0;
    /** run number */
    int m_runNumber = 0;

    /** counter of 1-sample events*/
    int m_n1samples = 0;
    /** counter of 3-sample events*/
    int m_n3samples = 0;
    /** counter of 6-sample events*/
    int m_n6samples = 0;
    /** counter of local-run events */
    int m_nLocalRunEvts = 0;
    /** counter of non zero-suppressed events */
    int m_nNoZSEvts = 0;
    /** counter of bad TB events */
    int m_nBadTBEvts = 0;

    /** if true shut up warnings */
    bool m_shutUpWarnings = false;
    /** couter of events */
    int m_evtsCounter = 0;
    /** couter of events with at least one strip */
    int m_stripEvtsCounter = 0;
    /** couter of events with at least one strip with a warning*/
    int m_problematicEvtsCounter = 0;
    /** max number of events with at least one strip with a warning*/
    int m_maxProblematicEvts = 10;
    /** max number of strips in one with at least a warning*/

  protected:

    /** Name of the collection to use for the SVDShaperDigits */
    std::string m_storeShaperDigitsName;
    /** Name of the collection to use for the SVDDAQDiagnostic */
    std::string m_storeDAQName;
    /** Name of the SVDEventInfo object */
    std::string m_svdEventInfoName;


  };
}
#endif


























