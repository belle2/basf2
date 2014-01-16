#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/vxdCaTracking/TrackletFilters.h>
#include <tracking/vxdCaTracking/LittleHelper.h>
#include <tracking/vxdCaTracking/SharedFunctions.h>

// stl:
#include <vector>
#include <math.h>
#include <utility> // pair
#include <fstream>

// root:
#include <TVector3.h>
#include <gtest/gtest.h>
#include <TRandom.h>
#include <TMatrixD.h>

//boost:
// #include <boost/foreach.hpp>
#include <boost/bind/bind.hpp>
// #ifndef __CINT__
// #include <boost/chrono.hpp>
// #endif

//Eigen
#include <Eigen/Dense>

using namespace std;

namespace Belle2 {


  /** command x should exit using B2FATAL. */
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

  /** Producing suitable input data for for the TrackletFilters-class, execute this file typing "test_tracking" */
  class TrackletFiltersTest : public ::testing::Test {
  protected:
  };

  /** TrackletFilters expects a row of hits stored as a vector of TVector3. These should be on a realistic track through the detector, meaning in the x-y-plane, they form a circle passing the origin (or a point near to it). There have to be at least 4 hits for the ZigZag-Test, for the circlefitter it can be 3 hits, but it should be 4 at least. we will test it first with an aequivalent of a 500 MeV (pT) track and another one having only 50 MeV. The reason for having more than one cirle is the fact that the circlefitter seems to be optimized somehow and therefore could have problems with circles formed by a small number of hits or circles with small radii*/
  TEST_F(TrackletFiltersTest, simpleTest)
  {
    TVector3 a, b, c, d, e, f, g, h, i, j, k, l, m; // there are up to 12+1 Hits (one for the primary vertex) possible within the VXD before the particle curls back the first time

    //The hits will be produced using the same radius (which will be smeared) and different phi-values to describe a cirle-segment covering only a part of the outline of the circle. To get a full helix, the z-value of the hits are changed accordingly to their original phi-value

    LittleHelper hBox = LittleHelper(); // smears hits using smearValueGauss(double low, double high, double mean, double sigma)
    double sigma = 0.001, scalePhi = 11.9, scaleZ = 88., t90 = M_PI * 0.5, radius = 11.;
    double aVal = 0.0001, bVal = 0.006, cVal = 0.007, dVal = 0.011, eVal = 0.0124, fVal = 0.018, gVal = 0.0202, hVal = 0.039, iVal = 0.042, jVal = 0.052, kVal = 0.055, lVal = 0.068, mVal = 0.0715;
    vector<TVector3> highPtMini, highPtMaxi; // carry hitPositions;
    vector<PositionInfo> tempMiniStuff, tempMaxiStuff, tempTestStuff; // tempVectors to be able to keep PositionInfo
    vector<PositionInfo*> miniStuff, maxiStuff, testStuff; // carry positionInfo (TrackletFilters expects pointers)

    //phi does not have to be smeared to simulate the realistic situation, some hits will be positioned pretty near next to each other
    a.SetMagThetaPhi(hBox.smearValueGauss(-1111., 1111., radius, sigma), t90, aVal * scalePhi),
                     b.SetMagThetaPhi(hBox.smearValueGauss(-1111., 1111., radius, sigma), t90, bVal * scalePhi), //L1
                     c.SetMagThetaPhi(hBox.smearValueGauss(-1111., 1111., radius, sigma), t90, cVal * scalePhi),
                     d.SetMagThetaPhi(hBox.smearValueGauss(-1111., 1111., radius, sigma), t90, dVal * scalePhi), //L2
                     e.SetMagThetaPhi(hBox.smearValueGauss(-1111., 1111., radius, sigma), t90, eVal * scalePhi),
                     f.SetMagThetaPhi(hBox.smearValueGauss(-1111., 1111., radius, sigma), t90, fVal * scalePhi), //L3
                     g.SetMagThetaPhi(hBox.smearValueGauss(-1111., 1111., radius, sigma), t90, gVal * scalePhi),
                     h.SetMagThetaPhi(hBox.smearValueGauss(-1111., 1111., radius, sigma), t90, hVal * scalePhi), //L4
                     i.SetMagThetaPhi(hBox.smearValueGauss(-1111., 1111., radius, sigma), t90, iVal * scalePhi),
                     j.SetMagThetaPhi(hBox.smearValueGauss(-1111., 1111., radius, sigma), t90, jVal * scalePhi), //L5
                     k.SetMagThetaPhi(hBox.smearValueGauss(-1111., 1111., radius, sigma), t90, kVal * scalePhi),
                     l.SetMagThetaPhi(hBox.smearValueGauss(-1111., 1111., radius, sigma), t90, lVal * scalePhi), //L6
                     m.SetMagThetaPhi(hBox.smearValueGauss(-1111., 1111., radius, sigma), t90, mVal * scalePhi);

    a.SetZ(aVal * scaleZ), b.SetZ(bVal * scaleZ), c.SetZ(cVal * scaleZ), d.SetZ(dVal * scaleZ), e.SetZ(eVal * scaleZ), f.SetZ(fVal * scaleZ), g.SetZ(gVal * scaleZ), h.SetZ(hVal * scaleZ), i.SetZ(iVal * scaleZ), j.SetZ(jVal * scaleZ), k.SetZ(kVal * scaleZ), l.SetZ(lVal * scaleZ), m.SetZ(mVal * scaleZ); // atm not needed, forces hits to lie on a helix in xyz

    //now we have got hits lying on a small part of a circle having its center at the origin. Therefore I have to move the circle so its arch passes the origin
    TVector3 move500 = TVector3(radius, 0.5, 0.);

    a -= move500, b -= move500, c -= move500, d -= move500, e -= move500, f -= move500, g -= move500, h -= move500, i -= move500, j -= move500, k -= move500, l -= move500, m -= move500;


    highPtMini.push_back(l), highPtMini.push_back(g), highPtMini.push_back(f), highPtMini.push_back(a);
    highPtMaxi.push_back(a), highPtMaxi.push_back(b), highPtMaxi.push_back(c), highPtMaxi.push_back(d), highPtMaxi.push_back(e), highPtMaxi.push_back(f), highPtMaxi.push_back(g), highPtMaxi.push_back(h), highPtMaxi.push_back(i), highPtMaxi.push_back(j), highPtMaxi.push_back(k), highPtMaxi.push_back(l), highPtMaxi.push_back(m);


    for (TVector3 & hit : highPtMaxi) {
      PositionInfo posInfo;
      posInfo.hitPosition = hit;
      posInfo.sigmaX = sigma;
      tempMaxiStuff.push_back(posInfo);
    }
    for (PositionInfo & hit : tempMaxiStuff) {
      maxiStuff.push_back(&hit);
    }

    for (TVector3 & hit : highPtMini) {
      PositionInfo posInfo;
      posInfo.hitPosition = hit;
      posInfo.sigmaX = sigma;
      tempMiniStuff.push_back(posInfo);
    }
    for (PositionInfo & hit : tempMiniStuff) {
      miniStuff.push_back(&hit);
    }

    TVector3 testa = TVector3(1 - (1. / sqrt(2.)), 1. / sqrt(2.), 0);
    TVector3 testb = TVector3(1., 1., 0);
    TVector3 testc = TVector3(1 + (1. / sqrt(2.)), 1. / sqrt(2.), 0);
    TVector3 testd = TVector3(2., 0., 0);
    TVector3 teste = TVector3(1., -1., 0);
    TVector3 moveSmall = TVector3(0.1, 0.0, 0.);
    vector<TVector3> testVector;
    testVector.push_back(testa += moveSmall); testVector.push_back(testb += moveSmall); testVector.push_back(testc += moveSmall); testVector.push_back(testd += moveSmall); testVector.push_back(teste += moveSmall);

    for (TVector3 & hit : testVector) {
      PositionInfo posInfo;
      posInfo.hitPosition = hit;
      posInfo.sigmaX = sigma;
      tempTestStuff.push_back(posInfo);
    }
    for (PositionInfo & hit : tempTestStuff) {
      testStuff.push_back(&hit);
    }

    ThreeHitFilters f3h = ThreeHitFilters();

//     B2WARNING(" calcsign mlk, lkj, kji, jih, ihg, hgf, gfe, fed, edc, dcb, cba:" << f3h.calcSign(m, l, k) << "," << f3h.calcSign(l, k, j) << "," << f3h.calcSign(k, j, i) << "," << f3h.calcSign(j, i, h) << "," << f3h.calcSign(i, h, g) << "," << f3h.calcSign(h, g, f) << "," << f3h.calcSign(g, f, e) << "," << f3h.calcSign(f, e, d) << "," << f3h.calcSign(e, d, c) << "," << f3h.calcSign(d, c, b) << "," << f3h.calcSign(c, b, a));
    EXPECT_EQ(f3h.calcSign(l, g, f), f3h.calcSign(g, f, a));

    TrackletFilters aFilter = TrackletFilters();
    double clapPhi = 0, clapR = 0, estimatedRadius = 0;

    aFilter.resetValues(&maxiStuff);
    EXPECT_FALSE(aFilter.ziggZaggXY());
    EXPECT_FALSE(aFilter.ziggZaggRZ());
    double chi2 = aFilter.circleFit(clapPhi, clapR, estimatedRadius);
//     B2WARNING("after maxi-test, chi2 is " << chi2 << ", clapPhi,clapR,estimatedRadius is: " << clapPhi << "," << clapR << "," << estimatedRadius)
    chi2++; // is simply for removing the warning. Has no other meaning since Value is resetted all the time

    aFilter.resetValues(&miniStuff);
    EXPECT_FALSE(aFilter.ziggZaggXY());
    EXPECT_FALSE(aFilter.ziggZaggRZ());
    chi2 = aFilter.circleFit(clapPhi, clapR, estimatedRadius);
//     B2WARNING("after mini-test, chi2 is " << chi2 << " clapPhi,clapR,estimatedRadius is: " << clapPhi << "," << clapR << "," << estimatedRadius)

    aFilter.resetValues(&testStuff);
    EXPECT_FALSE(aFilter.ziggZaggXY());
    EXPECT_FALSE(aFilter.ziggZaggRZ());
    chi2 = aFilter.circleFit(clapPhi, clapR, estimatedRadius);
//     B2WARNING("after testVector-test, chi2 is " << chi2 << " clapPhi,clapR,estimatedRadius is: " << clapPhi << "," << clapR << "," << estimatedRadius)

    /// comparing calcCircleCenter and circleFit: taking 3 hits lying on a circle (by definition) and a 4th one lying there too. If calcCircleCenter and circleFit are without errors, they should deliver the same result:
    TVector3 hitA, hitB, hitC, hitD, hitE;
    radius = 15;
    sigma = 0.001;
    TVector3 moveCircle = TVector3(radius, 0, 0.);
    hitA.SetMagThetaPhi(gRandom->Gaus(radius, sigma) , gRandom->Gaus(M_PI * 0.5, sigma), bVal * scalePhi); hitA += moveCircle;
    hitB.SetMagThetaPhi(gRandom->Gaus(radius, sigma) , gRandom->Gaus(M_PI * 0.5, sigma), cVal * scalePhi); hitB += moveCircle;
    hitC.SetMagThetaPhi(gRandom->Gaus(radius, sigma) , gRandom->Gaus(M_PI * 0.5, sigma), dVal * scalePhi); hitC += moveCircle;
    hitD.SetMagThetaPhi(gRandom->Gaus(radius, sigma) , gRandom->Gaus(M_PI * 0.5, sigma), eVal * scalePhi); hitD += moveCircle;
    hitE.SetMagThetaPhi(gRandom->Gaus(radius, sigma) , gRandom->Gaus(M_PI * 0.5, sigma), fVal * scalePhi); hitE += moveCircle;
    vector<TVector3> hits;
    hits.push_back(hitA), hits.push_back(hitB), hits.push_back(hitC), hits.push_back(hitD), hits.push_back(hitE);

    f3h.resetValues(hitA, hitB, hitC);

    vector<PositionInfo*> compareVec;
    vector<PositionInfo> tempCompareVec;

    ofstream myfile;
    myfile.open("output.txt");

    int ctr = 0;
    for (TVector3 & hit : hits) {
      PositionInfo posInfo;
      posInfo.hitPosition = hit;
      posInfo.sigmaX = 0.001;
      posInfo.sigmaY = 0.001;

      myfile << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << 0.001 << " " << 0.001 << endl;

      tempCompareVec.push_back(posInfo);
//      compareVec.push_back(&(tempCompareVec.at(ctr)));
      ctr++;
    }
    myfile.close();

    for (int i = 0; i < int(tempCompareVec.size()); ++i) {
      compareVec.push_back(&(tempCompareVec.at(i)));
    }

    aFilter.resetValues(&compareVec);
    aFilter.resetMagneticField();

    TVector3 circleCenter;

//    typedef boost::chrono::high_resolution_clock boostClock; /**< used for measuring time comsumption */ // high_resolution_clock, process_cpu_clock
//     typedef boost::chrono::nanoseconds boostNsec; /**< defines time resolution (currently nanoseconds) */ // microseconds, milliseconds

//    boostClock::time_point startTimer = boostClock::now();
    f3h.calcCircleCenter(hitA, hitB, hitC, circleCenter);
//    boostClock::time_point stopTimer = boostClock::now();
//    boostNsec durCircleCenter = boost::chrono::duration_cast<boostNsec>(stopTimer - startTimer);

//    startTimer = boostClock::now();
    aFilter.circleFit(clapPhi, clapR, estimatedRadius);
    EXPECT_FLOAT_EQ(14.511606, estimatedRadius);
//    stopTimer = boostClock::now();
//    boostNsec durCircleFit = boost::chrono::duration_cast<boostNsec>(stopTimer - startTimer);
//    B2WARNING("after comparison-test, chi2 is " << chi2 << ", clapPhi,clapR,estimatedRadius is: " << clapPhi << "," << clapR << "," << estimatedRadius)
//     double estimatedHelixRadius;
//     TVector3 estimatedMomentum;
//    startTimer = boostClock::now();
    pair<double, TVector3> returnValues = aFilter.helixFit();
    /*    stopTimer = boostClock::now();
        boostNsec durHelixFit = boost::chrono::duration_cast<boostNsec>(stopTimer - startTimer);
      */
//    EXPECT_FLOAT_EQ((hitA-circleCenter).Mag(), returnValues.first);

    EXPECT_FLOAT_EQ(14.511622, returnValues.first);

//    B2WARNING("duration(ns): durCircleCenter: " << durCircleCenter.count() << ", durCircleFit: " << durCircleFit.count() << ", durHelixFit: " << durHelixFit.count() )

    /// testing realistic values of b2fw:
//    PositionInfo pos1, pos2, pos3, pos4, pos5, pos6;
//    pos1.hitPosition = TVector3(4.30225,-9.65892,-1.448);
//     pos1.sigmaX = 0.00201488;
//     pos1.sigmaY = 0.0005161;
//
//    pos2.hitPosition = TVector3(3.64093,-13.1215,-6.748);
//     pos2.sigmaX = 0.00168024;
//     pos2.sigmaY = 0.000568051;
//
//    pos3.hitPosition = TVector3(4.03147,-6.97929,3.244);
//     pos3.sigmaX = 0.00161568;
//     pos3.sigmaY = 0.00188476;
//
//    pos4.hitPosition = TVector3(1.90634,-3.32975,-3.4);
//     pos4.sigmaX = 0.0100725;
//     pos4.sigmaY = 0.000319435;
//
//    pos5.hitPosition = TVector3(1.32762,-1.76951,-1.21305);
//     pos5.sigmaX = 0.000644498;
//     pos5.sigmaY = 0.00109565;
//
//    pos6.hitPosition = TVector3(1.17998,-0.805218,-0.88255);
//     pos6.sigmaX = 0.000683021;
//     pos6.sigmaY = 0.000819625;
  }


