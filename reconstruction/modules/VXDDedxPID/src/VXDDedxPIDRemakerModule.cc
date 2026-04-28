/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/modules/VXDDedxPID/VXDDedxPIDRemakerModule.h>
#include <reconstruction/dataobjects/DedxConstants.h>

#include <mdst/dataobjects/Track.h>
#include <reconstruction/dataobjects/VXDDedxTrack.h>
#include <reconstruction/dataobjects/VXDDedxLikelihood.h>
#include <svd/dbobjects/SVDdEdxPDFs.h>
#include <pxd/dbobjects/PXDdEdxPDFs.h>

namespace Belle2 {

  using namespace Dedx;

  //-----------------------------------------------------------------
  ///                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(VXDDedxPIDRemaker);

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  VXDDedxPIDRemakerModule::VXDDedxPIDRemakerModule() : Module()
  {
    setPropertyFlags(c_ParallelProcessingCertified);

    // Set module description
    setDescription("Module that re-makes VXD PID likelihoods by taking dE/dx stored in VXDDedxTracks and lookup table PDF's from DB.");

    // Parameter definitions
    addParam("useIndividualHits", m_useIndividualHits,
             "Use individual hits (true) or truncated mean (false) to determine likelihoods", false);
    addParam("usePXD", m_usePXD, "Use dE/dx from PXD", false);
    addParam("useSVD", m_useSVD, "Use dE/dx from SVD", true);
  }

  void VXDDedxPIDRemakerModule::checkPDFs()
  {
    if (m_usePXD) {
      if (not m_PXDDedxPDFs) B2FATAL("No PXD dE/dx PDF's available");
      bool ok = m_PXDDedxPDFs->checkPDFs(not m_useIndividualHits);
      if (not ok) B2FATAL("Binning or ranges of PXD dE/dx PDF's differ");
    }
    if (m_useSVD) {
      if (not m_SVDDedxPDFs) B2FATAL("No SVD Dedx PDF's available");
      bool ok = m_SVDDedxPDFs->checkPDFs(not m_useIndividualHits);
      if (not ok) B2FATAL("Binning or ranges of SVD dE/dx PDF's differ");
    }
  }

  void VXDDedxPIDRemakerModule::initialize()
  {
    // required inputs
    m_tracks.isRequired();
    m_dedxTracks.isRequired();

    // register likelihoods
    m_dedxLikelihoods.registerInDataStore();
    m_tracks.registerRelationTo(m_dedxLikelihoods);

    // check PDF's and add callback
    m_SVDDedxPDFs.addCallback([this]() {checkPDFs();});
    m_PXDDedxPDFs.addCallback([this]() {checkPDFs();});
    checkPDFs();
  }

  void VXDDedxPIDRemakerModule::event()
  {
    m_dedxLikelihoods.clear();

    for (const auto& track : m_tracks) {
      auto* dedxTrack = track.getRelatedTo<VXDDedxTrack>();
      if (not dedxTrack) continue;

      // re-calculate log likelihoods
      dedxTrack->clearLogLikelihoods();
      bool truncated = not m_useIndividualHits;
      if (m_usePXD) dedxTrack->addLogLikelihoods(m_PXDDedxPDFs->getPDFs(truncated), Dedx::c_PXD, truncated);
      if (m_useSVD) dedxTrack->addLogLikelihoods(m_SVDDedxPDFs->getPDFs(truncated), Dedx::c_SVD, truncated);

      // save log likelihoods
      if (dedxTrack->areLogLikelihoodsAvailable()) {
        auto* likelihoodObj = m_dedxLikelihoods.appendNew(dedxTrack->getLogLikelihoods());
        track.addRelationTo(likelihoodObj);
      }
    }

  }

} // Belle2 namespace

