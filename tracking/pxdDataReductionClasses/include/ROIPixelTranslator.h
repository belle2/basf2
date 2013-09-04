/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ROIPIXELTRANSLATOR_H_
#define ROIPIXELTRANSLATOR_H_

#include <framework/datastore/RelationArray.h>
#include <tracking/dataobjects/PXDIntercept.h>
#include <tracking/dataobjects/ROIid.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  /**
   * This Class translates the information of the ROI geometry into a list of pixels.
   */
  class ROIPixelTranslator {

  public:

    /**
     * Constructor.
     */

    ROIPixelTranslator(double sigmaSystU, double sigmaSystV,
                       double numSigmaTotU, double numSigmaTotV,
                       double maxWidthU, double maxWidthV);


    /**
     * Destructor.
     */
    ~ROIPixelTranslator();

    /**
     * Append the ROIid to the list listToBeFilled
     */
    void fillRoiIDList(StoreArray<PXDIntercept>* listOfIntercepts,
                       StoreArray<ROIid>* ROIidList);



  protected:
    double  m_sigmaSystU;
    double  m_sigmaSystV;
    double  m_numSigmaTotU;
    double  m_numSigmaTotV;
    double  m_maxWidthU;
    double  m_maxWidthV;


  };




}


#endif


