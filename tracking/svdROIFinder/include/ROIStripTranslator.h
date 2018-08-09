/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/SVDIntercept.h>
#include <tracking/dataobjects/ROIid.h>
#include <tracking/pxdDataReductionClasses/ROIinfo.h>

namespace Belle2 {

  /** Translator for ROI-geometry-information into a list of pixels.
   */
  class ROIStripTranslator {

  public:

    /** Constructor.
     */
    ROIStripTranslator(const ROIinfo*  theROIinfo);

    /** Second Constructor.
     */
    ROIStripTranslator(double sigmaSystU, double sigmaSystV,
                       double numSigmaTotU, double numSigmaTotV,
                       double maxWidthU, double maxWidthV);


    /** Destructor.
     */
    ~ROIStripTranslator();

    /** Append the ROIid to the list listToBeFilled.
     */
    void fillRoiIDList(StoreArray<SVDIntercept>* listOfIntercepts,
                       StoreArray<ROIid>* ROIidList);

  protected:
    double  m_sigmaSystU; /**< systematic error along the U coordinate*/
    double  m_sigmaSystV; /**< systematic error along the V coordinate*/
    double  m_numSigmaTotU; /**< number of standard deviations to be taken to define a ROI along U*/
    double  m_numSigmaTotV; /**< number of standard deviations to be taken to define a ROI along V*/
    double  m_maxWidthU; /**< ROI maximum width allowed in the U direction*/
    double  m_maxWidthV; /**< ROI maximum width allowed in the V direction*/
  };
}