  /** testing Eigen library and comparing results with root TMatrixD */
  TEST_F(TrackletFiltersTest, eigenLibraryTest)
  {
    int sizeOfMatrix = 4;
    double seed = 4;
    TMatrixD rootMatrix(sizeOfMatrix, sizeOfMatrix);  // testing matrix -root
    TMatrixD rootMatrix2(sizeOfMatrix, sizeOfMatrix);  // testing matrix -root
    TMatrixD rootVector(1, sizeOfMatrix); // testing vector -root
    rootMatrix.Randomize(3.3, 1.0, seed); // .Randomize(double alpha, double beta, Double_t& seed);
    rootMatrix2.Randomize(4.3, 1.0, seed);
    rootVector.Randomize(3.3, 1.0, seed);
    TMatrixD multipliedRoot = rootVector * rootMatrix;
    TMatrixD multipliedRoot2 = rootMatrix2 * rootMatrix;
    TMatrixD minusRoot = rootMatrix2 - rootMatrix;

    Eigen::MatrixXd eigenMatrix(sizeOfMatrix, sizeOfMatrix);   // testing matrix -eigenLibrary
    Eigen::MatrixXd eigenMatrix2(sizeOfMatrix, sizeOfMatrix);   // testing matrix -eigenLibrary
    Eigen::MatrixXd eigenVector(1, sizeOfMatrix); // testing vector -eigenLibrary


    for (int i = 0; i < sizeOfMatrix; ++i) {
      eigenVector(0, i) = rootVector(0, i);
      EXPECT_DOUBLE_EQ(eigenVector(0, i), rootVector(0, i));
      for (int j = 0; j < sizeOfMatrix; ++j) {
        eigenMatrix(i, j) = rootMatrix(i, j);
        eigenMatrix2(i, j) = rootMatrix2(i, j);
        EXPECT_DOUBLE_EQ(eigenMatrix(i, j), rootMatrix(i, j));
        EXPECT_DOUBLE_EQ(eigenMatrix2(i, j), rootMatrix2(i, j));
//        B2WARNING("eintrag ("<<i<<"/"<<j<<"): " << rootMatrix(i,j))
      }
    }
    Eigen::MatrixXd multipliedEigen;
    multipliedEigen = eigenVector * eigenMatrix;
    Eigen::MatrixXd multipliedEigen2 = eigenMatrix2 * eigenMatrix;
    Eigen::MatrixXd minusEigen;
    minusEigen = eigenMatrix2 - eigenMatrix;
    EXPECT_DOUBLE_EQ(multipliedEigen(0, 0), multipliedRoot(0, 0));
    for (int i = 0; i < sizeOfMatrix; ++i) {
      for (int j = 0; j < sizeOfMatrix; ++j) {
        EXPECT_DOUBLE_EQ(minusRoot(i, j), minusEigen(i, j));
      }
    }

//    multipliedRoot2.Abs().Max();
    /*minusRoot.Abs().Max()*/;
//    multipliedEigen2.array().abs().maxCoeff();
    /*minusEigen.array().abs().maxCoeff()*/;
    EXPECT_DOUBLE_EQ(multipliedRoot2.Abs().Max(), multipliedEigen2.array().abs().maxCoeff());
    EXPECT_DOUBLE_EQ(minusRoot.Abs().Max(), minusEigen.array().abs().maxCoeff());


    //    for (int i= 0; i < sizeOfMatrix; ++i) {
//      for (int j= 0; j < sizeOfMatrix; ++j) {
//        EXPECT_DOUBLE_EQ(multipliedRoot2(i,j), multipliedEigen2(i,j));
//        EXPECT_DOUBLE_EQ(minusRoot(i,j), minusEigen(i,j));
//      }
//    }
  }



