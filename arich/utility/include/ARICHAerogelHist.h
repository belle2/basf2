/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leonid Burmistrov                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

//root
#include <TObject.h>
#include <TH2Poly.h>
#include <TGraph.h>
#include <TVector2.h>

//c, c++
#include <string>
#include <vector>
#include <map>

namespace Belle2 {

  /**
   * Base class for geometry parameters.
   */
  class ARICHAerogelHist: public TH2Poly {

  public:

    /**
     * Default constructor.
     */
    ARICHAerogelHist() {};

    /**
     * Default destructor
     */
    ~ARICHAerogelHist() {};

    /**
     * Constructor with name, title.
     * @param name name.
     * @param title title.
     */
    ARICHAerogelHist(const char* name, const char* title);

    /**
     * Function which return histogram bin id from ring and column id's.
     * @param ring ringID number.
     * @param column columnID number.
     */
    Int_t GetBinIDFromRingColumn(Int_t ring, Int_t column);

    /**
     * Function to draw the histogram.
     * @param opt draw option string default value : "ZCOLOT text same".
     * @param outDirName name of epe and pdf to save the plots.
     */
    void DrawHisto(TString opt, TString outDirName);

  protected:

    /**
     * Function which set initial values of input parameters.
     */
    void SetInitialParametersByDefault();

    /**
     * Function for calculation vertices for one aerogel tile.
     */
    void SetUpVerticesMap();

    /**
     * Function to print vertices for one aerogel tile.
     */
    void dumpVerticesMap();

    /**
     * Function to rotate 2D point (x and y) around z axis by angle phi.
     * @param xold old x coordinate.
     * @param yold old y coordinate.
     * @param xnew new x coordinate.
     * @param ynew new y coordinate.
     * @param phi roration angle.
     */
    void makeRotation(double xold, double yold, double& xnew, double& ynew, double phi);

    std::vector<Int_t> m_nTiles;       /**< Number of tiles per ring. */
    std::vector<double> m_tileRmin;    /**< Minimum radius of aerogel ring. */
    std::vector<double> m_tileRmax;    /**< Maximum radius of aerogel ring. */
    std::vector<double> m_tileRcenter; /**< Center radius of aerogel ring. */
    std::vector<double>
    m_tileDeltaPhiCenter; /**< Angle opening (phi) of the aerogel tile measured between two rays (0.0,0.0 : and centre of the ring from left/right). */
    std::vector<double>
    m_aerogelAriGapDeltaPhiCenter; /**< Angle opening (phi) of the air gap between aerogel tiles. Measured between ray (0.0,0.0 : and centre of the ring from left/right). */
    Int_t m_verboseLevel;    /**< Verbose level. */
    Int_t m_nCircularPoints; /**< Number of circular points. */
    double m_aerogelTileGap; /**< Distance between aerogel tiles. */

    ////Aerogel vertices map Int_t -> aerogel ring number
    std::map<Int_t, std::vector<TVector2>> m_verticesMap; /**< Aerogel vertices map. */

    TString m_histName;  /**< Histogram name. */
    TString m_histTitle; /**< Histogram title. */

    ClassDef(ARICHAerogelHist, 1); /**< ClassDef */

  };

} //end namespace Belle2
