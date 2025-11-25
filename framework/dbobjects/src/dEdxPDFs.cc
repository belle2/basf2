
/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/dbobjects/dEdxPDFs.h>

namespace Belle2 {

  const std::vector<const TH2F*>& dEdxPDFs::getPDFs(bool truncated) const
  {
    auto& cachePDFs = truncated ? m_cachePDFsTruncated : m_cachePDFs;
    if (cachePDFs.empty()) {
      const auto& PDFs = truncated ? m_dEdxPDFsTruncated : m_dEdxPDFs;
      for (const auto& pdf : PDFs) cachePDFs.push_back(&pdf);
    }
    return cachePDFs;
  }


  bool dEdxPDFs::checkPDFs(bool truncated) const
  {
    const auto& PDFs = truncated ? m_dEdxPDFsTruncated : m_dEdxPDFs;

    int nx = PDFs[0].GetNbinsX();
    int ny = PDFs[0].GetNbinsY();
    double xmin = PDFs[0].GetXaxis()->GetXmin();
    double xmax = PDFs[0].GetXaxis()->GetXmax();
    double ymin = PDFs[0].GetYaxis()->GetXmin();
    double ymax = PDFs[0].GetYaxis()->GetXmax();

    for (const auto& h : PDFs) {
      if (h.GetNbinsX() != nx) return false;
      if (h.GetNbinsY() != ny) return false;
      if (h.GetXaxis()->GetXmin() != xmin) return false;
      if (h.GetXaxis()->GetXmax() != xmax) return false;
      if (h.GetYaxis()->GetXmin() != ymin) return false;
      if (h.GetYaxis()->GetXmax() != ymax) return false;
    }

    return true;
  }

} //Belle2 namespace
