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

    /** Add grid for clusterkind */
    void addGrid(int clusterkind, const TH2F& grid)
    {
      m_gridmap[clusterkind] = grid;
      m_shapeClassifiers[clusterkind] = std::vector<PXDClusterShapeClassifierPar>();

      for (auto uBin = 1; uBin <= m_gridmap[clusterkind].GetXaxis()->GetNbins(); uBin++) {
        for (auto vBin = 1; vBin <= m_gridmap[clusterkind].GetYaxis()->GetNbins(); vBin++) {
          auto size = m_shapeClassifiers[clusterkind].size();
          m_gridmap[clusterkind].SetBinContent(uBin, vBin, size);
          m_shapeClassifiers[clusterkind].push_back(PXDClusterShapeClassifierPar());
        }
      }
    }

    /** Return grid map*/
    const std::map<int, TH2F>& getGridMap() const
    {
      return m_gridmap;
    }

    /** Set shape classifier*/
    void setShapeClassifier(const PXDClusterShapeClassifierPar& classifier, int uBin, int vBin, int clusterkind)
    {
      int key = m_gridmap[clusterkind].GetBinContent(uBin, vBin);
      m_shapeClassifiers[clusterkind][key] = classifier;
    }

    /** Returns shape classifier */
    const PXDClusterShapeClassifierPar& getShapeClassifier(int uBin, int vBin, int clusterkind) const
    {
      int key = m_gridmap.at(clusterkind).GetBinContent(uBin, vBin);
      const PXDClusterShapeClassifierPar& classifier = m_shapeClassifiers.at(clusterkind)[key];
      return classifier;
    }

    /** Returns shape classifier for incidence angles and clusterkind */
    const PXDClusterShapeClassifierPar& getShapeClassifier(double thetaU, double thetaV, int clusterkind) const
    {
      auto grid = m_gridmap.at(clusterkind);
      int uBin = grid.GetXaxis()->FindBin(thetaU);
      int vBin = grid.GetYaxis()->FindBin(thetaV);
      int key = grid.GetBinContent(uBin, vBin);
      return m_shapeClassifiers.at(clusterkind)[key];
    }

    /** Returns True if there is a classifier available */
    bool hasClassifier(double thetaU, double thetaV, int clusterkind) const
    {
      //Check clusterkind is valid
      if (m_gridmap.find(clusterkind) == m_gridmap.end()) {
        return false;
      }

      // Check thetaU, thetaV are inside grid
      auto grid = m_gridmap.at(clusterkind);
      int uBin = grid.GetXaxis()->FindBin(thetaU);
      int vBin = grid.GetYaxis()->FindBin(thetaV);
      int uBins = grid.GetXaxis()->GetNbins();
      int vBins = grid.GetYaxis()->GetNbins();
      if ((uBin < 1) || (uBin > uBins) || (vBin < 1) || (vBin > vBins))
        return false;

      return true;
    }

    /** Returns correction (offset) for cluster shape relative to center of pixel (startU/startV) if available, otherwise returns nullptr.*/
    const PXDClusterOffsetPar* getOffset(int shape_index, float eta, double thetaU, double thetaV, int clusterkind) const
    {
      //Check if there is a classifier
      if (not hasClassifier(thetaU, thetaV, clusterkind)) {
        return nullptr;
      }
      const PXDClusterShapeClassifierPar& classifier = getShapeClassifier(thetaU, thetaV, clusterkind);
      return classifier.getOffset(shape_index, eta);
    }

    /** Returns shape likelyhood. Returns zero if shape not known, otherwise positive float*/
    float getShapeLikelyhood(int shape_index, double thetaU, double thetaV, int clusterkind) const
    {
      // Return zero if there no classifier
      if (not hasClassifier(thetaU, thetaV, clusterkind)) {
        return 0;
      }
      auto likelyhoodMap = getShapeClassifier(thetaU, thetaV, clusterkind).getShapeLikelyhoodMap();

      // Return zero if no likelyhood was estimated
      auto it = likelyhoodMap.find(shape_index);
      if (it == likelyhoodMap.end())
        return 0;

      return it->second;
    }

  private:

    /** Map of angular grids for different clusterkinds  */
    std::map<int, TH2F> m_gridmap;
    /** Map of cluster shape classifiers for different clusterkinds*/
    std::map<int, std::vector<PXDClusterShapeClassifierPar> > m_shapeClassifiers;

    ClassDef(PXDClusterPositionEstimatorPar, 2);   /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2
