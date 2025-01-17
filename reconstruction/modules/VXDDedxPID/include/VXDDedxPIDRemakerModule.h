/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <reconstruction/dataobjects/VXDDedxTrack.h>
#include <reconstruction/dataobjects/VXDDedxLikelihood.h>
#include <framework/database/DBObjPtr.h>
#include <svd/dbobjects/SVDdEdxPDFs.h>
#include <pxd/dbobjects/PXDdEdxPDFs.h>

namespace Belle2 {

  /**
   * Module that re-makes VXD PID likelihoods by taking dE/dx stored in VXDDedxTracks and lookup table PDF's from DB.
   */
  class VXDDedxPIDRemakerModule : public Module {

  public:

    /** Default constructor */
    VXDDedxPIDRemakerModule();

    /** Destructor */
    virtual ~VXDDedxPIDRemakerModule()
    {}

    /** Initialize the module */
    virtual void initialize() override;

    /** This method is called for each event. All processing of the event takes place in this method. */
    virtual void event() override;

  private:

    /** Check the pdfs for consistency every time they change in the database */
    void checkPDFs();

    // module steering parameters
    bool m_useIndividualHits; /**< use individual hits (true) or truncated mean (false) to determine likelihoods */
    bool m_usePXD; /**< use PXD data for likelihood */
    bool m_useSVD; /**< use SVD data for likelihood */

    // collections
    StoreArray<Track> m_tracks; /**< collection of Tracks */
    StoreArray<VXDDedxTrack> m_dedxTracks; /**< collection of VXDDedxTracks */
    StoreArray<VXDDedxLikelihood> m_dedxLikelihoods; /**< collection of VXDDedxLikelihoods */

    // PDF's for PID
    DBObjPtr<SVDdEdxPDFs> m_SVDDedxPDFs; /**< look-up tables of SVD PDF's */
    DBObjPtr<PXDdEdxPDFs> m_PXDDedxPDFs; /**< look-up tables of PXD PDF's */

  };

} // Belle2 namespace
