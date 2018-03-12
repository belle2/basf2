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
     * Default constructor
     */
    ARICHAerogelHist() {};

    /**
     * Default destructor
     */
    ~ARICHAerogelHist() {};

    /**
     * Constructor with name, title
     * @param name name
     * @param title title
     */
    ARICHAerogelHist(const char* name, const char* title);

    Int_t GetBinIDFromRingColumn(Int_t ring, Int_t column);

    void DrawHisto(TString opt, TString outDirName);

  protected:

    void SetInitialParametersByDefault();

    void SetUpVerticesMap();

    void dumpVerticesMap();

    void makeRotation(double xold, double yold, double& xnew, double& ynew, double phi);

    //Number of tiles per ring.
    std::vector<Int_t> m_nTiles;
    //Minimum radius of aerogel ring
    std::vector<double> m_tileRmin;
    //Maximum radius of aerogel ring
    std::vector<double> m_tileRmax;
    //Center radius of aerogel ring
    std::vector<double> m_tileRcenter;
    //Angle opening (phi) of the aerogel tile measured between two rays (0.0,0.0 : and centre of the ring from ).
    std::vector<double> m_tileDeltaPhiCenter;
    //Angle opening (phi) of the air gap between aerogel tiles. Measured between ray (0.0,0.0 : and centre of the ring from left/right).
    std::vector<double> m_aerogelAriGapDeltaPhiCenter;
    //Verbose level
    Int_t m_verboseLevel;
    //Number of circular points
    Int_t m_nCircularPoints;
    //Distance between aerogel tiles
    double m_aerogelTileGap;

    ////Aerogel vertices map Int_t -> aerogel ring number
    std::map<Int_t, std::vector<TVector2>> m_verticesMap;

    TString m_histName;
    TString m_histTitle;

    ClassDef(ARICHAerogelHist, 1); /**< ClassDef */

  };

} //end namespace Belle2
