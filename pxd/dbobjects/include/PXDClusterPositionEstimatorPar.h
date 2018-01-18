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


namespace Belle2 {


  /** The class for PXD cluster position lookup table payload
   */

  class PXDClusterPositionEstimatorPar: public TObject {
  public:
    /** Default constructor */
    PXDClusterPositionEstimatorPar() {}

    /** Destructor */
    ~ PXDClusterPositionEstimatorPar() {}

    /** Returns grids map */
    const std::map<int, TH2F>& getGrids() const {return m_grids; }

    /** Set shape classifier*/
    void setShapeClassifier(const PXDClusterShapeClassifierPar& classifier, int uBin, int vBin, int pixelkind)
    {
      int key = m_grids[pixelkind].GetBinContent(uBin, vBin);
      m_shapeClassifiers[pixelkind][key] = classifier;
    }

    /** Returns shape classifier */
    const PXDClusterShapeClassifierPar& getShapeClassifier(int uBin, int vBin, int pixelkind)
    {
      int key = m_grids[pixelkind].GetBinContent(uBin, vBin);
      return m_shapeClassifiers[pixelkind][key];
    }

    /** Returns shape classifier for incidence angles and pixelkind */
    PXDClusterShapeClassifierPar& getShapeClassifier(double thetaU, double thetaV, int pixelkind)
    {
      int uBin = m_grids[pixelkind].GetXaxis()->FindBin(thetaU);
      int vBin = m_grids[pixelkind].GetYaxis()->FindBin(thetaV);
      int key = m_grids[pixelkind].GetBinContent(uBin, vBin);
      return m_shapeClassifiers[pixelkind][key];
    }

    /** Returns True if there are valid position corrections available */
    bool hasOffset(int shape_index, int feature_index, double thetaU, double thetaV, int pixelkind)
    {
      //Check pixelkind is valid
      if (m_grids.find(pixelkind) == m_grids.end())
        return false;

      // Check thetaU, thetaV are inside grid
      int uBin = m_grids[pixelkind].GetXaxis()->FindBin(thetaU);
      int vBin = m_grids[pixelkind].GetYaxis()->FindBin(thetaV);
      int uBins = m_grids[pixelkind].GetXaxis()->GetNbins();
      int vBins = m_grids[pixelkind].GetYaxis()->GetNbins();
      if ((uBin < 1) || (uBin > uBins) || (vBin < 1) || (vBin > vBins))
        return false;

      // Check index is valid
      const PXDClusterShapeClassifierPar& classifier = getShapeClassifier(uBin, vBin, pixelkind);
      if (not classifier.hasOffset(shape_index))
        return false;

      return true;
    }

    /**
    Returns hit data for label. Hit is implemented as tuple (offset, cov, prob)
    offset: position corrrection for shape, relative to center of pixel (uStart,vStart) in cluster
    cov   : covariance matrix for offsets
    prob  : probability for observing shape, relative to training data for shape classifier
    */
    PXDClusterOffsetPar& getOffset(int shape_index, int feature_index, double thetaU, double thetaV, int pixelkind)
    {
      PXDClusterShapeClassifierPar& classifier = getShapeClassifier(thetaU, thetaV, pixelkind);
      return classifier.getOffset(shape_index, feature_index);
    }

  private:

    /** Map of angular grids for all pixelkinds  */
    std::map<int, TH2F> m_grids;
    /** Map of cluster shape classifiers */
    std::map<int, std::vector<PXDClusterShapeClassifierPar> > m_shapeClassifiers;

    ClassDef(PXDClusterPositionEstimatorPar, 1);   /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2