  /** testing helix fit for different cases*/
  TEST_F(TrackletFiltersTest, helixFitterTest)
  {
    TrackletFilters aFilter = TrackletFilters();
    vector<PositionInfo*> badVec1, badVec2, badVec3, badVec4, badVec5;
    vector<PositionInfo> tbadVec1, tbadVec2, tbadVec3, tbadVec4, tbadVec5;
    double clapPhi, clapR, estimatedRadius;

    stringstream HitInfos;
    HitInfos << "bad p: " << 15.0808 << "\n";
    /// badVec1
    TVector3 hit = TVector3(-6.516, -1.05416, 0.134742);
    PositionInfo posInfo;
    posInfo.hitPosition = hit;
    posInfo.sigmaX = 0.0160026;
    posInfo.sigmaY = 0.0112734;
    tbadVec1.push_back(posInfo);
    HitInfos << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaX << " " <<  posInfo.sigmaY << endl;

    hit.SetXYZ(-3.516, -0.986426, 0.14058);
    posInfo.hitPosition = hit;
    posInfo.sigmaX = 0.0160026;
    posInfo.sigmaY = 0.0112734;
    tbadVec1.push_back(posInfo);
    HitInfos << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaX << " " <<  posInfo.sigmaY << endl;

//   pVector  x/y/z: -15.0774/-0.319219/0.0254473  { module: VXDTF }
    hit.SetXYZ(-1.016, -0.930059, 0.142918);
    posInfo.hitPosition = hit;
    posInfo.sigmaX = 0.0160026;
    posInfo.sigmaY = 0.0112734;
    tbadVec1.push_back(posInfo);
    HitInfos << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaX << " " <<  posInfo.sigmaY << endl;

    hit.SetXYZ(3.184, -0.839871, 0.14732);
    posInfo.hitPosition = hit;
    posInfo.sigmaX = 0.0160026;
    posInfo.sigmaY = 0.0112734;
    tbadVec1.push_back(posInfo);
    HitInfos << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaX << " " <<  posInfo.sigmaY << endl;
    B2INFO("badVec1: " << HitInfos.str() << endl)

    for (int i = 0; i < int(tbadVec1.size()); ++i) { badVec1.push_back(&(tbadVec1.at(i))); }
    aFilter.resetValues(&badVec1);
    aFilter.resetMagneticField(0.976);

    pair<double, TVector3> returnValues = aFilter.helixFit();
//    EXPECT_FLOAT_EQ(15.0808, returnValues.second.Mag());

    aFilter.circleFit(clapPhi, clapR, estimatedRadius);
    EXPECT_FLOAT_EQ(returnValues.first, estimatedRadius);



    /// badVec2
    stringstream HitInfos1;
    HitInfos1 << "bad p: " << 258.543 << "\n";
    hit.SetXYZ(-6.516, -1.05416, 0.134742);
    posInfo.hitPosition = hit;
    posInfo.sigmaX = 0.0160026;
    posInfo.sigmaY = 0.0112734;
    tbadVec2.push_back(posInfo);
    HitInfos1 << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaX << " " <<  posInfo.sigmaY << endl;
//    [WARNING] helixFit: strange pVector (Mag=258.543) detected. The following hits were part of this TC:

//  pVector  x/y/z: -258.477/-5.82419/0.388155  { module: VXDTF }

    hit.SetXYZ(-3.516, -0.986426, 0.14058);
    posInfo.hitPosition = hit;
    posInfo.sigmaX = 0.0160026;
    posInfo.sigmaY = 0.0112734;
    tbadVec2.push_back(posInfo);
    HitInfos1 << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaX << " " <<  posInfo.sigmaY << endl;

    hit.SetXYZ(-1.016, -0.930059, 0.142918);
    posInfo.hitPosition = hit;
    posInfo.sigmaX = 0.0160026;
    posInfo.sigmaY = 0.0112734;
    tbadVec2.push_back(posInfo);
    HitInfos1 << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaX << " " <<  posInfo.sigmaY << endl;

    B2INFO("badVec2: " << HitInfos1.str() << endl)

    for (int i = 0; i < int(tbadVec2.size()); ++i) { badVec2.push_back(&(tbadVec2.at(i))); }
    aFilter.resetValues(&badVec2);
    aFilter.resetMagneticField(0.976);

    aFilter.circleFit(clapPhi, clapR, estimatedRadius);

    returnValues = aFilter.helixFit();
//    EXPECT_FLOAT_EQ(258.543, returnValues.second.Mag());
    EXPECT_FLOAT_EQ(returnValues.first, estimatedRadius);



    /// badVec3
    stringstream HitInfos2;
    HitInfos2 << "bad p: " << 25.2143 << "\n";
    hit.SetXYZ(-6.516, -1.00897, 0.314466);
    posInfo.hitPosition = hit;
    posInfo.sigmaX = 0.0160026;
    posInfo.sigmaY = 0.0112734;
    tbadVec3.push_back(posInfo);
    HitInfos2 << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaX << " " <<  posInfo.sigmaY << endl;
//    [WARNING] helixFit: strange pVector (Mag=25.2143) detected. The following hits were part of this TC:
//  pVector  x/y/z: -25.208/-0.563156/-0.0166426  { module: VXDTF }

    hit.SetXYZ(-3.516, -0.943345, 0.313504);
    posInfo.hitPosition = hit;
    posInfo.sigmaX = 0.0160026;
    posInfo.sigmaY = 0.0112734;
    tbadVec3.push_back(posInfo);
    HitInfos2 << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaX << " " <<  posInfo.sigmaY << endl;

    hit.SetXYZ(-1.016, -0.887857, 0.310771);
    posInfo.hitPosition = hit;
    posInfo.sigmaX = 0.0160026;
    posInfo.sigmaY = 0.0112734;
    tbadVec3.push_back(posInfo);
    HitInfos2 << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaX << " " <<  posInfo.sigmaY << endl;

    B2INFO("badVec3: " << HitInfos2.str() << endl)

    for (int i = 0; i < int(tbadVec3.size()); ++i) { badVec3.push_back(&(tbadVec3.at(i))); }
    aFilter.resetValues(&badVec3);
    aFilter.resetMagneticField(0.976);

    returnValues = aFilter.helixFit();
    // since every machine produces other values, I will catch the result here using a more loose check:
    bool didWork = false;
    if (returnValues.second.Mag() < 26. and returnValues.second.Mag() > 25) { didWork = true; }
    EXPECT_TRUE(didWork);

//    aFilter.circleFit(clapPhi, clapR, estimatedRadius);
    EXPECT_FLOAT_EQ(returnValues.first, estimatedRadius);




    /// badVec4
    stringstream HitInfos3;
    HitInfos3 << "\n bad p: " << 14.8262 << "\n";
    hit.SetXYZ(-6.516, -0.575024, 0.326773);
    posInfo.hitPosition = hit;
    posInfo.sigmaX = 0.0160026;
    posInfo.sigmaY = 0.0112734;
    tbadVec4.push_back(posInfo);
    HitInfos3 << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaX << " " <<  posInfo.sigmaY << endl;
//    [WARNING] helixFit: strange pVector (Mag=) detected. The following hits were part of this TC:
//  pVector  x/y/z: -14.8227/-0.318451/-0.000269897  { module: VXDTF }
    hit.SetXYZ(-3.516, -0.512941, 0.325116);
    posInfo.hitPosition = hit;
    posInfo.sigmaX = 0.0160026;
    posInfo.sigmaY = 0.0112734;
    tbadVec4.push_back(posInfo);
    HitInfos3 << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaX << " " <<  posInfo.sigmaY << endl;

    hit.SetXYZ(-1.016, -0.459848, 0.326773);
    posInfo.hitPosition = hit;
    posInfo.sigmaX = 0.0160026;
    posInfo.sigmaY = 0.0112734;
    tbadVec4.push_back(posInfo);

    HitInfos3 << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaX << " " <<  posInfo.sigmaY << endl;
    B2INFO("badVec4: " << HitInfos3.str() << endl)

    for (int i = 0; i < int(tbadVec4.size()); ++i) { badVec4.push_back(&(tbadVec4.at(i))); }
    aFilter.resetValues(&badVec4);
    aFilter.resetMagneticField(0.976);

    aFilter.circleFit(clapPhi, clapR, estimatedRadius);

    returnValues = aFilter.helixFit();
//    EXPECT_FLOAT_EQ(14.8262, returnValues.second.Mag());
    EXPECT_FLOAT_EQ(returnValues.first, estimatedRadius);



    /// badVec5
    stringstream HitInfos4;
    HitInfos4 << "bad p: " << 337657 << "\n";
    hit.SetXYZ(-6.516, -0.930059, -0.153305);
    posInfo.hitPosition = hit;
    posInfo.sigmaX = 0.0160026;
    posInfo.sigmaY = 0.0112734;
    tbadVec5.push_back(posInfo);
    HitInfos4 << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaX << " " <<  posInfo.sigmaY << endl;
//    [WARNING] helixFit: strange pVector (Mag=) detected. The following hits were part of this TC:
//  pVector  x/y/z: -337570/-7611.14/906.841  { module: VXDTF }
    hit.SetXYZ(-3.516, -0.862418, -0.143971);
    posInfo.hitPosition = hit;
    posInfo.sigmaX = 0.0160026;
    posInfo.sigmaY = 0.0112734;
    tbadVec5.push_back(posInfo);
    HitInfos4 << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaX << " " <<  posInfo.sigmaY << endl;

    hit.SetXYZ(-1.016, -0.806051, -0.137302);
    posInfo.hitPosition = hit;
    posInfo.sigmaX = 0.0160026;
    posInfo.sigmaY = 0.0112734;
    tbadVec5.push_back(posInfo);
    HitInfos4 << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaX << " " <<  posInfo.sigmaY << endl;

    B2INFO("badVec5: " << HitInfos4.str() << endl)

    for (int i = 0; i < int(tbadVec5.size()); ++i) { badVec5.push_back(&(tbadVec5.at(i))); }
    aFilter.resetValues(&badVec5);
    aFilter.resetMagneticField(0.976);

    returnValues = aFilter.helixFit();
//    EXPECT_FLOAT_EQ(337657, returnValues.second.Mag());

    aFilter.circleFit(clapPhi, clapR, estimatedRadius);
    EXPECT_FLOAT_EQ(estimatedRadius, returnValues.first);

    /*

        [WARNING] helixFit: strange pVector (Mag=15.0808) detected. The following hits were part of this TC:
     hit 0: x/y/z/sigmaU/sigmaV: -6.516/-1.05416/0.134742/0.0160026/0.0112734
     hit 1: x/y/z/sigmaU/sigmaV: -3.516/-0.986426/0.14058/0.0160026/0.0112734
     hit 2: x/y/z/sigmaU/sigmaV: -1.016/-0.930059/0.142918/0.0160026/0.0112734
     hit 3: x/y/z/sigmaU/sigmaV: 3.184/-0.839871/0.14732/0.0160026/0.0112734
      pVector  x/y/z: -15.0774/-0.319219/0.0254473  { module: VXDTF }


    [WARNING] helixFit: strange pVector (Mag=258.543) detected. The following hits were part of this TC:
     hit 0: x/y/z/sigmaU/sigmaV: -6.516/-1.05416/0.134742/0.0160026/0.0112734
     hit 1: x/y/z/sigmaU/sigmaV: -3.516/-0.986426/0.14058/0.0160026/0.0112734
     hit 2: x/y/z/sigmaU/sigmaV: -1.016/-0.930059/0.142918/0.0160026/0.0112734
     pVector  x/y/z: -258.477/-5.82419/0.388155  { module: VXDTF }


     [WARNING] helixFit: strange pVector (Mag=25.2143) detected. The following hits were part of this TC:
     hit 0: x/y/z/sigmaU/sigmaV: -6.516/-1.00897/0.314466/0.0160026/0.0112734
     hit 1: x/y/z/sigmaU/sigmaV: -3.516/-0.943345/0.313504/0.0160026/0.0112734
     hit 2: x/y/z/sigmaU/sigmaV: -1.016/-0.887857/0.310771/0.0160026/0.0112734
     pVector  x/y/z: -25.208/-0.563156/-0.0166426  { module: VXDTF }


     [WARNING] helixFit: strange pVector (Mag=14.8262) detected. The following hits were part of this TC:
     hit 0: x/y/z/sigmaU/sigmaV: -6.516/-0.575024/0.326773/0.0160026/0.0112734
     hit 1: x/y/z/sigmaU/sigmaV: -3.516/-0.512941/0.325116/0.0160026/0.0112734
     hit 2: x/y/z/sigmaU/sigmaV: -1.016/-0.459848/0.326773/0.0160026/0.0112734
     pVector  x/y/z: -14.8227/-0.318451/-0.000269897  { module: VXDTF }


     [WARNING] helixFit: strange pVector (Mag=337657) detected. The following hits were part of this TC:
     hit 0: x/y/z/sigmaU/sigmaV: -6.516/-0.930059/-0.153305/0.0160026/0.0112734
     hit 1: x/y/z/sigmaU/sigmaV: -3.516/-0.862418/-0.143971/0.0160026/0.0112734
     hit 2: x/y/z/sigmaU/sigmaV: -1.016/-0.806051/-0.137302/0.0160026/0.0112734
     pVector  x/y/z: -337570/-7611.14/906.841  { module: VXDTF }*/
  }
}  // namespace
