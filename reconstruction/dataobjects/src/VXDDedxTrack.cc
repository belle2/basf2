/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <reconstruction/dataobjects/VXDDedxTrack.h>
#include <framework/logging/Logger.h>
#include <cmath>

namespace Belle2 {

  void VXDDedxTrack::addHit(int sid, int layer, int adcCount, double dx, double dEdx)
  {
    m_sensorID.push_back(sid);
    m_layer.push_back(layer);
    m_adcCount.push_back(adcCount);
    m_dx.push_back(dx);
    m_dEdx.push_back(dEdx);
  }

  void VXDDedxTrack::addDedx(int layer, double distance, double dedxValue)
  {
    dedxLayer.push_back(layer);
    dist.push_back(distance);
    dedx.push_back(dedxValue);
    m_length += distance;
  }

  double VXDDedxTrack::getDedx(Const::EDetector detector) const
  {
    if (detector == Const::PXD) return m_dedxAvgTruncated[Dedx::c_PXD];
    if (detector == Const::SVD) return m_dedxAvgTruncated[Dedx::c_SVD];
    return 0.0;
  }

  double VXDDedxTrack::getDedxError(Const::EDetector detector) const
  {
    if (detector == Const::PXD) return m_dedxAvgTruncatedErr[Dedx::c_PXD];
    if (detector == Const::SVD) return m_dedxAvgTruncatedErr[Dedx::c_SVD];
    return 0.0;
  }

  double VXDDedxTrack::getDedxMean(Const::EDetector detector) const
  {
    if (detector == Const::PXD) return m_dedxAvg[Dedx::c_PXD];
    if (detector == Const::SVD) return m_dedxAvg[Dedx::c_SVD];
    return 0.0;
  }

  void VXDDedxTrack::clearLogLikelihoods()
  {
    for (auto& logl : m_vxdLogl) logl = 0;
    m_numAdded = 0;
  }

  void VXDDedxTrack::addLogLikelihoods(const std::vector<const TH2F*>& PDFs, Dedx::Detector detector, bool truncated)
  {
    if (detector != Dedx::c_PXD and detector != Dedx::c_SVD) return;

    if (truncated) {
      addLogLikelihoods(PDFs, m_dedxAvgTruncated[detector], 1e-7);
    } else {
      for (size_t i = 0; i < dedx.size(); i++) {
        if (detector == Dedx::c_PXD and std::abs(dedxLayer[i]) > 2) continue;
        if (detector == Dedx::c_SVD and std::abs(dedxLayer[i]) < 3) continue;
        addLogLikelihoods(PDFs, dedx[i], 1e-5);
      }
    }

  }

  void VXDDedxTrack::addLogLikelihoods(const std::vector<const TH2F*>& PDFs, double dedxValue, double minPDFValue)
  {
    if (dedxValue <= 0) return;

    int binX = PDFs[0]->GetXaxis()->FindFixBin(m_p);
    int binY = PDFs[0]->GetYaxis()->FindFixBin(dedxValue);
    bool inRange = binX > 0 and binX <= PDFs[0]->GetNbinsX() and binY > 0 and binY <= PDFs[0]->GetNbinsY();
    for (unsigned int iPart = 0; iPart < Const::ChargedStable::c_SetSize; iPart++) {
      double pdfValue = 0;
      const auto& pdf = PDFs[iPart];
      if (inRange) {
        pdfValue = const_cast<TH2F*>(pdf)->Interpolate(m_p, dedxValue);
      } else {
        pdfValue = pdf->GetBinContent(binX, binY);
      }
      if (pdfValue != pdfValue) {
        B2ERROR("pdfValue is NAN for a track with p=" << m_p << " and dedx=" << dedxValue);
        clearLogLikelihoods();
        return;
      }
      if (pdfValue <= 0) pdfValue = minPDFValue;
      m_vxdLogl[iPart] += std::log(pdfValue);
    }

    m_numAdded++;
  }

} // end namespace Belle2
