/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <TH2F.h>
#include <map>


#include <pxd/dbobjects/PXDClusterShapeClassifierPar.h>
#include <pxd/dbobjects/PXDClusterOffsetPar.h>

#include <framework/logging/Logger.h>


namespace Belle2 {


  /** The class for PXD cluster position lookup table payload
   */

  class PXDClusterPositionEstimatorPar: public TObject {
  public:
    /** Default constructor */
    PXDClusterPositionEstimatorPar() {}

    /** Destructor */
    ~ PXDClusterPositionEstimatorPar() {}

    /** Add grid for pixelkind */
    void addGrid(int pixelkind, const TH2F& grid)
    {
      m_gridmap[pixelkind] = grid;
      m_shapeClassifiers[pixelkind] = std::vector<PXDClusterShapeClassifierPar>();

      for (auto uBin = 1; uBin <= m_gridmap[pixelkind].GetXaxis()->GetNbins(); uBin++) {
        for (auto vBin = 1; vBin <= m_gridmap[pixelkind].GetYaxis()->GetNbins(); vBin++) {
          auto size = m_shapeClassifiers[pixelkind].size();
          m_gridmap[pixelkind].SetBinContent(uBin, vBin, size);
          m_shapeClassifiers[pixelkind].push_back(PXDClusterShapeClassifierPar());
        }
      }
    }

    /** Return grid map*/
    const std::map<int, TH2F>& getGridMap() const
    {
      return m_gridmap;
    }

    /** Set shape classifier*/
    void setShapeClassifier(const PXDClusterShapeClassifierPar& classifier, int uBin, int vBin, int pixelkind)
    {
      int key = m_gridmap[pixelkind].GetBinContent(uBin, vBin);
      m_shapeClassifiers[pixelkind][key] = classifier;
    }

    /** Returns shape classifier */
    const PXDClusterShapeClassifierPar& getShapeClassifier(int uBin, int vBin, int pixelkind) const
    {
      int key = m_gridmap.at(pixelkind).GetBinContent(uBin, vBin);
      const PXDClusterShapeClassifierPar& classifier = m_shapeClassifiers.at(pixelkind)[key];
      return classifier;
    }

    /** Returns shape classifier for incidence angles and pixelkind */
    const PXDClusterShapeClassifierPar& getShapeClassifier(double thetaU, double thetaV, int pixelkind) const
    {
      auto grid = m_gridmap.at(pixelkind);
      int uBin = grid.GetXaxis()->FindBin(thetaU);
      int vBin = grid.GetYaxis()->FindBin(thetaV);
      int key = grid.GetBinContent(uBin, vBin);
      return m_shapeClassifiers.at(pixelkind)[key];
    }

    /** Returns True if there are valid position corrections available */
    bool hasOffset(int shape_index, int feature_index, double thetaU, double thetaV, int pixelkind) const
    {
      //Check pixelkind is valid
      if (m_gridmap.find(pixelkind) == m_gridmap.end()) {
        return false;
      }

      // Check thetaU, thetaV are inside grid
      auto grid = m_gridmap.at(pixelkind);
      int uBin = grid.GetXaxis()->FindBin(thetaU);
      int vBin = grid.GetYaxis()->FindBin(thetaV);
      int uBins = grid.GetXaxis()->GetNbins();
      int vBins = grid.GetYaxis()->GetNbins();
      if ((uBin < 1) || (uBin > uBins) || (vBin < 1) || (vBin > vBins))
        return false;

      // Check index is valid
      const PXDClusterShapeClassifierPar& classifier = getShapeClassifier(uBin, vBin, pixelkind);
      if (not classifier.hasOffset(shape_index, feature_index))
        return false;

      return true;
    }

    /** Returns correction (offset) for cluster shape relative to center of pixel (startU/startV)*/
    const PXDClusterOffsetPar& getOffset(int shape_index, int feature_index, double thetaU, double thetaV, int pixelkind) const
    {
      const PXDClusterShapeClassifierPar& classifier = getShapeClassifier(thetaU, thetaV, pixelkind);
      return classifier.getOffset(shape_index, feature_index);
    }

  private:

    /** Map of angular grids for different pixelkinds  */
    std::map<int, TH2F> m_gridmap;
    /** Map of cluster shape classifiers for different pixelkinds*/
    std::map<int, std::vector<PXDClusterShapeClassifierPar> > m_shapeClassifiers;

    ClassDef(PXDClusterPositionEstimatorPar, 1);   /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2
