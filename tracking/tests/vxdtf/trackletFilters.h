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
#include <TMathBase.h>

//boost:
#include <boost/bind/bind.hpp>

//Eigen
#include <Eigen/Dense>

using namespace std;

namespace Belle2 {





  /** Producing suitable input data for for the TrackletFilters-class, execute this file typing "test_tracking" */
  class TrackletFiltersTest : public ::testing::Test {
  protected:
  };



  /** TrackletFilters expects a row of hits stored as a vector of TVector3. These should be on a realistic track through the detector, meaning in the x-y-plane, they form a circle passing the origin (or a point near to it). There have to be at least 4 hits for the ZigZag-Test, for the circlefitter it can be 3 hits, but it should be 4 at least. we will test it first with an aequivalent of a 500 MeV (pT) track and another one having only 50 MeV. The reason for having more than one cirle is the fact that the circlefitter seems to be optimized somehow and therefore could have problems with circles formed by a small number of hits or circles with small radii*/
  TEST_F(TrackletFiltersTest, simpleTest)
  {
    B2WARNING("TrackletFilters.simpleTest: for convenience, this test has been temporarily disabled, this warning shall stay here until the test has been replaced by are more practical one");
//     TVector3 a, b, c, d, e, f, g, h, i, j, k, l, m; // there are up to 12+1 Hits (one for the primary vertex) possible within the VXD before the particle curls back the first time
//
//     //The hits will be produced using the same radius (which will be smeared) and different phi-values to describe a cirle-segment covering only a part of the outline of the circle. To get a full helix, the z-value of the hits are changed accordingly to their original phi-value
//
//     LittleHelper hBox = LittleHelper(); // smears hits using smearValueGauss(double low, double high, double mean, double sigma)
//     double sigma = 0.001, scalePhi = 11.9, scaleZ = 88., t90 = M_PI * 0.5, radius = 11.;
//     double aVal = 0.0001, bVal = 0.006, cVal = 0.007, dVal = 0.011, eVal = 0.0124, fVal = 0.018, gVal = 0.0202, hVal = 0.039, iVal = 0.042, jVal = 0.052, kVal = 0.055, lVal = 0.068, mVal = 0.0715;
//     vector<TVector3> highPtMini, highPtMaxi; // carry hitPositions;
//     vector<PositionInfo> tempMiniStuff, tempMaxiStuff, tempTestStuff; // tempVectors to be able to keep PositionInfo
//     vector<PositionInfo*> miniStuff, maxiStuff, testStuff; // carry positionInfo (TrackletFilters expects pointers)
//
//     //phi does not have to be smeared to simulate the realistic situation, some hits will be positioned pretty near next to each other
//     a.SetMagThetaPhi(hBox.smearValueGauss(-1111., 1111., radius, sigma), t90, aVal * scalePhi),
//                      b.SetMagThetaPhi(hBox.smearValueGauss(-1111., 1111., radius, sigma), t90, bVal * scalePhi), //L1
//                      c.SetMagThetaPhi(hBox.smearValueGauss(-1111., 1111., radius, sigma), t90, cVal * scalePhi),
//                      d.SetMagThetaPhi(hBox.smearValueGauss(-1111., 1111., radius, sigma), t90, dVal * scalePhi), //L2
//                      e.SetMagThetaPhi(hBox.smearValueGauss(-1111., 1111., radius, sigma), t90, eVal * scalePhi),
//                      f.SetMagThetaPhi(hBox.smearValueGauss(-1111., 1111., radius, sigma), t90, fVal * scalePhi), //L3
//                      g.SetMagThetaPhi(hBox.smearValueGauss(-1111., 1111., radius, sigma), t90, gVal * scalePhi),
//                      h.SetMagThetaPhi(hBox.smearValueGauss(-1111., 1111., radius, sigma), t90, hVal * scalePhi), //L4
//                      i.SetMagThetaPhi(hBox.smearValueGauss(-1111., 1111., radius, sigma), t90, iVal * scalePhi),
//                      j.SetMagThetaPhi(hBox.smearValueGauss(-1111., 1111., radius, sigma), t90, jVal * scalePhi), //L5
//                      k.SetMagThetaPhi(hBox.smearValueGauss(-1111., 1111., radius, sigma), t90, kVal * scalePhi),
//                      l.SetMagThetaPhi(hBox.smearValueGauss(-1111., 1111., radius, sigma), t90, lVal * scalePhi), //L6
//                      m.SetMagThetaPhi(hBox.smearValueGauss(-1111., 1111., radius, sigma), t90, mVal * scalePhi);
//
//     a.SetZ(aVal * scaleZ), b.SetZ(bVal * scaleZ), c.SetZ(cVal * scaleZ), d.SetZ(dVal * scaleZ), e.SetZ(eVal * scaleZ), f.SetZ(fVal * scaleZ), g.SetZ(gVal * scaleZ), h.SetZ(hVal * scaleZ), i.SetZ(iVal * scaleZ), j.SetZ(jVal * scaleZ), k.SetZ(kVal * scaleZ), l.SetZ(lVal * scaleZ), m.SetZ(mVal * scaleZ); // atm not needed, forces hits to lie on a helix in xyz
//
//     //now we have got hits lying on a small part of a circle having its center at the origin. Therefore I have to move the circle so its arch passes the origin
//     TVector3 move500 = TVector3(radius, 0.5, 0.);
//
//     a -= move500, b -= move500, c -= move500, d -= move500, e -= move500, f -= move500, g -= move500, h -= move500, i -= move500, j -= move500, k -= move500, l -= move500, m -= move500;
//
//
//     highPtMini.push_back(l), highPtMini.push_back(g), highPtMini.push_back(f), highPtMini.push_back(a);
//     highPtMaxi.push_back(a), highPtMaxi.push_back(b), highPtMaxi.push_back(c), highPtMaxi.push_back(d), highPtMaxi.push_back(e), highPtMaxi.push_back(f), highPtMaxi.push_back(g), highPtMaxi.push_back(h), highPtMaxi.push_back(i), highPtMaxi.push_back(j), highPtMaxi.push_back(k), highPtMaxi.push_back(l), highPtMaxi.push_back(m);
//
//
//     for (TVector3 & hit : highPtMaxi) {
//       PositionInfo posInfo;
//       posInfo.hitPosition = hit;
//       posInfo.hitSigma.SetXYZ(sigma, sigma, sigma);
//       posInfo.sigmaU = sigma;
//       tempMaxiStuff.push_back(posInfo);
//     }
//     for (PositionInfo & hit : tempMaxiStuff) {
//       maxiStuff.push_back(&hit);
//     }
//
//     for (TVector3 & hit : highPtMini) {
//       PositionInfo posInfo;
//       posInfo.hitPosition = hit;
//       posInfo.hitSigma.SetXYZ(sigma, sigma, sigma);
//       posInfo.sigmaU = sigma;
//       tempMiniStuff.push_back(posInfo);
//     }
//     for (PositionInfo & hit : tempMiniStuff) {
//       miniStuff.push_back(&hit);
//     }
//
//     TVector3 testa = TVector3(1 - (1. / sqrt(2.)), 1. / sqrt(2.), 0);
//     TVector3 testb = TVector3(1., 1., 0);
//     TVector3 testc = TVector3(1 + (1. / sqrt(2.)), 1. / sqrt(2.), 0);
//     TVector3 testd = TVector3(2., 0., 0);
//     TVector3 teste = TVector3(1., -1., 0);
//     TVector3 moveSmall = TVector3(0.1, 0.0, 0.);
//     vector<TVector3> testVector;
//     testVector.push_back(testa += moveSmall); testVector.push_back(testb += moveSmall); testVector.push_back(testc += moveSmall); testVector.push_back(testd += moveSmall); testVector.push_back(teste += moveSmall);
//
//     for (TVector3 & hit : testVector) {
//       PositionInfo posInfo;
//       posInfo.hitPosition = hit;
//       posInfo.sigmaU = sigma;
//       posInfo.hitSigma.SetXYZ(sigma, sigma, sigma);
//       tempTestStuff.push_back(posInfo);
//     }
//     for (PositionInfo & hit : tempTestStuff) {
//       testStuff.push_back(&hit);
//     }
//
//     ThreeHitFilters f3h = ThreeHitFilters();
//
// //     B2WARNING(" calcsign mlk, lkj, kji, jih, ihg, hgf, gfe, fed, edc, dcb, cba:" << f3h.calcSign(m, l, k) << "," << f3h.calcSign(l, k, j) << "," << f3h.calcSign(k, j, i) << "," << f3h.calcSign(j, i, h) << "," << f3h.calcSign(i, h, g) << "," << f3h.calcSign(h, g, f) << "," << f3h.calcSign(g, f, e) << "," << f3h.calcSign(f, e, d) << "," << f3h.calcSign(e, d, c) << "," << f3h.calcSign(d, c, b) << "," << f3h.calcSign(c, b, a));
//     EXPECT_EQ(f3h.calcSign(l, g, f), f3h.calcSign(g, f, a));
//
//     TrackletFilters aFilter = TrackletFilters();
//     double pocaPtPhi = 0, pocaR = 0, estimatedCurvature = 0;
//
//     aFilter.resetValues(&maxiStuff);
//     EXPECT_FALSE(aFilter.ziggZaggXY());
//     EXPECT_FALSE(aFilter.ziggZaggRZ());
//     double chi2 = aFilter.circleFit(pocaPtPhi, pocaR, estimatedCurvature);
// //     B2WARNING("after maxi-test, chi2 is " << chi2 << ", pocaPtPhi,pocaR,estimatedRadius is: " << pocaPtPhi << "," << pocaR << "," << estimatedRadius)
//     chi2++; // is simply for removing the warning. Has no other meaning since Value is resetted all the time
//
//     aFilter.resetValues(&miniStuff);
//     EXPECT_FALSE(aFilter.ziggZaggXY());
//     EXPECT_FALSE(aFilter.ziggZaggRZ());
//     chi2 = aFilter.circleFit(pocaPtPhi, pocaR, estimatedCurvature);
// //     B2WARNING("after mini-test, chi2 is " << chi2 << " pocaPtPhi,pocaR,estimatedRadius is: " << pocaPtPhi << "," << pocaR << "," << estimatedRadius)
//
//     aFilter.resetValues(&testStuff);
//     EXPECT_FALSE(aFilter.ziggZaggXY());
//     EXPECT_FALSE(aFilter.ziggZaggRZ());
//     chi2 = aFilter.circleFit(pocaPtPhi, pocaR, estimatedCurvature);
// //     B2WARNING("after testVector-test, chi2 is " << chi2 << " pocaPtPhi,pocaR,estimatedRadius is: " << pocaPtPhi << "," << pocaR << "," << estimatedRadius)
//
//     /// comparing calcCircleCenter and circleFit: taking 3 hits lying on a circle (by definition) and a 4th one lying there too. If calcCircleCenter and circleFit are without errors, they should deliver the same result:
//     TVector3 hitA, hitB, hitC, hitD, hitE;
//     radius = 15;
//     sigma = 0.001;
//     TVector3 moveCircle = TVector3(radius, 0, 0.);
//     hitA.SetMagThetaPhi(gRandom->Gaus(radius, sigma) , gRandom->Gaus(M_PI * 0.5, sigma), bVal * scalePhi); hitA += moveCircle;
//     hitB.SetMagThetaPhi(gRandom->Gaus(radius, sigma) , gRandom->Gaus(M_PI * 0.5, sigma), cVal * scalePhi); hitB += moveCircle;
//     hitC.SetMagThetaPhi(gRandom->Gaus(radius, sigma) , gRandom->Gaus(M_PI * 0.5, sigma), dVal * scalePhi); hitC += moveCircle;
//     hitD.SetMagThetaPhi(gRandom->Gaus(radius, sigma) , gRandom->Gaus(M_PI * 0.5, sigma), eVal * scalePhi); hitD += moveCircle;
//     hitE.SetMagThetaPhi(gRandom->Gaus(radius, sigma) , gRandom->Gaus(M_PI * 0.5, sigma), fVal * scalePhi); hitE += moveCircle;
//     vector<TVector3> hits;
//     hits.push_back(hitA), hits.push_back(hitB), hits.push_back(hitC), hits.push_back(hitD), hits.push_back(hitE);
//
//     f3h.resetValues(hitA, hitB, hitC);
//
//     vector<PositionInfo*> compareVec;
//     vector<PositionInfo> tempCompareVec;
//
//  /// optional fileOutput, uncomment if needed!
// //     ofstream myfile;
// //     myfile.open("output.txt");
// //
// //     int ctr = 0;
// //     for (TVector3 & hit : hits) {
// //       PositionInfo posInfo;
// //       posInfo.hitPosition = hit;
// //       posInfo.sigmaU = 0.001;
// //       posInfo.sigmaV = 0.001;
// //       posInfo.hitSigma.SetXYZ(sigma, sigma, sigma);
// //
// //       myfile << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << 0.001 << " " << 0.001 << endl;
// //
// //       tempCompareVec.push_back(posInfo);
// //       ctr++;
// //     }
// //     myfile.close();
//
//     for (int i = 0; i < int(tempCompareVec.size()); ++i) {
//       compareVec.push_back(&(tempCompareVec.at(i)));
//     }
//
//     aFilter.resetValues(&compareVec);
//     aFilter.resetMagneticField();
//
//     TVector3 circleCenter;
//
//     f3h.calcCircleCenter(hitA, hitB, hitC, circleCenter);
//
//     aFilter.circleFit(pocaPtPhi, pocaR, estimatedCurvature);
//     EXPECT_NEAR(1. / 14.511606, estimatedCurvature, 0.15); // one percent deviation should be enough
//
// //    B2WARNING("after comparison-test, chi2 is " << chi2 << ", pocaPtPhi,pocaR,estimatedRadius is: " << pocaPtPhi << "," << pocaR << "," << estimatedRadius)
//
//     pair<double, TVector3> returnValues = aFilter.helixFit();
//
//     EXPECT_NEAR(14.511622, returnValues.first, 0.15); // one percent deviation should be enough
//
//     /// testing realistic values of b2fw:
// //    PositionInfo pos1, pos2, pos3, pos4, pos5, pos6;
// //    pos1.hitPosition = TVector3(4.30225,-9.65892,-1.448);
// //     pos1.sigmaU = 0.00201488;
// //     pos1.sigmaV = 0.0005161;
// //
// //    pos2.hitPosition = TVector3(3.64093,-13.1215,-6.748);
// //     pos2.sigmaU = 0.00168024;
// //     pos2.sigmaV = 0.000568051;
// //
// //    pos3.hitPosition = TVector3(4.03147,-6.97929,3.244);
// //     pos3.sigmaU = 0.00161568;
// //     pos3.sigmaV = 0.00188476;
// //
// //    pos4.hitPosition = TVector3(1.90634,-3.32975,-3.4);
// //     pos4.sigmaU = 0.0100725;
// //     pos4.sigmaV = 0.000319435;
// //
// //    pos5.hitPosition = TVector3(1.32762,-1.76951,-1.21305);
// //     pos5.sigmaU = 0.000644498;
// //     pos5.sigmaV = 0.00109565;
// //
// //    pos6.hitPosition = TVector3(1.17998,-0.805218,-0.88255);
// //     pos6.sigmaU = 0.000683021;
// //     pos6.sigmaV = 0.000819625;
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
    double pocaPtPhi, pocaR, estimatedCurvature;

//     stringstream HitInfos;
//     HitInfos << "bad p: " << 15.0808 << "\n";
    /// badVec1
    TVector3 hit = TVector3(-6.516, -1.05416, 0.134742);
    PositionInfo posInfo;
    posInfo.hitPosition = hit;
    posInfo.sigmaU = 0.0160026;
    posInfo.sigmaV = 0.0112734;
    posInfo.hitSigma.SetXYZ(0.0112734, 0.0112734, 0.0160026);
    tbadVec1.push_back(posInfo);
//     HitInfos << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaU << " " <<  posInfo.sigmaV << endl;

    hit.SetXYZ(-3.516, -0.986426, 0.14058);
    posInfo.hitPosition = hit;
    posInfo.sigmaU = 0.0160026;
    posInfo.sigmaV = 0.0112734;
    posInfo.hitSigma.SetXYZ(0.0112734, 0.0112734, 0.0160026);
    tbadVec1.push_back(posInfo);
//     HitInfos << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaU << " " <<  posInfo.sigmaV << endl;

//   pVector  x/y/z: -15.0774/-0.319219/0.0254473  { module: VXDTF }
    hit.SetXYZ(-1.016, -0.930059, 0.142918);
    posInfo.hitPosition = hit;
    posInfo.sigmaU = 0.0160026;
    posInfo.sigmaV = 0.0112734;
    posInfo.hitSigma.SetXYZ(0.0112734, 0.0112734, 0.0160026);
    tbadVec1.push_back(posInfo);
//     HitInfos << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaU << " " <<  posInfo.sigmaV << endl;

    hit.SetXYZ(3.184, -0.839871, 0.14732);
    posInfo.hitPosition = hit;
    posInfo.sigmaU = 0.0160026;
    posInfo.sigmaV = 0.0112734;
    posInfo.hitSigma.SetXYZ(0.0112734, 0.0112734, 0.0160026);
    tbadVec1.push_back(posInfo);
//     HitInfos << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaU << " " <<  posInfo.sigmaV << endl;
//     B2INFO("badVec1: " << HitInfos.str() << endl)

    for (int i = 0; i < int(tbadVec1.size()); ++i) { badVec1.push_back(&(tbadVec1.at(i))); }
    aFilter.resetValues(&badVec1);
    aFilter.resetMagneticField(0.976);

    pair<double, TVector3> returnValues = aFilter.helixFit();
//    EXPECT_FLOAT_EQ(15.0808, returnValues.second.Mag());

    aFilter.circleFit(pocaPtPhi, pocaR, estimatedCurvature);
    // for counterClockwise movement, the sign of the curvature calculated by the circleFit is negative...
    EXPECT_FLOAT_EQ(returnValues.first, -1. / estimatedCurvature);


    /// badVec2
//     stringstream HitInfos1;
//     HitInfos1 << "bad p: " << 258.543 << "\n";
    hit.SetXYZ(-6.516, -1.05416, 0.134742);
    posInfo.hitPosition = hit;
    posInfo.sigmaU = 0.0160026;
    posInfo.sigmaV = 0.0112734;
    posInfo.hitSigma.SetXYZ(0.0112734, 0.0112734, 0.0160026);
    tbadVec2.push_back(posInfo);
//     HitInfos1 << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaU << " " <<  posInfo.sigmaV << endl;
//    [WARNING] helixFit: strange pVector (Mag=258.543) detected. The following hits were part of this TC:

//  pVector  x/y/z: -258.477/-5.82419/0.388155  { module: VXDTF }

    hit.SetXYZ(-3.516, -0.986426, 0.14058);
    posInfo.hitPosition = hit;
    posInfo.sigmaU = 0.0160026;
    posInfo.sigmaV = 0.0112734;
    posInfo.hitSigma.SetXYZ(0.0112734, 0.0112734, 0.0160026);
    tbadVec2.push_back(posInfo);
//     HitInfos1 << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaU << " " <<  posInfo.sigmaV << endl;

    hit.SetXYZ(-1.016, -0.930059, 0.142918);
    posInfo.hitPosition = hit;
    posInfo.sigmaU = 0.0160026;
    posInfo.sigmaV = 0.0112734;
    posInfo.hitSigma.SetXYZ(0.0112734, 0.0112734, 0.0160026);
    tbadVec2.push_back(posInfo);
//     HitInfos1 << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaU << " " <<  posInfo.sigmaV << endl;

//     B2INFO("badVec2: " << HitInfos1.str() << endl)

    for (int i = 0; i < int(tbadVec2.size()); ++i) { badVec2.push_back(&(tbadVec2.at(i))); }
    aFilter.resetValues(&badVec2);
    aFilter.resetMagneticField(0.976);

    aFilter.circleFit(pocaPtPhi, pocaR, estimatedCurvature);

    returnValues = aFilter.helixFit();
//    EXPECT_FLOAT_EQ(258.543, returnValues.second.Mag());
    // for counterClockwise movement, the sign of the curvature calculated by the circleFit is negative...  (curvature in direction of flight is bent upwards)
    EXPECT_FLOAT_EQ(returnValues.first, -1. / estimatedCurvature);


    /// badVec3
//     stringstream HitInfos2;
//     HitInfos2 << "bad p: " << 25.2143 << "\n";
    hit.SetXYZ(-6.516, -1.00897, 0.314466);
    posInfo.hitPosition = hit;
    posInfo.sigmaU = 0.0160026;
    posInfo.sigmaV = 0.0112734;
    posInfo.hitSigma.SetXYZ(0.0112734, 0.0112734, 0.0160026);
    tbadVec3.push_back(posInfo);
//     HitInfos2 << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaU << " " <<  posInfo.sigmaV << endl;
//    [WARNING] helixFit: strange pVector (Mag=25.2143) detected. The following hits were part of this TC:
//  pVector  x/y/z: -25.208/-0.563156/-0.0166426  { module: VXDTF }

    hit.SetXYZ(-3.516, -0.943345, 0.313504);
    posInfo.hitPosition = hit;
    posInfo.sigmaU = 0.0160026;
    posInfo.sigmaV = 0.0112734;
    posInfo.hitSigma.SetXYZ(0.0112734, 0.0112734, 0.0160026);
    tbadVec3.push_back(posInfo);
//     HitInfos2 << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaU << " " <<  posInfo.sigmaV << endl;

    hit.SetXYZ(-1.016, -0.887857, 0.310771);
    posInfo.hitPosition = hit;
    posInfo.sigmaU = 0.0160026;
    posInfo.sigmaV = 0.0112734;
    posInfo.hitSigma.SetXYZ(0.0112734, 0.0112734, 0.0160026);
    tbadVec3.push_back(posInfo);
//     HitInfos2 << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaU << " " <<  posInfo.sigmaV << endl;

//     B2INFO("badVec3: " << HitInfos2.str() << endl)

    for (int i = 0; i < int(tbadVec3.size()); ++i) { badVec3.push_back(&(tbadVec3.at(i))); }
    aFilter.resetValues(&badVec3);
    aFilter.resetMagneticField(0.976);

    returnValues = aFilter.helixFit();
// // //     // since every machine produces other values, I will catch the result here using a more loose check:
// // // //     bool didWork = false;
// // // //     if (returnValues.second.Perp() < 26. and returnValues.second.Perp() > 25) { didWork = true; }
// // // //     EXPECT_TRUE(didWork);
    aFilter.circleFit(pocaPtPhi, pocaR, estimatedCurvature);
    // particle has a clockwise movement (curvature in direction of flight is bent downwards)
    EXPECT_NEAR(returnValues.first, /*-*/1. / estimatedCurvature, 0.5); // accepting deviation of 0.5
// //     EXPECT_NEAR(returnValues.second.Perp(), estimatedRadius, 0.5); // accepting deviation of 0.5
//    aFilter.circleFit(pocaPtPhi, pocaR, estimatedRadius);
    // EXPECT_FLOAT_EQ(returnValues.first, estimatedRadius);


    /// badVec4
//     stringstream HitInfos3;
//     HitInfos3 << "\n bad p: " << 14.8262 << "\n";
    hit.SetXYZ(-6.516, -0.575024, 0.326773);
    posInfo.hitPosition = hit;
    posInfo.sigmaU = 0.0160026;
    posInfo.sigmaV = 0.0112734;
    posInfo.hitSigma.SetXYZ(0.0112734, 0.0112734, 0.0160026);
    tbadVec4.push_back(posInfo);
//     HitInfos3 << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaU << " " <<  posInfo.sigmaV << endl;
//    [WARNING] helixFit: strange pVector (Mag=) detected. The following hits were part of this TC:
//  pVector  x/y/z: -14.8227/-0.318451/-0.000269897  { module: VXDTF }
    hit.SetXYZ(-3.516, -0.512941, 0.325116);
    posInfo.hitPosition = hit;
    posInfo.sigmaU = 0.0160026;
    posInfo.sigmaV = 0.0112734;
    posInfo.hitSigma.SetXYZ(0.0112734, 0.0112734, 0.0160026);
    tbadVec4.push_back(posInfo);
//     HitInfos3 << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaU << " " <<  posInfo.sigmaV << endl;

    hit.SetXYZ(-1.016, -0.459848, 0.326773);
    posInfo.hitPosition = hit;
    posInfo.sigmaU = 0.0160026;
    posInfo.sigmaV = 0.0112734;
    posInfo.hitSigma.SetXYZ(0.0112734, 0.0112734, 0.0160026);
    tbadVec4.push_back(posInfo);
// bv2: 44,2909026 44,3521919 ccw  bv3: 45,7142857 45,0547866 cw  bv4: 48,3224071 47,0871866 cw
//     HitInfos3 << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaU << " " <<  posInfo.sigmaV << endl;
//     B2INFO("badVec4: " << HitInfos3.str() << endl)

    for (int i = 0; i < int(tbadVec4.size()); ++i) { badVec4.push_back(&(tbadVec4.at(i))); }
    aFilter.resetValues(&badVec4);
    aFilter.resetMagneticField(0.976);

    aFilter.circleFit(pocaPtPhi, pocaR, estimatedCurvature);

    returnValues = aFilter.helixFit();
//    EXPECT_NEAR(returnValues.second.Perp(), estimatedRadius, 0.5); // accepting deviation of 5%
    // particle has a clockwise movement (curvature in direction of flight is bent downwards)
    EXPECT_NEAR(returnValues.first, 1. / estimatedCurvature, 0.5);
//    EXPECT_FLOAT_EQ(14.8262, returnValues.second.Mag());
//    EXPECT_FLOAT_EQ(returnValues.first, estimatedRadius);
//     EXPECT_NEAR(returnValues.first, estimatedRadius, 0.5);


    /// badVec5
//     stringstream HitInfos4;
//     HitInfos4 << "bad p: " << 337657 << "\n";
    hit.SetXYZ(-6.516, -0.930059, -0.153305);
    posInfo.hitPosition = hit;
    posInfo.sigmaU = 0.0160026;
    posInfo.sigmaV = 0.0112734;
    posInfo.hitSigma.SetXYZ(0.0112734, 0.0112734, 0.0160026);
    tbadVec5.push_back(posInfo);
//     HitInfos4 << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaU << " " <<  posInfo.sigmaV << endl;
//    [WARNING] helixFit: strange pVector (Mag=) detected. The following hits were part of this TC:
//  pVector  x/y/z: -337570/-7611.14/906.841  { module: VXDTF }
    hit.SetXYZ(-3.516, -0.862418, -0.143971);
    posInfo.hitPosition = hit;
    posInfo.sigmaU = 0.0160026;
    posInfo.sigmaV = 0.0112734;
    posInfo.hitSigma.SetXYZ(0.0112734, 0.0112734, 0.0160026);
    tbadVec5.push_back(posInfo);
//     HitInfos4 << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaU << " " <<  posInfo.sigmaV << endl;

    hit.SetXYZ(-1.016, -0.806051, -0.137302);
    posInfo.hitPosition = hit;
    posInfo.sigmaU = 0.0160026;
    posInfo.sigmaV = 0.0112734;
    posInfo.hitSigma.SetXYZ(0.0112734, 0.0112734, 0.0160026);
    tbadVec5.push_back(posInfo);
//     HitInfos4 << hit.X() << " " << hit.Y() << " " << hit.Z() << " " << posInfo.sigmaU << " " <<  posInfo.sigmaV << endl;

//     B2INFO("badVec5: " << HitInfos4.str() << endl)

    for (int i = 0; i < int(tbadVec5.size()); ++i) { badVec5.push_back(&(tbadVec5.at(i))); }
    aFilter.resetValues(&badVec5);
    aFilter.resetMagneticField(0.976);

//     returnValues = aFilter.helixFit();
//    EXPECT_FLOAT_EQ(337657, returnValues.second.Mag());
    // hits are practically on a straight line:
    EXPECT_ANY_THROW(aFilter.helixFit());
//     aFilter.circleFit(pocaPtPhi, pocaR, estimatedCurvature);
//     EXPECT_FLOAT_EQ(1./estimatedCurvature, returnValues.first);

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



  /** checking a case where the hits are practically on a line */
  TEST_F(TrackletFiltersTest, TestStrangepVector)
  {
    TrackletFilters aFilter = TrackletFilters();
    PositionInfo hit1, hit2, hit3;

    vector<PositionInfo*> v1;

    hit1.hitPosition = TVector3(-6.516, -0.930059, -0.153305);  hit1.hitSigma = TVector3(.1, .1, .1);
    hit2.hitPosition = TVector3(-3.516, -0.862418, -0.143971);  hit2.hitSigma = TVector3(.1, .1, .1);
    hit3.hitPosition = TVector3(-1.016, -0.806051, -0.137302);  hit3.hitSigma = TVector3(.1, .1, .1);

    v1.push_back(&hit1);
    v1.push_back(&hit2);
    v1.push_back(&hit3);
    aFilter.resetValues(&v1);

    // hits are practically on a straight line:
    EXPECT_ANY_THROW(aFilter.helixFit());
//     EXPECT_LT(1000, aFilter.helixFit().second.Mag());    //The huge value is not so strange, since the hits are approx straight.
  }



  /** test ziggZagg */
  TEST_F(TrackletFiltersTest, TestZiggZagg)
  {
    double near = 1e-15;  //used for EXPECT_NEAR(val1, val2, abs_error);

    TrackletFilters aFilter = TrackletFilters();
    PositionInfo
    hit1, hit2, hit3, hit4, hit5, hit6, hit7,
          hit11, hit12, hit13, hit14, hit15;

    vector<PositionInfo*> v1, v2, v3;

    hit1.hitPosition = TVector3(1., 1., 0.);  hit1.hitSigma = TVector3(.1, .1, .1);
    hit2.hitPosition = TVector3(2., 2., 0.);  hit2.hitSigma = TVector3(.1, .1, .1);
    hit3.hitPosition = TVector3(3., 2.1, 0.); hit3.hitSigma = TVector3(.1, .1, .1);
    hit4.hitPosition = TVector3(4., 3., 0.);  hit4.hitSigma = TVector3(.1, .1, .1);
    hit5.hitPosition = TVector3(5., 3, 0.); hit5.hitSigma = TVector3(.1, .1, .1);
    hit6.hitPosition = TVector3(5., 5., 0.);  hit6.hitSigma = TVector3(.1, .1, .1);
    hit7.hitPosition = TVector3(5., 5.1, 0.); hit7.hitSigma = TVector3(.1, .1, .1);

    v1.push_back(&hit1);
    v1.push_back(&hit2);
    v1.push_back(&hit3);
    v1.push_back(&hit4);

    aFilter.resetValues(&v1);
    EXPECT_TRUE(aFilter.ziggZaggXY());
    EXPECT_TRUE(aFilter.ziggZaggXYWithSigma());

    v1.pop_back(); v1.pop_back();
    v1.push_back(&hit4);        //Now: hits 1, 2, 4
    aFilter.resetValues(&v1);
    EXPECT_FLOAT_EQ(7.1428571, aFilter.simpleLineFit3D().first);
    EXPECT_FLOAT_EQ(1.1888083, aFilter.simpleLineFit3D().second.Mag());
    EXPECT_FLOAT_EQ(0.642857, aFilter.simpleLineFit3D().second[1]);
    EXPECT_FLOAT_EQ(-0.642857, aFilter.simpleLineFit3D(&v1, true, 0).second[1]);
    // y=kx+d
    EXPECT_FLOAT_EQ(0.642857, aFilter.getStraightLineFitResults()[0]);   // k_y
    EXPECT_FLOAT_EQ(0.5, aFilter.getStraightLineFitResults()[1]);    // d_y
    EXPECT_FLOAT_EQ(0, aFilter.getStraightLineFitResults()[2]);    // k_z
    EXPECT_FLOAT_EQ(0, aFilter.getStraightLineFitResults()[3]);    // d_yz

    v1.pop_back();/*v1.pop_back();*/
    v1.push_back(&hit3);
    v1.push_back(&hit4);  v1.push_back(&hit4);    //Now: hits 1, 2, 3, 4, 4

    aFilter.resetValues(&v1);
    EXPECT_FLOAT_EQ(14.911765, aFilter.simpleLineFit3D().first);
    EXPECT_FLOAT_EQ(0.620588, aFilter.getStraightLineFitResults()[0]);    // k_y
    EXPECT_FLOAT_EQ(0.482353, aFilter.getStraightLineFitResults()[1]);    // d_y
    EXPECT_TRUE(aFilter.ziggZaggXY());
    EXPECT_TRUE(aFilter.ziggZaggXYWithSigma());

    v1.pop_back(); v1.pop_back(); v1.pop_back();
    v1.push_back(&hit6);        //Now: hits 1, 2, 6

    aFilter.resetValues(&v1);
    EXPECT_NEAR(0, aFilter.simpleLineFit3D().first, near);

    v1.pop_back();
    v1.push_back(&hit7);        //Now: hits 1, 2, 7
    aFilter.resetValues(&v1);
    EXPECT_FLOAT_EQ(0.03846154, aFilter.simpleLineFit3D().first);
    EXPECT_FLOAT_EQ(1.0269231, aFilter.getStraightLineFitResults()[0]);    // k_y
    EXPECT_FLOAT_EQ(-0.03846154, aFilter.getStraightLineFitResults()[1]);   // d_y

    v2.push_back(&hit1);
    v2.push_back(&hit2);
    v2.push_back(&hit4);
    v2.push_back(&hit5);

    aFilter.resetValues(&v2);
    EXPECT_FALSE(aFilter.ziggZaggXY());
    aFilter.resetValues(&v2);
    EXPECT_FALSE(aFilter.ziggZaggXYWithSigma());

    /** Difference between ZiggZaggs **/
    hit11.hitPosition = TVector3(1.001, 1., 0.);    hit11.hitSigma = TVector3(.1, .1, .1);
    hit12.hitPosition = TVector3(2., 2.0002, 0.);   hit12.hitSigma = TVector3(.1, .1, .1);
    hit13.hitPosition = TVector3(3.0005, 3., 0.);   hit13.hitSigma = TVector3(.1, .1, .1);
    hit14.hitPosition = TVector3(4., 4.0003, 0.);   hit14.hitSigma = TVector3(.1, .1, .1);
    hit15.hitPosition = TVector3(5.001, 5., 0.);    hit15.hitSigma = TVector3(.1, .1, .1);

    v3.push_back(&hit11);
    v3.push_back(&hit12);
    v3.push_back(&hit13);
    v3.push_back(&hit14);
    v3.push_back(&hit15);

    aFilter.resetValues(&v3);
    EXPECT_TRUE(aFilter.ziggZaggXY());
    aFilter.resetValues(&v3);
    EXPECT_FALSE(aFilter.ziggZaggXYWithSigma());  //a case where V2 returns a different value
  }



  /** checks chi2-value of circleFit */
  TEST_F(TrackletFiltersTest, TestQuad)
  {
    double somewherenear = 1e-5;

    TrackletFilters aFilter = TrackletFilters();
    PositionInfo
    hit1, hit2, hit3;

    vector<PositionInfo*> v;

    hit1.hitPosition = TVector3(4. - 4.5, 6., 0.);
    hit1.hitSigma = TVector3(.1, .1, .1);
    hit2.hitPosition = TVector3(4. - 4.5 * cos(5. / 180.*M_PI), 6. + 4.5 * sin(5. / 180.*M_PI), 0.);
    hit2.hitSigma = TVector3(.1, .1, .1);
    hit3.hitPosition = TVector3(4. - 4.5 * cos(10. / 180.*M_PI), 6. + 4.5 * sin(10. / 180.*M_PI), 0.);
    hit3.hitSigma = TVector3(.1, .1, .1);
    v.push_back(&hit1);
    v.push_back(&hit2);
    v.push_back(&hit3);

    double pocaPtPhi, pocaR, curvature;
    aFilter.resetValues(&v);
    EXPECT_NEAR(0, aFilter.circleFit(pocaPtPhi, pocaR, curvature), somewherenear);    // Returns Chi^2

//       B2WARNING( "ClapR " <<pocaR << " ClapPhi " << pocaPtPhi << ", curvature " << curvature );
  }



  /** some cirle-tests */
  TEST_F(TrackletFiltersTest, TestCircles)
  {
    double near = 1e-15;      //used for EXPECT_NEAR(val1, val2, abs_error);
    double somewherenear = 1e-5;

    TrackletFilters aFilter = TrackletFilters();
    PositionInfo
    hit1, hit2, hit4, hit5, hit6, hit7,
          hit21, hit22, hit23, hit24, hit25, hit26, hit27,
          hit31, hit32, hit33, hit34, hit35, hit36, hit37, hit38,
          hit41, hit42, hit43, hit44,
          hit51, hit52, hit53;

    vector<PositionInfo*> v1, v4, v5, v6;

    hit1.hitPosition = TVector3(0., 0., 0.);
    hit1.hitSigma = TVector3(.1, .1, .1); //hits 1 2 4 and 1 2 5 are circles with radius 1 and m=(1,0) in (x,y)
    hit2.hitPosition = TVector3(1., 1., 2.);  hit2.hitSigma = TVector3(.1, .1, .1);
    hit4.hitPosition = TVector3(2., 0, 4.); hit4.hitSigma = TVector3(.1, .1, .1);

    v1.push_back(&hit1);
    v1.push_back(&hit2);
    v1.push_back(&hit4);      //hit 1 2 4

    aFilter.resetValues(&v1);
    EXPECT_NEAR(0, aFilter.helixFit().second[0], near);
    EXPECT_FLOAT_EQ(aFilter.calcPt(1), aFilter.helixFit().second[1]);
    EXPECT_DOUBLE_EQ(-aFilter.calcPt(4. / M_PI), aFilter.helixFit().second[2]); // pz = pt / tan (Theta)
    //on the unit circle, the s-distance from hit (0,0) to (1,1) is Pi/2. The z distance is 2. -> Theta = Atan (Pi/4)

    hit5.hitPosition = TVector3(2., 2., 2.);  hit5.hitSigma = TVector3(.1, .1, .1);
    hit6.hitPosition = TVector3(4., 0, 4.); hit6.hitSigma = TVector3(.1, .1, .1);

    v1.pop_back();
    v1.pop_back();
    v1.push_back(&hit5);
    v1.push_back(&hit6);      //hit 1 5 6

    aFilter.resetValues(&v1);
    EXPECT_DOUBLE_EQ(-aFilter.calcPt(4. / M_PI), aFilter.helixFit().second[2]);

    hit7.hitPosition = TVector3(2.*(1. - 1. / sqrt(2.)), 2. / sqrt(2.), 1.);  hit7.hitSigma = TVector3(.1, .1, .1);

    v1.pop_back();
    v1.pop_back();
    v1.push_back(&hit7);
    v1.push_back(&hit5);      //hit 1 7 5, a circle with m=2 and points seperated by Pi/4

    aFilter.resetValues(&v1);
    EXPECT_FLOAT_EQ(-aFilter.calcPt(4. / M_PI), aFilter.helixFit().second[2]);

    hit21.hitPosition = TVector3(0., 0., 0.);
    hit21.hitSigma = TVector3(.1, .1, .1); //hits 1 2 4 and 1 2 5 are circles with radius 1 and m=(1,0) in (x,y)
    hit22.hitPosition = TVector3(1., 1., 1.); hit22.hitSigma = TVector3(.1, .1, .1);
    hit23.hitPosition = TVector3(.5, 1., 1.8);  hit23.hitSigma = TVector3(.1, .1, .1); //hit 3 is off
    hit24.hitPosition = TVector3(2., 0, 2.);  hit24.hitSigma = TVector3(.1, .1, .1);
    hit25.hitPosition = TVector3(1., 1., -1.);  hit25.hitSigma = TVector3(.1, .1, .1);
    hit26.hitPosition = TVector3(2., 0, -2.); hit26.hitSigma = TVector3(.1, .1, .1);
    hit27.hitPosition = TVector3(23., -14, -2.);  hit27.hitSigma = TVector3(.1, .1, .1);

    v4.push_back(&hit21);
    v4.push_back(&hit22);
    v4.push_back(&hit24);     //hit 1 2 4

    double pocaPtPhi, pocaR, curvature;
    aFilter.resetValues(&v4);
    EXPECT_FLOAT_EQ(66.666666, aFilter.simpleLineFit3D().first);
    EXPECT_NEAR(0, aFilter.circleFit(pocaPtPhi, pocaR, curvature), near);     // Returns Chi^2
    EXPECT_NEAR(0, pocaR, near);
    EXPECT_DOUBLE_EQ(3 * M_PI / 2, pocaPtPhi); // Phi measured from x axis, straight up = Pi/2
    EXPECT_DOUBLE_EQ(-1, curvature);

    EXPECT_DOUBLE_EQ(1, aFilter.helixFit().first);          // Returns the Radius (as of Feb 2014)
    EXPECT_DOUBLE_EQ(0, aFilter.helixFit().second[0]);
    EXPECT_DOUBLE_EQ(aFilter.calcPt(),
                     aFilter.helixFit().second[1]);     // pt*PVector, the unit vector in (x,y); which is (0,1) in our case.
    EXPECT_DOUBLE_EQ(-aFilter.calcPt(2. / M_PI), aFilter.helixFit().second[2]); // pz = pt / tan (Theta)
    //on the unit circle, the s-distance from hit (0,0) to (1,1) is Pi/2. The z distance is 1. -> Theta = Atan (Pi/2)

    v4.pop_back(); v4.pop_back();
    v4.push_back(&hit25);
    v4.push_back(&hit26);     //hit 1 5 6

    aFilter.resetValues(&v4);
    EXPECT_DOUBLE_EQ(aFilter.calcPt() * 2. / M_PI, aFilter.helixFit().second[2]);

    v4.pop_back(); v4.pop_back();
    v4.push_back(&hit23);
    v4.push_back(&hit24);     //hit 1 3 4
    aFilter.resetValues(&v4);
    EXPECT_NEAR(0, aFilter.circleFit(pocaPtPhi, pocaR, curvature), somewherenear);

    v4.push_back(&hit26);     //hit 1 3 4 6
    aFilter.resetValues(&v4);
    EXPECT_NEAR(0, aFilter.circleFit(pocaPtPhi, pocaR, curvature), near);     // Returns Chi^2, Circle (x,y)

    v4.push_back(&hit27);     //hit 1 3 4 6 7; 7 is way off
    aFilter.resetValues(&v4);
    EXPECT_LT(0, aFilter.circleFit(pocaPtPhi, pocaR, curvature));       // Returns Chi^2

    /** v5 */
    hit31.hitPosition = TVector3(-1., 1., 0.);
    hit31.hitSigma = TVector3(.1, .1, .1); // hits 1 2 3 4 ... circle: m=(-3,4), r=2*sqrt(2)
    hit32.hitPosition = TVector3(2 * sqrt(2) - 3, 3, 1.); hit32.hitSigma = TVector3(.1, .1, .1);
    hit33.hitPosition = TVector3(-1., 5., 2.);   hit33.hitSigma = TVector3(.1, .1, .1);
    hit34.hitPosition = TVector3(-3., 2 * sqrt(2) + 3, 3.); hit34.hitSigma = TVector3(.1, .1, .1);

    hit35.hitPosition = TVector3(1., 1., 0.);   hit35.hitSigma = TVector3(.1, .1, .1); // hits 5 6 7 8 ... circle: m=(3,4), r=2*sqrt(2)
    hit36.hitPosition = TVector3(3 - 2 * sqrt(2), 3, 1.); hit36.hitSigma = TVector3(.1, .1, .1); // = circle mirrored along the y-axis
    hit37.hitPosition = TVector3(1., 5., 2.);    hit37.hitSigma = TVector3(.1, .1, .1);
    hit38.hitPosition = TVector3(3., 2 * sqrt(2) + 3, 3.);  hit38.hitSigma = TVector3(.1, .1, .1);

    v5.push_back(&hit31);
    v5.push_back(&hit32);
    v5.push_back(&hit33);
    v5.push_back(&hit34);

    aFilter.resetValues(&v5);

    EXPECT_NEAR(0, aFilter.circleFit(pocaPtPhi, pocaR, curvature), somewherenear);  // Returns Chi^2
    EXPECT_FLOAT_EQ(5 * M_PI / 4, pocaPtPhi);         // Phi of the track
    EXPECT_FLOAT_EQ(2 * sqrt(2), 1. / curvature); //plus for clockwise curved tracks
    EXPECT_FLOAT_EQ(sqrt(2), pocaR);              //same sign as curvature since poca is in same quadrant as circleCenter

    EXPECT_DOUBLE_EQ(2 * sqrt(2), aFilter.helixFit().first);      // Returns the Radius (as of Feb 2014)
    EXPECT_DOUBLE_EQ(aFilter.calcPt(2 * sqrt(2)), aFilter.helixFit().second[0]);
    EXPECT_NEAR(0, aFilter.helixFit().second[1], near);
//   sthg is wrong here. factor rho=radius missing.
    //     EXPECT_FLOAT_EQ(aFilter.calcPt(2*sqrt(2))*2/M_PI, aFilter.helixFit().second[2]); // pz = pt / tan (Theta); z=1, r= radius * Phi == 2*sqrt(2) *Pi/4

    B2WARNING("Test: left Circle, increasing" << "ClapR " << pocaR << " ClapPhi " << pocaPtPhi << ", Radius " << curvature);

    v5.clear();
    v5.push_back(&hit34);
    v5.push_back(&hit33);
    v5.push_back(&hit32);
    v5.push_back(&hit31);

    aFilter.resetValues(&v5);

    EXPECT_NEAR(0, aFilter.circleFit(pocaPtPhi, pocaR, curvature), somewherenear);  // Returns Chi^2
    EXPECT_FLOAT_EQ(-sqrt(2), pocaR);             //minus for counterclockwise curved tracks
    EXPECT_FLOAT_EQ(M_PI / 4, pocaPtPhi);           // Phi of the track
    EXPECT_FLOAT_EQ(-2 * sqrt(2), 1. / curvature);

    EXPECT_FLOAT_EQ(2 * sqrt(2), aFilter.helixFit().first);     //why is it less precise with hits in the other order. (?)
    EXPECT_FLOAT_EQ(aFilter.calcPt(2),
                    aFilter.helixFit().second[0]);     //= aFilter.calcPt(2*sqrt(2)) /sqrt(2), where r=2*sqrt(2) and 1/sqrt(2) comes from the norm
    EXPECT_FLOAT_EQ(aFilter.calcPt(2), aFilter.helixFit().second[1]);
//     //   sthg is wrong here. factor rho=radius missing.
//     EXPECT_FLOAT_EQ(-aFilter.calcPt(1.)*sqrt(2)/M_PI, aFilter.helixFit().second[2]); // pz = pt / tan (Theta)

    B2INFO("Test: left Circle, decreasing" << "pocaR " << pocaR << " pocaPtPhi " << pocaPtPhi << ", curvature " << curvature);

    v5.clear();
    v5.push_back(&hit35);
    v5.push_back(&hit36);
    v5.push_back(&hit37);
    v5.push_back(&hit38);

    aFilter.resetValues(&v5);

    EXPECT_NEAR(0, aFilter.circleFit(pocaPtPhi, pocaR, curvature), somewherenear);  // Returns Chi^2
    EXPECT_FLOAT_EQ(-sqrt(2), pocaR);
    EXPECT_FLOAT_EQ(-M_PI / 4, pocaPtPhi);            // Phi of the track
    EXPECT_FLOAT_EQ(-2 * sqrt(2), 1. / curvature);

    EXPECT_DOUBLE_EQ(2 * sqrt(2), aFilter.helixFit().first);      // Returns the Radius (as of Feb 2014)
    EXPECT_FLOAT_EQ(-aFilter.calcPt(2 * sqrt(2)), aFilter.helixFit().second[0]);
    EXPECT_NEAR(0, aFilter.helixFit().second[1], near);
//   sthg is wrong here. factor rho=radius missing.
    //     EXPECT_FLOAT_EQ(aFilter.calcPt(1.)*sqrt(2)/M_PI, aFilter.helixFit().second[2]);  // pz = pt / tan (Theta)

//      B2WARNING("aFilter.helixFit().second[2]"  <<aFilter.helixFit().second[2] );
    B2INFO("Test: right circle, increasing" << "pocaR " << pocaR << " pocaPtPhi " << pocaPtPhi << ", curvature " << curvature);

    v5.clear();
    v5.push_back(&hit38);
    v5.push_back(&hit37);
    v5.push_back(&hit36);
    v5.push_back(&hit35);

    aFilter.resetValues(&v5);

    EXPECT_FLOAT_EQ(-sqrt(2), pocaR);
    EXPECT_FLOAT_EQ(-M_PI / 4, pocaPtPhi);            // Phi of the track
    EXPECT_FLOAT_EQ(-2 * sqrt(2), 1. / curvature);

    B2INFO("Test: right circle, decreasing" << "pocaR " << pocaR << " pocaPtPhi " << pocaPtPhi << ", curvature " << curvature);

    hit51.hitPosition = TVector3(-1., 0., 0.);
    hit51.hitSigma = TVector3(.1, .1, .1); //hits 1 2 3 = circle with radius 2 and m=(1,0) in (x,y)
    hit52.hitPosition = TVector3(1., 2., 1.); hit52.hitSigma = TVector3(.1, .1, .1);
    hit53.hitPosition = TVector3(3., 0., 2.); hit53.hitSigma = TVector3(.1, .1, .1);

    v5.clear();
    v5.push_back(&hit51);
    v5.push_back(&hit52);
    v5.push_back(&hit53);

    aFilter.resetValues(&v5);

    EXPECT_NEAR(0, aFilter.circleFit(pocaPtPhi, pocaR, curvature), somewherenear);  // Returns Chi^2
//     B2WARNING("circle with radius 2 and m=(1,0) in (x,y) " << "ClapR " <<pocaR << " ClapPhi " << pocaPtPhi << ", Radius " << radius );

    v5.clear();
    v5.push_back(&hit53);
    v5.push_back(&hit52);
    v5.push_back(&hit51);

    aFilter.resetValues(&v5);

    EXPECT_NEAR(0, aFilter.circleFit(pocaPtPhi, pocaR, curvature), somewherenear);  // Returns Chi^2
//     B2WARNING("circle with radius 2 and m=(1,0) counterclockwise " << "ClapR " <<pocaR << " ClapPhi " << pocaPtPhi << ", Radius " << radius );

    hit51.hitPosition = TVector3(1., 0., 0.);
    hit51.hitSigma = TVector3(.1, .1, .1); //hits 1 2 3 = circle with radius 2 and m=(3,0) in (x,y)
    hit52.hitPosition = TVector3(3., 2., 1.); hit52.hitSigma = TVector3(.1, .1, .1);
    hit53.hitPosition = TVector3(5., 0., 2.); hit53.hitSigma = TVector3(.1, .1, .1);

    v5.clear();
    v5.push_back(&hit51);
    v5.push_back(&hit52);
    v5.push_back(&hit53);

    aFilter.resetValues(&v5);

    EXPECT_NEAR(0, aFilter.circleFit(pocaPtPhi, pocaR, curvature), somewherenear);  // Returns Chi^2
//     B2WARNING("circle with radius 2 and m=(3,0) in (x,y) " << "ClapR " <<pocaR << " ClapPhi " << pocaPtPhi << ", Radius " << radius );

    v5.clear();
    v5.push_back(&hit53);
    v5.push_back(&hit52);
    v5.push_back(&hit51);

    aFilter.resetValues(&v5);

    EXPECT_NEAR(0, aFilter.circleFit(pocaPtPhi, pocaR, curvature), somewherenear);  // Returns Chi^2
//     B2WARNING("circle with radius 2 and m=(3,0) counterclockwise " << "ClapR " <<pocaR << " ClapPhi " << pocaPtPhi << ", Radius " << radius );

    hit51.hitPosition = TVector3(-5., 0., 0.);
    hit51.hitSigma = TVector3(.1, .1, .1); //hits 1 2 3 = circle with radius 2 and m=(-3,0) in (x,y)
    hit52.hitPosition = TVector3(-3., 2., 1.);  hit52.hitSigma = TVector3(.1, .1, .1);
    hit53.hitPosition = TVector3(-1., 0., 2.);  hit53.hitSigma = TVector3(.1, .1, .1);

    v5.clear();
    v5.push_back(&hit51);
    v5.push_back(&hit52);
    v5.push_back(&hit53);

    aFilter.resetValues(&v5);

    EXPECT_NEAR(0, aFilter.circleFit(pocaPtPhi, pocaR, curvature), somewherenear);  // Returns Chi^2
//     B2WARNING("circle with radius 2 and m=(-3,0) in (x,y)" << "ClapR " <<pocaR << " ClapPhi " << pocaPtPhi << ", Radius " << radius );

    v5.clear();
    v5.push_back(&hit53);
    v5.push_back(&hit52);
    v5.push_back(&hit51);

    aFilter.resetValues(&v5);

    EXPECT_NEAR(0, aFilter.circleFit(pocaPtPhi, pocaR, curvature), somewherenear);  // Returns Chi^2
//     B2WARNING("circle with radius 2 and m=(-3,0) counterclockwise " << "ClapR " <<pocaR << " ClapPhi " << pocaPtPhi << ", Radius " << radius );

    /** Circle Fit produces nan <-> m= (0,0) */
    B2WARNING("Circle Fit produces nan <-> m= (0,0)");    //TODO decide if m= (0,0) can happen in any real life situation -- probably not.
    hit51.hitPosition = TVector3(-1., 0., 0.);
    hit51.hitSigma = TVector3(.1, .1, .1); //hits 1 2 3 = circle with radius 1 and m=(0,0) in (x,y)
    hit52.hitPosition = TVector3(0., 1., 1.); hit52.hitSigma = TVector3(.1, .1, .1);
    hit53.hitPosition = TVector3(1., 0., 2.); hit53.hitSigma = TVector3(.1, .1, .1);

    v5.clear();
    v5.push_back(&hit51);
    v5.push_back(&hit52);
    v5.push_back(&hit53);

    aFilter.resetValues(&v5);

    EXPECT_ANY_THROW(aFilter.circleFit(pocaPtPhi, pocaR, curvature));
    EXPECT_ANY_THROW(aFilter.helixFit().second[0]);
  }



  /**MUST produce errors: y=kx+d ... k=inf, d=... .
   *The second case is y=0, so a possible source of errors if the coordinates get changed.*/
  TEST_F(TrackletFiltersTest, TestLineFitStraightUp)
  {
    TrackletFilters aFilter = TrackletFilters();
    PositionInfo
    hit1, hit2, hit3,
          hit11, hit12, hit13;

    hit1.hitPosition = TVector3(0., 1., 0.);  hit1.hitSigma = TVector3(.1, .1, .1);
    hit2.hitPosition = TVector3(0., 2., 0.);  hit2.hitSigma = TVector3(.1, .1, .1);
    hit3.hitPosition = TVector3(0., 3., 0.);  hit3.hitSigma = TVector3(.1, .1, .1); // problematic for y=kx+d

    vector<PositionInfo*> v1, v2;

    v1.push_back(&hit1);
    v1.push_back(&hit2);
    v1.push_back(&hit3);
    aFilter.resetValues(&v1);       //hits 1 2 3

    EXPECT_ANY_THROW(aFilter.simpleLineFit3D()/*.first*/);

    hit11.hitPosition = TVector3(1., 0., 0.); hit11.hitSigma = TVector3(.1, .1, .1);
    hit12.hitPosition = TVector3(2., 0., 0.); hit12.hitSigma = TVector3(.1, .1, .1);
    hit13.hitPosition = TVector3(3., 0., 0.); hit13.hitSigma = TVector3(.1, .1, .1);

    v2.push_back(&hit11);
    v2.push_back(&hit12);
    v2.push_back(&hit13);
    aFilter.resetValues(&v2);       //hits 1 2 3

    EXPECT_FLOAT_EQ(0., aFilter.simpleLineFit3D().first);
    EXPECT_FLOAT_EQ(0., aFilter.getStraightLineFitResults()[0]);    // k_y
    EXPECT_FLOAT_EQ(0., aFilter.getStraightLineFitResults()[1]);    // d_y
  }



  /** testing straight line fit for different cases*/
  TEST_F(TrackletFiltersTest, straightLineFitterTest)
  {
    TrackletFilters aFilter = TrackletFilters(); // simpleLineFit3D(const std::vector<PositionInfo*>* hits, useBackwards, )

    vector<PositionInfo>
    tLine1, // short line (3 hits, smeared in y,z)
    tLine2, // long line (6 hits, smeared in y,z)
    tLine3, // medium line (5 hits, smeared in x, z)
    tLine4, // medium line (5 hits, smeared in x,y, z)
    tLine5, // same as line1, but y & z values switched
    tLine6; // same as line2, but y & z values switched

    vector<PositionInfo*>  line1(3), line2(6), line3(5), line4(5), line5(3), line6(6); // pointer to upper entries (needed for filter)

    //http://stackoverflow.com/questions/9835560/c-vector-of-set-gives-segmentation-fault-after-performing-push-back
    //When you push_back into your vector you invalidate all references to elements in it in the case the vector needs to allocate more memory.
    //-> so we give them their size and never push_back.

    double sigmaY = 0.008, sigmaZ = 0.008; // sigma for smearing, x and y are using sigmaY, z is using sigmaZ
    double slope1to4Y = 1.1, slope5to8Y = 0.1, slope9to13Y = 85,
           intercept1to4Y = 0.1, intercept5to8Y = -3, intercept9to13Y = 10,
           slope1to4Z = 3, slope5to8Z = 42, slope9to13Z = 0.5,
           intercept1to4Z = 0, intercept5to8Z = 23,
           intercept9to13Z =
             -2; // 1to4, 5to8 and 9to13 are all using line1-4, using different 3D-lines for testing the same setup subsequently

    // now storing values in an array for fast access (upper lines are simply for better readability)
    std::array<double, 6> parametersY { { slope1to4Y, slope5to8Y, slope9to13Y, intercept1to4Y, intercept5to8Y, intercept9to13Y } };
    std::array<double, 6> parametersZ { { slope1to4Z, slope5to8Z, slope9to13Z, intercept1to4Z, intercept5to8Z, intercept9to13Z } };
    unsigned int testCase = 0; // steering lambda-function
//         bool useY = true; // if true, use y-case, else use z-case
    double xVal = 0; // used xValue
    std::array<double, 6> xValues { { -7.7, -4.5, -2.3, 0., 1.3, 2.8} }; // carries xValues
    PositionInfo thisPos;
    thisPos.hitSigma.SetXYZ(sigmaY, sigmaY, sigmaZ);
    pair<double, TVector3> chi2AndDirection;
    vector<double> estFitParams;


    // doing  small lambdas for better readability: calcVal, clearVecs and fillValues
    auto calcVal = [&](bool useY) -> double { /// c++11 lambda function...
      double val = 0;
      if (useY == true)
      {
        val = xVal * parametersY.at(testCase) + parametersY.at(testCase + 3); // y=a*x+b
        val = gRandom->Gaus(val, sigmaY);
      } else { // -> useZ
        val = xVal * parametersZ.at(testCase) + parametersZ.at(testCase + 3); // z=c*x+d
        val = gRandom->Gaus(val, sigmaZ);
      }
      return val;
    }; /**< calculates value to estimate. Using parameters testCase for chosing slope and intercept, useY for chosing case of y or z fit */


    auto clearVecs = [&]() {  /// c++11 lambda function...
      //line1.clear(); line2.clear(); line3.clear(); line4.clear(); line5.clear(); line6.clear();
      tLine1.clear(); tLine2.clear(); tLine3.clear(); tLine4.clear(); tLine5.clear(); tLine6.clear();
    }; /**< clears vectors before use */


    auto fillValues = [&]() {
      clearVecs();

      for (int i = 0; i < 3; ++i) {
        xVal = xValues[i]; //xValues.at(i);
        thisPos.hitPosition.SetXYZ(xVal, calcVal(true), calcVal(false));
        tLine1.push_back(thisPos);

        thisPos.hitPosition.SetXYZ(xVal, calcVal(false), calcVal(true));
        tLine5.push_back(thisPos);
      }

      for (int i = 0; i < 3; ++i) {
        line1[i] = &tLine1[i];
        line5[i] = &tLine5[i];

//    for(int j=0;j<3;j++)
//    {
//      B2WARNING("Line1 push back: "<< j <<": " << line1[i]->hitPosition[j]<<" sigma: "<<line1[i]->hitSigma[j]);
//    B2WARNING("Line5 push back: "<< j <<": " << line5[i]->hitPosition[j]<<" sigma: "<<line5[i]->hitSigma[j]);
//    }
      }

      for (int i = 0; i < 6; ++i) {
        xVal = xValues.at(i);
        thisPos.hitPosition.SetXYZ(xVal, calcVal(true), calcVal(false));
        tLine2.push_back(thisPos);

        thisPos.hitPosition.SetXYZ(xVal, calcVal(false), calcVal(true));
        tLine6.push_back(thisPos);
      }
      for (int i = 0; i < 6; ++i) {
        line2[i] = &tLine2[i];
        line6[i] = &tLine6[i];
//    for(int j=0;j<3;j++)
//    {
//      B2WARNING("Line2 push back: "<< j <<": " << line2[i]->hitPosition[j]<<" sigma: "<<line2[i]->hitSigma[j]);
//    B2WARNING("Line6 push back: "<< j <<": " << line6[i]->hitPosition[j]<<" sigma: "<<line6[i]->hitSigma[j]);
//    }
      }
      for (int i = 0; i < 5; ++i) {
        xVal = xValues.at(i);
        thisPos.hitPosition.SetXYZ(xVal, calcVal(true), calcVal(false));
        tLine3.push_back(thisPos);

        xVal = gRandom->Gaus(xValues.at(i), sigmaY);
        thisPos.hitPosition.SetXYZ(xVal, calcVal(true), calcVal(false));
        tLine4.push_back(thisPos);
      }
      for (int i = 0; i < 5; ++i) {
        line3[i] = &tLine3[i];
        line4[i] = &tLine4[i];
//    for(int j=0;j<3;j++)
//    {
//      B2WARNING("Line3 push back: "<< j <<": " << line3[i]->hitPosition[j]<<" sigma: "<<line3[i]->hitSigma[j]);
//    B2WARNING("Line4 push back: "<< j <<": " << line4[i]->hitPosition[j]<<" sigma: "<<line4[i]->hitSigma[j]);
//    }
      }
    }; /**< fills all the vectors with randomized data. */


    auto testGenericValues = [&]() {
      EXPECT_NEAR(parametersY.at(testCase), estFitParams.at(0), 0.5); // accepting deviation of 5%
      EXPECT_NEAR(parametersZ.at(testCase), estFitParams.at(2), 0.5);
      EXPECT_NEAR(parametersY.at(testCase + 3), estFitParams.at(1), 0.5);
      EXPECT_NEAR(parametersZ.at(testCase + 3), estFitParams.at(3), 0.5);
      EXPECT_NEAR(parametersY.at(testCase), estFitParams.at(0), 0.5); // accepting deviation of 5%
      EXPECT_NEAR(parametersZ.at(testCase), estFitParams.at(2), 0.5);
      EXPECT_NEAR(parametersY.at(testCase + 3), estFitParams.at(1), 0.5);
      EXPECT_NEAR(parametersZ.at(testCase + 3), estFitParams.at(3), 0.5);
      EXPECT_NEAR(parametersY[testCase], estFitParams[0],
                  0.05 * (1. + abs(parametersY[testCase]))); // accepting deviation of 5% plus, for small paramters + 0.05, since 0.05*0=0
      EXPECT_NEAR(parametersZ[testCase], estFitParams[2], 0.05 * (1. + abs(parametersZ[testCase])));
      EXPECT_NEAR(parametersY[testCase + 3], estFitParams[1], 0.05 * (1. + abs(parametersY[testCase + 3])));
      EXPECT_NEAR(parametersZ[testCase + 3], estFitParams[3], 0.05 * (1. + abs(parametersZ[testCase + 3])));
    }; /**< does the general part for the following testing routine... */


    auto testGenericValuesZY = [&]() {      /** Y <--> Z*/
      EXPECT_NEAR(parametersZ.at(testCase), estFitParams.at(0), 0.5); // accepting deviation of 5%
      EXPECT_NEAR(parametersY.at(testCase), estFitParams.at(2), 0.5);
      EXPECT_NEAR(parametersZ.at(testCase + 3), estFitParams.at(1), 0.5);
      EXPECT_NEAR(parametersY.at(testCase + 3), estFitParams.at(3), 0.5);
      EXPECT_NEAR(parametersZ.at(testCase), estFitParams.at(0), 0.5); // accepting deviation of 5%
      EXPECT_NEAR(parametersY.at(testCase), estFitParams.at(2), 0.5);
      EXPECT_NEAR(parametersZ.at(testCase + 3), estFitParams.at(1), 0.5);
      EXPECT_NEAR(parametersY.at(testCase + 3), estFitParams.at(3), 0.5);
      EXPECT_NEAR(parametersZ[testCase], estFitParams[0],
                  0.05 * (1. + abs(parametersY[testCase]))); // accepting deviation of 5% plus, for small paramters + 0.05, since 0.05*0=0
      EXPECT_NEAR(parametersY[testCase], estFitParams[2], 0.05 * (1. + abs(parametersZ[testCase])));
      EXPECT_NEAR(parametersZ[testCase + 3], estFitParams[1], 0.05 * (1. + abs(parametersY[testCase + 3])));
      EXPECT_NEAR(parametersY[testCase + 3], estFitParams[3], 0.05 * (1. + abs(parametersZ[testCase + 3])));
    }; /**< does the general part for the following testing routine - part II */


    auto testValues = [&]() {
//     B2WARNING("testValues");
      aFilter.resetValues(&line1);
      chi2AndDirection = aFilter.simpleLineFit3D();
      estFitParams = aFilter.getStraightLineFitResults();
      testGenericValues();
//             B2WARNING("lineFit: case1: slopeY, slopeZ, interceptY, interceptZ, estSlY, estSlZ, estIntY, estIntZ, chi2: \n"<<"\t" << parametersY.at(testCase) <<"/"<< parametersZ.at(testCase) <<"/"<< parametersY.at(testCase+3) <<"/"<< parametersZ.at(testCase+3) <<"/"<< estFitParams.at(0) <<"/"<< estFitParams.at(2) <<"/"<< estFitParams.at(1) <<"/"<< estFitParams.at(3) <<"/"<<chi2AndDirection.first)

      aFilter.resetValues(&line2);
      chi2AndDirection = aFilter.simpleLineFit3D();
      estFitParams = aFilter.getStraightLineFitResults();
      testGenericValues();
//             B2WARNING("lineFit: case2: slopeY, slopeZ, interceptY, interceptZ, estSlY, estSlZ, estIntY, estIntZ, chi2: \n"<<"\t" << parametersY.at(testCase) <<"/"<< parametersZ.at(testCase) <<"/"<< parametersY.at(testCase+3) <<"/"<< parametersZ.at(testCase+3) <<"/"<< estFitParams.at(0) <<"/"<< estFitParams.at(2) <<"/"<< estFitParams.at(1) <<"/"<< estFitParams.at(3) <<"/"<<chi2AndDirection.first)

      aFilter.resetValues(&line3);
      chi2AndDirection = aFilter.simpleLineFit3D();
      estFitParams = aFilter.getStraightLineFitResults();
      testGenericValues();
//             B2WARNING("lineFit: case3: slopeY, slopeZ, interceptY, interceptZ, estSlY, estSlZ, estIntY, estIntZ, chi2: \n"<<"\t" << parametersY.at(testCase) <<"/"<< parametersZ.at(testCase) <<"/"<< parametersY.at(testCase+3) <<"/"<< parametersZ.at(testCase+3) <<"/"<< estFitParams.at(0) <<"/"<< estFitParams.at(2) <<"/"<< estFitParams.at(1) <<"/"<< estFitParams.at(3) <<"/"<<chi2AndDirection.first)

      aFilter.resetValues(&line4);
      chi2AndDirection = aFilter.simpleLineFit3D();
      estFitParams = aFilter.getStraightLineFitResults();
      testGenericValues();
//             B2WARNING("lineFit: case4: slopeY, slopeZ, interceptY, interceptZ, estSlY, estSlZ, estIntY, estIntZ, chi2: \n"<<"\t" << parametersY.at(testCase) <<"/"<< parametersZ.at(testCase) <<"/"<< parametersY.at(testCase+3) <<"/"<< parametersZ.at(testCase+3) <<"/"<< estFitParams.at(0) <<"/"<< estFitParams.at(2) <<"/"<< estFitParams.at(1) <<"/"<< estFitParams.at(3) <<"/"<<chi2AndDirection.first)

      aFilter.resetValues(&line5);
      chi2AndDirection = aFilter.simpleLineFit3D();
      estFitParams = aFilter.getStraightLineFitResults();
      testGenericValuesZY();
//             B2WARNING("lineFit: case5: slopeY, slopeZ, interceptY, interceptZ, estSlY, estSlZ, estIntY, estIntZ, chi2: \n"<<"\t" << parametersY.at(testCase) <<"/"<< parametersZ.at(testCase) <<"/"<< parametersY.at(testCase+3) <<"/"<< parametersZ.at(testCase+3) <<"/"<< estFitParams.at(0) <<"/"<< estFitParams.at(2) <<"/"<< estFitParams.at(1) <<"/"<< estFitParams.at(3) <<"/"<<chi2AndDirection.first)

      aFilter.resetValues(&line6);
      chi2AndDirection = aFilter.simpleLineFit3D();
      estFitParams = aFilter.getStraightLineFitResults();
      testGenericValuesZY();
//             B2WARNING("lineFit: case6: slopeY, slopeZ, interceptY, interceptZ, estSlY, estSlZ, estIntY, estIntZ, chi2: \n"<<"\t" << parametersY.at(testCase) <<"/"<< parametersZ.at(testCase) <<"/"<< parametersY.at(testCase+3) <<"/"<< parametersZ.at(testCase+3) <<"/"<< estFitParams.at(0) <<"/"<< estFitParams.at(2) <<"/"<< estFitParams.at(1) <<"/"<< estFitParams.at(3) <<"/"<<chi2AndDirection.first)
    }; /**< using freshly filled vectors for straight line fit */

    /// finally executing that stuff...
    // iteration 1 to 4:
    testCase = 0;
    fillValues();
    testValues();

    // iteration 5 to 8:
    testCase = 1;
    fillValues();
    testValues();

    // iteration 9 to 13:
    testCase = 2;
    fillValues();
    testValues();
  }



  /** some more extensive tests of the helix- and circleFitter */
  TEST_F(TrackletFiltersTest, TestHelixFitAndCircleFit)
  {
    double near = 1e-15;
    double somewherenear = 1e-3;
    double phiVal = 0, pocaR = 0, curvature = 0;

    TrackletFilters aFilter = TrackletFilters();
    PositionInfo
    hit1, hit2, hit3, hit4,
          hit5, hit6, hit7;

    hit1.hitPosition = TVector3(3.00001, -1.9999, 0.);
    hit1.hitSigma = TVector3(.01, .01, .01); //near perfect circle with m= (3,1), r=3
    hit2.hitPosition = TVector3(-0.00001, 1.00001, 0.); hit2.hitSigma = TVector3(.01, .01, .01);
    hit3.hitPosition = TVector3(3 * (1 - 1 / sqrt(2)), (1 + 3. / sqrt(2)), 0.); hit3.hitSigma = TVector3(.01, .01, .01);
    hit4.hitPosition = TVector3(2.999998, 3.999999, 0.);  hit4.hitSigma = TVector3(.01, .01, .01);

    vector<PositionInfo*> v1;

    v1.push_back(&hit1);
    v1.push_back(&hit2);
    v1.push_back(&hit3);
    v1.push_back(&hit4);
    aFilter.resetValues(&v1);       //hits 1 2 3 4

    EXPECT_NEAR(3, aFilter.helixFit().first, somewherenear);

    aFilter.resetValues(&v1);
    EXPECT_FLOAT_EQ(-aFilter.calcPt(), aFilter.helixFit().second[0]);
    EXPECT_NEAR(0, aFilter.helixFit().second[1], somewherenear);
    EXPECT_NEAR(0, aFilter.helixFit().second[2], near); // pz = pt / tan (Theta)

    aFilter.resetValues(&v1);
    EXPECT_NEAR(0, aFilter.circleFit(phiVal, pocaR, curvature), somewherenear); // Chi^2
    EXPECT_NEAR(-sqrt(10) + 3, pocaR, somewherenear); //minus for counterclockwise curved tracks
    EXPECT_FLOAT_EQ(-1.2490274, phiVal);        // Phi of the track at closest approach
    EXPECT_NEAR(-1. / 3., curvature, somewherenear);

    aFilter.resetValues(&v1);
    EXPECT_NEAR(aFilter.calcPt(3), aFilter.calcPt(), somewherenear);

    aFilter.resetValues(&v1);
    EXPECT_NEAR(aFilter.calcPt(3), aFilter.helixFit().second.Mag(), somewherenear);
    aFilter.resetValues(&v1);
    EXPECT_NEAR(-aFilter.calcPt(), aFilter.helixFit().second[0], somewherenear);
    aFilter.resetValues(&v1);
    EXPECT_NEAR(0, aFilter.helixFit().second[1],
                somewherenear); // at the point where we want to calculate the Momentum vector, the vector points parallel to the x-axis

    aFilter.resetValues(&v1);
    EXPECT_NEAR(0, aFilter.helixFit().second[1], somewherenear);
    aFilter.resetValues(&v1);
    EXPECT_NEAR(0, aFilter.helixFit().second[2], near); // pz = pt / tan (Theta)

    aFilter.resetValues(&v1);
    EXPECT_NEAR(0, aFilter.circleFit(phiVal, pocaR, curvature), somewherenear); // Chi^2
    aFilter.resetValues(&v1);
    EXPECT_NEAR(-sqrt(10) + 3, pocaR, somewherenear); //minus for counterclockwise curved tracks
    aFilter.resetValues(&v1);
    EXPECT_FLOAT_EQ(-1.2490274, phiVal);  // Phi of the track at closest approach

    /// curvature carries information whether particle flies clockwise or counterclockwise (negative sign means counterClockwise direction of flight (direction of flight is moving from last entry to the first entry of the added hits))
    EXPECT_NEAR(-3, 1. / curvature, somewherenear);

//    hit1.hitPosition=TVector3(3.00001, -1.9999, 0.);  hit1.hitSigma=TVector3(.01, .01, .01);  //near perfect circle with m= (3,1), r=3
//     hit2.hitPosition=TVector3(-0.00001, 1.00001, 0.);  hit2.hitSigma=TVector3(.01, .01, .01);
//     hit3.hitPosition=TVector3(3*(1-1/sqrt(2)), (1+3./sqrt(2)), 0.);  hit3.hitSigma=TVector3(.01, .01, .01);
//     hit4.hitPosition=TVector3(2.999998, 3.999999, 0.); hit4.hitSigma=TVector3(.01, .01, .01);

    vector<PositionInfo*> v2, v3;

    v2.push_back(&hit1);
    v2.push_back(&hit2);
    v2.push_back(&hit3);
    v2.push_back(&hit4);
    aFilter.resetValues(&v2);       //hits 1 2 3 4
    aFilter.circleFit(phiVal, pocaR, curvature);
    aFilter.resetValues(&v2);       //hits 1 2 3 4

    pair<double, TVector3> results = aFilter.circleFit();
    EXPECT_NEAR(0, results.first, somewherenear); // Returns Chi^2
    EXPECT_NEAR(aFilter.calcPt(-1. / curvature), results.second.Mag(), somewherenear);
    EXPECT_NEAR(aFilter.calcPt(-1. / curvature), results.second.Perp(), somewherenear);
    EXPECT_NEAR(aFilter.calcPt(-1. / curvature), results.second.X(), somewherenear);
    EXPECT_NEAR(aFilter.calcPt(3), results.second.Mag(), somewherenear);
    EXPECT_NEAR(aFilter.calcPt(3), results.second.Perp(), somewherenear);
    EXPECT_NEAR(aFilter.calcPt(3), results.second[0], somewherenear);
    B2WARNING("results pt: X/Y/Z: " << results.second[0] << "/" << results.second[1] << "/" << results.second[2]);

    v3 = v2;
    std::reverse(v3.begin(), v3.end());
    aFilter.resetValues(&v3);       //hits 4 3 2 1
    aFilter.circleFit(phiVal, pocaR, curvature);
    aFilter.resetValues(&v3);       //hits 1 2 3 4

    pair<double, TVector3> resultsReverse = aFilter.circleFit();
    EXPECT_NEAR(0, resultsReverse.first, somewherenear); // Returns Chi^2
    EXPECT_NEAR(aFilter.calcPt(1. / curvature), resultsReverse.second.Mag(), somewherenear);
    EXPECT_NEAR(aFilter.calcPt(1. / curvature), resultsReverse.second.Perp(), somewherenear);
    EXPECT_NEAR(aFilter.calcPt(1. / curvature), resultsReverse.second.X(), somewherenear);
    EXPECT_NEAR(aFilter.calcPt(3), resultsReverse.second.Mag(), somewherenear);
    EXPECT_NEAR(aFilter.calcPt(3), resultsReverse.second.Perp(), somewherenear);
    EXPECT_NEAR(aFilter.calcPt(3), resultsReverse.second[0], somewherenear);
    B2WARNING("resultsReverse pt: X/Y/Z: " << resultsReverse.second[0] << "/" << resultsReverse.second[1] << "/" <<
              resultsReverse.second[2]);
//    The difference between aFilter.calcPt(3) and results.second.X() is 0.0074555377781446441, which exceeds somewherenear, where
// aFilter.calcPt(3) evaluates to 0.013486950000000001,
// results.second.X() evaluates to 0.0060314122218553569, and
//
// The difference between aFilter.calcPt(3) and results.second.X() is 0.0074555377781446441, which exceeds somewherenear, where
// aFilter.calcPt(3) evaluates to 0.013486950000000001,
// results.second.X() evaluates to 0.0060314122218553569, and

    /** Now we fill a straight line to expect a throw */
    hit5.hitPosition = TVector3(1., 1., 0.);  hit5.hitSigma = TVector3(.1, .1, .1); //a straight line
    hit6.hitPosition = TVector3(2., 2., 0.);  hit6.hitSigma = TVector3(.1, .1, .1);
    hit7.hitPosition = TVector3(3., 3., 0.);  hit7.hitSigma = TVector3(.1, .1, .1);

    v1.clear();
    v1.push_back(&hit5);
    v1.push_back(&hit6);
    v1.push_back(&hit7);
    aFilter.resetValues(&v1);       //hits 5 6 7

    EXPECT_ANY_THROW(aFilter.helixFit()/*.first*/);

    hit5.hitPosition = TVector3(1., 1., 0.);  hit5.hitSigma = TVector3(.1, .1, .1); //a straight line with slope 0
    hit6.hitPosition = TVector3(2., 1., 0.);  hit6.hitSigma = TVector3(.1, .1, .1);
    hit7.hitPosition = TVector3(3., 1., 0.);  hit7.hitSigma = TVector3(.1, .1, .1);

    v1.clear();
    v1.push_back(&hit5);
    v1.push_back(&hit6);
    v1.push_back(&hit7);
    aFilter.resetValues(&v1);       //hits 5 6 7

    EXPECT_ANY_THROW(aFilter.helixFit()/*.first*/);

    hit5.hitPosition = TVector3(1., 1., 0.);  hit5.hitSigma = TVector3(.91, .1, .1); //a straight line with slope 0
    hit6.hitPosition = TVector3(2.2, 1., 0.); hit6.hitSigma = TVector3(.1, .21, .1);
    hit7.hitPosition = TVector3(3., 1., 0.);  hit7.hitSigma = TVector3(.1, .1, .11);

    v1.clear();
    v1.push_back(&hit5);
    v1.push_back(&hit6);
    v1.push_back(&hit7);
    aFilter.resetValues(&v1);       //hits 5 6 7

    EXPECT_ANY_THROW(aFilter.helixFit()/*.first*/);
  }



  /** test helixFit where points move in xy */
  TEST_F(TrackletFiltersTest, TestHelixInXY)
  {
    TrackletFilters aFilter = TrackletFilters();

    PositionInfo hit1, hit2, hit3;
    double sigma = 1e-1;
    hit1.hitSigma = TVector3(sigma, sigma, sigma);
    hit2.hitSigma = TVector3(sigma, sigma, sigma);
    hit3.hitSigma = TVector3(sigma, sigma, sigma);
    hit1.hitPosition = TVector3(1,  0,  0.);
    hit3.hitPosition = TVector3(201,  200,  2.);

    //for(int r=2;i<22;i++){
    vector<PositionInfo*> v;

//     ofstream raus("HelixNansOverXY.out");
//       raus<<"r"<<"\t"<<"n"<<endl;

    // r=10^rpow
    for (int x = -10; x < 11; x++) {
      for (int y = -10; y < 11; y++) {

        hit1.hitPosition = TVector3(x / 10., y / 10., 0.); //points move in xy, and the circle is of radius 800, with hits seperated by 2°
        hit3.hitPosition = TVector3(x / 10. + 0.487338384723415995005247964856, y / 10. + 27.9195973620007773167961453003, 1.);
        hit2.hitPosition = TVector3(x / 10. + 1.94875979214060190946985548460, y / 10. + 55.8051789953002406207670681553, 2.);
        v.clear();
        v.push_back(&hit3);
        v.push_back(&hit2);
        v.push_back(&hit1);
        aFilter.resetValues(&v);

        EXPECT_FLOAT_EQ(800, aFilter.helixFit().first);

        //     if(std::isnan(aFilter.helixFit().second[2]))
        //       {raus<<1<<"\t"<<x<<"\t"<<y<<endl;} //<<aFilter.helixFit().second[2]<<"\t"<<hit1.hitPosition[0]<<"\t"<<hit1.hitPosition[1]<<"\t"<<hit1.hitPosition[2]<<"\t"<<hit2.hitPosition[0]<<"\t"<<hit2.hitPosition[1]<<"\t"<<hit2.hitPosition[2]<<"\t"<<hit3.hitPosition[0]<<"\t"<<hit3.hitPosition[1]<<"\t"<<hit3.hitPosition[2]<<endl;}
        //     else
        //       {raus<<0<<"\t"<<x<<"\t"<<y<<endl;} //<<aFilter.helixFit().second[2]<<"\t"<<hit1.hitPosition[0]<<"\t"<<hit1.hitPosition[1]<<"\t"<<hit1.hitPosition[2]<<"\t"<<hit2.hitPosition[0]<<"\t"<<hit2.hitPosition[1]<<"\t"<<hit2.hitPosition[2]<<"\t"<<hit3.hitPosition[0]<<"\t"<<hit3.hitPosition[1]<<"\t"<<hit3.hitPosition[2]<<endl;}
      }
    }
  }



  /** testing behavior of circleFit in different cases:
   * Do not forget that the last hit will be treated as the first hit in direction of flight!
   * Case1: clockwise, pTVector and poca-vector form right handed system
   * Case2: clockwise, pTVector and poca-vector form right handed system
   * Case3: clockwise, pTVector and poca-vector form left handed system
   * Case4: clockwise, pTVector and poca-vector form left handed system
   * Case5-8: case 5  = case1 reversed, case 6 = case2 reversed , ...
   * */
  TEST_F(TrackletFiltersTest, TestCircleFitBehaviorNonSmearedInput)
  {
    // initial definitions:
    using namespace boost::assign; // allows adding more than one entry at once for vectors
    typedef pair<double, TVector3> FitResult;

    struct CircleCollection {
      vector <pair<vector<PositionInfo*> , TrackletFilters> > myQuadrantsAndFilters;
      float clockwise; // 1 if clockwise, -1 if not
      float rightHanded; // 1 if right handed, -1 if left handed
      TVector3 center;
      TVector3 poca;
    }; /**< stores Info about circles */


    // hits of four different circles for that case sharing the same point of closest approach, Cc = circleCenter, POCA = point of closest approach, will use hits of quadrant I & IV for circle1 and hits of quadrant II & III
    TVector3 Cc2(4, 2, 0), POCA12(-2, -1, 0), Cc1(-8, -4, 0);
    TVector3 Cc3(-4, 2, 0), POCA34(2, -1, 0), Cc4(8, -4, 0);
    vector<TVector3> circleCenters = {Cc1, Cc2, Cc3, Cc4 };
    vector<TVector3> circlePocas = {POCA12, POCA12, POCA34, POCA34 };
    vector<double> quadrants = {0, M_PI * 0.5, M_PI, M_PI * 1.5};
    vector<double> phiPusher = {0.001, 0.2, 0.5, 1., 1.5};
    vector<double> clockwisers = {1, 1, 1, 1};
    vector<double> rightHanders = {1, 1, -1, -1};
    double radius4All = 6.708203932;
    double manuallyCalculatedPt = 0.03015773701; // only valid for B = 1.5T (which is standard setting)
    double near = 1e-15;      //used for EXPECT_NEAR(val1, val2, abs_error);
    double somewherenear = 1e-5;
    double phiVal = 0, pocaR = 0, curvature = 0;
    TVector3 sigmaVec(somewherenear, somewherenear, somewherenear); // needed since circleFit does not work without positionSigmas


    /**< WARNING Main test function: test the trackletFilters-results */
    auto testStuff = [&](pair<vector<PositionInfo*>, TrackletFilters>& aQuadrantFilterCombi,
    CircleCollection & aCircle) { /// c++11 lambda function...
      aQuadrantFilterCombi.second.resetValues(&aQuadrantFilterCombi.first);
      EXPECT_THROW(aQuadrantFilterCombi.second.calcPt(near), FilterExceptions::Circle_too_small);

      aQuadrantFilterCombi.second.resetValues(&aQuadrantFilterCombi.first);
      EXPECT_FLOAT_EQ(manuallyCalculatedPt, aQuadrantFilterCombi.second.calcPt(radius4All)); // hits of quadrant not used -> simple case

      aQuadrantFilterCombi.second.resetValues(&aQuadrantFilterCombi.first);
      EXPECT_FALSE(aQuadrantFilterCombi.second.ziggZaggXY()); // using hits, but simple calculation. good for checking sanity of hits

      aQuadrantFilterCombi.second.resetValues(&aQuadrantFilterCombi.first);
      EXPECT_FLOAT_EQ(manuallyCalculatedPt, aQuadrantFilterCombi.second.calcPt()); // uses circleFit for calculation

      aQuadrantFilterCombi.second.resetValues(&aQuadrantFilterCombi.first);
      double chi2 = aQuadrantFilterCombi.second.circleFit(phiVal, pocaR, curvature);
      EXPECT_NEAR(0, chi2, somewherenear); // Returns Chi^2

      EXPECT_NEAR(1. / radius4All, aCircle.clockwise * curvature, somewherenear); // estimated curvature

      /// INFO: according to the paper, the pocaR-value carries the information about the system being left or right handed, but here the pocaR value is always positive, but the phiValue provides the hand-information about the system!
      /// intermediate: result carries estimate of poca...
//      EXPECT_NEAR(aCircle.poca.Perp(), /*aCircle.rightHanded**/pocaR, somewherenear); // estimated distance of poca
//      EXPECT_NEAR(aCircle.poca.Perp(), /*aCircle.rightHanded**/pocaR, somewherenear); // estimated distance of poca
//      EXPECT_NEAR(aCircle.poca.Phi() + M_PI*0.5, aCircle.rightHanded*phiVal, somewherenear); // estimated phi of pT vector at pocaR

      aQuadrantFilterCombi.second.resetValues(&aQuadrantFilterCombi.first);
      FitResult aResult = aQuadrantFilterCombi.second.circleFit();
      EXPECT_NEAR(0., aResult.first, somewherenear); // chi2

//      B2WARNING("phiVal, pocaR, curvature: " <<phiVal<<", "<< pocaR<<", "<< curvature)

      EXPECT_DOUBLE_EQ(0., aResult.second.Z());

      /// activate for intermediate: result carries estimate of poca... (WARNING: if you activate this part (and the part in the circleFit, other tests will fail (see down below)))
// //       EXPECT_NEAR((aCircle.poca-aCircle.center).Perp(), (aResult.second-aCircle.center).Perp(), 0.01);

      /// intermediate: result carries estimate of circleCenter... (WARNING: if you activate this part (and the part in the circleFit, other tests will fail (see down below)))
//      EXPECT_NEAR(aCircle.center.Y()/aCircle.center.X(), aResult.second.Y()/aResult.second.X(), 0.01); // checking same slope
//      EXPECT_NEAR(aCircle.center.Perp(), aResult.second.Perp(), 0.01); // checking same magnitude


      EXPECT_NEAR(manuallyCalculatedPt, aResult.second.Perp(), somewherenear); // |pT|



      TVector3 point2SeedHit = aQuadrantFilterCombi.first.front()->hitPosition - aCircle.center;
      TVector3 point2BackSeedHit = aQuadrantFilterCombi.first.back()->hitPosition - aCircle.center;

      /// intermediate: result carries position of seed hit... (WARNING: if you activate this part (and the part in the circleFit, other tests will fail (see down below)))
//      EXPECT_NEAR(point2BackSeedHit.Perp(), aResult.second.Perp(), 0.01); // checking same magnitude
//      EXPECT_NEAR(point2BackSeedHit.Phi(), aResult.second.Phi(), 0.01); // checking same magnitude

// // //      EXPECT_NEAR(point2SeedHit.Phi() + /*aCircle.rightHanded**/M_PI*0.5, aResult.second.Phi(), 0.01);

// //       EXPECT_NEAR(point2SeedHit.Phi() + aCircle.rightHanded*M_PI*0.5, aResult.second.Phi(), 0.01);

// // //      EXPECT_NEAR(point2BackSeedHit.Phi() + /*aCircle.rightHanded**/M_PI*0.5, aResult.second.Phi(), 0.01);

      double calculatedPhi = point2BackSeedHit.Phi() + aCircle.clockwise * M_PI * 0.5;
      if (calculatedPhi > M_PI) calculatedPhi -= 2.*M_PI;
      if (calculatedPhi < -M_PI) calculatedPhi += 2.*M_PI;
      EXPECT_NEAR(calculatedPhi, aResult.second.Phi(), 0.01);

//      EXPECT_NEAR(point2BackSeedHit.Phi() - aCircle.clockwise*M_PI*0.5, aResult.second.Phi(), 0.01);

// // //      EXPECT_NEAR(point2BackSeedHit.Phi() - M_PI + /*aCircle.rightHanded**/M_PI*0.5, aResult.second.Phi(), 0.01);

//      EXPECT_NEAR(point2BackSeedHit.Phi() - M_PI + aCircle.clockwise*M_PI*0.5, aResult.second.Phi(), 0.01);

// // //      EXPECT_EQ(point2SeedHit.Phi() + aCircle.rightHanded*M_PI*0.5, aResult.second.Phi());
    };


    // creating and setting values
    // hits for quadrant 1 - 4:
//    PositionInfo q11, q12, q13, q14, q15, q21, q22, q23, q24, q25, q31, q32, q33, q34, q35, q41, q42, q43, q44, q45;
    // circle segment of a quadrant
//    vector<PositionInfo*> q1, q2, q3, q4, rq1, rq2, rq3, rq4;
    // circle 3 & 4:
//    PositionInfo q51, q52, q53, q54, q55, q61, q62, q63, q64, q65, q71, q72, q73, q34, q75, q81, q82, q83, q84, q85;
//    vector<PositionInfo*> q5, q6, q7, q8, rq5, rq6, rq7, rq8;
//    vector< vector<PositionInfo*>*> allVectors; // for faster filling of sigmaValue
//    allVectors += &q1, &q2, &q3, &q4, &rq1, &rq2, &rq3, &rq4, &q5, &q6, &q7, &q8, &rq5, &rq6, &rq7, &rq8;

    // fill circles with hits
    vector < CircleCollection >  allCircles, allReversedCircles;
    for (uint i = 0 ; i < circleCenters.size(); ++i) { // add circles
      CircleCollection aCircle, aReversedCircle;
      for (uint j = 0 ; j < quadrants.size() ; ++j) { // add quadrants
        vector<PositionInfo*> aQuadrant, reversedQuadrant;
        for (uint k = 0 ; k < phiPusher.size() ; ++k) { // add hits
          PositionInfo* aHit = new PositionInfo();
          aHit->hitPosition.SetMagThetaPhi(radius4All, M_PI * 0.5, quadrants.at(j) + phiPusher.at(k));
          aHit->hitPosition += circleCenters.at(i);
          aHit->hitSigma = sigmaVec;
          aQuadrant.push_back(aHit);
        }
        reversedQuadrant = aQuadrant;
        aCircle.myQuadrantsAndFilters.push_back(make_pair(aQuadrant, TrackletFilters()));
        reverse(reversedQuadrant.begin(), reversedQuadrant.end());
        aReversedCircle.myQuadrantsAndFilters.push_back(make_pair(reversedQuadrant, TrackletFilters()));
      }
      aCircle.center = circleCenters.at(i);
      aReversedCircle.center = circleCenters.at(i);
      aCircle.poca = circlePocas.at(i);
      aReversedCircle.poca = circlePocas.at(i);
      aCircle.clockwise = clockwisers.at(i);
      aReversedCircle.clockwise = -clockwisers.at(i);
      aCircle.rightHanded = rightHanders.at(i);
      aReversedCircle.rightHanded = rightHanders.at(i);

      allCircles.push_back(aCircle);
      allReversedCircles.push_back(aReversedCircle);
    }

    // safety check to be sure that filling went okay
    for (const auto& aCircle : allCircles) {
      for (const auto& aQuadrant : aCircle.myQuadrantsAndFilters) {
        for (const auto& aHit : aQuadrant.first) {
          EXPECT_EQ(sigmaVec, aHit->hitSigma);
        }
      }
    }
    // safety check II:
    for (uint i = 0; i < circleCenters.size(); ++i) {
      for (uint j = 0 ; j < quadrants.size(); ++j) {
        uint l = phiPusher.size() - 1;
        for (uint k = 0; k < phiPusher.size(); ++k) {
          EXPECT_EQ(
            allCircles.at(i).myQuadrantsAndFilters.at(j).first.at(k),
            allReversedCircles.at(i).myQuadrantsAndFilters.at(j).first.at(l)
          );
          --l;
        }
      }
    }

    // execute the tests defined in testStuff:
    uint countCircles = 0, countQuadrants = 0;
    for (auto& aCircle : allCircles) {
      for (auto& aQuadrantFilterCombi : aCircle.myQuadrantsAndFilters) {
        /// activate the following warning to be able to distinguish between the results (only feasible for debugging):
        B2INFO(" circle: " << countCircles << ", quadrant: " << countQuadrants);

        testStuff(aQuadrantFilterCombi, aCircle);

        ++countQuadrants;
      }
      countQuadrants = 0;
      ++countCircles;
    }

    for (auto& aCircle : allReversedCircles) {
      for (auto& aQuadrantFilterCombi : aCircle.myQuadrantsAndFilters) {
        /// activate the following warning to be able to distinguish between the results (only feasible for debugging):
        B2INFO("reversed circle: " << countCircles << ", quadrant: " << countQuadrants);

        testStuff(aQuadrantFilterCombi, aCircle);

        ++countQuadrants;
      }
      countQuadrants = 0;
      ++countCircles;
    }
    // radius in cm, pT in GeV
//    EXPECT_FLOAT_EQ(0.03015773701, filterQ1.calcPt());
//    filterQ1.resetValues(&q1);
//    EXPECT_FLOAT_EQ(0.03015773701, filterQ1.calcPt(radius4All)); // uses circleFit for calculation
//
//    /// checking double circleFit(double&, double&, double&);
//    /// curvature > 0, if clockwise, < 0 if counter clockwise
//    /// pocaR > 0, if rightHanded, < 0 if left handed system between poca-vector and pT-Vector
//    filterQ1.resetValues(&q1);
//    EXPECT_NEAR(0, filterQ1.circleFit(phiVal, pocaR, curvature),somewherenear); // Returns Chi^2
//    EXPECT_NEAR(1./radius4All, curvature, somewherenear); // estimated curvature
//    EXPECT_NEAR(POCA12.Perp(), pocaR, somewherenear); // estimated distance of poca
//    EXPECT_NEAR(POCA12.Phi() + M_PI*0.5, phiVal, somewherenear); // estimated phi of pT vector at pocaR

    // final cleanup
    for (auto& aCircle : allCircles) {
      for (auto& aQuadrant : aCircle.myQuadrantsAndFilters) {
        for (auto& aHit : aQuadrant.first) {
          delete aHit;
        }
        aQuadrant.first.clear();
      }
      aCircle.myQuadrantsAndFilters.clear();
    }
    allCircles.clear();
  }



  /** TestCircleFit */
  TEST_F(TrackletFiltersTest, TestCircleFit)
  {
    TrackletFilters aFilter = TrackletFilters();
    PositionInfo
    hit1, hit2, hit3;

    vector<PositionInfo*> v;

    double ClapPhi, ClapR, rho;
    double near = 1e-10;
    //Circle Center on (2,2) or (1,1) in all quadrants
//     double mx[8]={2,1,-2,-1,-2,-1,2,1};
//     double my[8]={2,1,2,1,-2,-1,-2,-1};
    double x, y;
    int cntr = 0;
    //Values calculated using Geogebra
    double phi[] = {
      333.434948822922 - 360., 315 - 360. , 270, 225,  206.565051177078,
      26.5650511770780, 45, 90, 135, 153.434948822922,
      153.434948822922, 135 , 90 , 45,  26.5650511770780,
      206.565051177078, 225, 270, 315 - 360. , 333.434948822922 - 360.,
    };
    double poca[] = {
      0.236067977499790 , -0.585786437626905 , -1 , -0.585786437626905 , 0.236067977499790,
      0.236067977499790 , -0.585786437626905 , -1 , -0.585786437626905 , 0.236067977499790,
      0.236067977499790 , -0.585786437626905 , -1 , -0.585786437626905 , 0.236067977499790,
      0.236067977499790 , -0.585786437626905 , -1 , -0.585786437626905 , 0.236067977499790
    };

    x = -1.;
    for (int yy = -2; yy < 3; yy++) {
      y = yy;
      hit1.hitPosition = TVector3(x - 2., y, 0.); hit1.hitSigma = TVector3(.1, .1, .1); //hits are clockwise circles of radius 2
      hit2.hitPosition = TVector3(x, y + 2., 1.); hit2.hitSigma = TVector3(.1, .1, .1);
      hit3.hitPosition = TVector3(x + 2., y, 2.); hit3.hitSigma = TVector3(.1, .1, .1);

      v.clear();
      v.push_back(&hit3);
      v.push_back(&hit2);
      v.push_back(&hit1);     //we must fill them backwards because circlefit expects them this way.
      aFilter.resetValues(&v);

      EXPECT_NEAR(0, aFilter.circleFit(ClapPhi, ClapR, rho), near);     // Returns Chi^2
      EXPECT_FLOAT_EQ(ClapPhi, phi[cntr]*M_PI / 180.);
      EXPECT_FLOAT_EQ(ClapR, poca[cntr]);
      EXPECT_FLOAT_EQ(rho, .5);
      cntr++;
    }

    x = 1.;
    for (int yy = -2; yy < 3; yy++) {
      y = yy;
      hit1.hitPosition = TVector3(x - 2., y, 0.); hit1.hitSigma = TVector3(.1, .1, .1); //hits are clockwise circles of radius 2
      hit2.hitPosition = TVector3(x, y + 2., 1.); hit2.hitSigma = TVector3(.1, .1, .1);
      hit3.hitPosition = TVector3(x + 2., y, 2.); hit3.hitSigma = TVector3(.1, .1, .1);

      v.clear();
      v.push_back(&hit3);
      v.push_back(&hit2);
      v.push_back(&hit1);     //we must fill them backwards because circlefit expects them this way.
      aFilter.resetValues(&v);

      EXPECT_NEAR(0, aFilter.circleFit(ClapPhi, ClapR, rho), near);     // Returns Chi^2
      EXPECT_FLOAT_EQ(ClapPhi, phi[cntr]*M_PI / 180.);
      EXPECT_FLOAT_EQ(ClapR, poca[cntr]);
      EXPECT_FLOAT_EQ(rho, .5);
      cntr++;
    }

    //Counterclockwise
    x = -1.;
    for (int yy = -2; yy < 3; yy++) {
      y = yy;
      hit1.hitPosition = TVector3(x - 2., y, 0.); hit1.hitSigma = TVector3(.1, .1, .1); //hits are clockwise circles of radius 2
      hit2.hitPosition = TVector3(x, y + 2., 1.); hit2.hitSigma = TVector3(.1, .1, .1);
      hit3.hitPosition = TVector3(x + 2., y, 2.); hit3.hitSigma = TVector3(.1, .1, .1);

      v.clear();
      v.push_back(&hit1);
      v.push_back(&hit2);
      v.push_back(&hit3);
      aFilter.resetValues(&v);

      EXPECT_NEAR(0, aFilter.circleFit(ClapPhi, ClapR, rho), near);     // Returns Chi^2
      EXPECT_FLOAT_EQ(ClapPhi, phi[cntr]*M_PI / 180.);
      EXPECT_FLOAT_EQ(ClapR, -poca[cntr]);      //For counterclockwise we expect a minus rho and poca
      EXPECT_FLOAT_EQ(rho, -.5);
      cntr++;
    }

    x = 1.;
    for (int yy = -2; yy < 3; yy++) {
      y = yy;
      hit1.hitPosition = TVector3(x - 2., y, 0.); hit1.hitSigma = TVector3(.1, .1, .1); //hits are clockwise circles of radius 2
      hit2.hitPosition = TVector3(x, y + 2., 1.); hit2.hitSigma = TVector3(.1, .1, .1);
      hit3.hitPosition = TVector3(x + 2., y, 2.); hit3.hitSigma = TVector3(.1, .1, .1);

      v.clear();
      v.push_back(&hit1);
      v.push_back(&hit2);
      v.push_back(&hit3);
      aFilter.resetValues(&v);

      EXPECT_NEAR(0, aFilter.circleFit(ClapPhi, ClapR, rho), near);     // Returns Chi^2
      EXPECT_FLOAT_EQ(ClapPhi, phi[cntr]*M_PI / 180.);
      EXPECT_FLOAT_EQ(ClapR, -poca[cntr]);      //For counterclockwise we expect a minus rho and poca
      EXPECT_FLOAT_EQ(rho, -.5);
      cntr++;
    }
  }



  /** TestCalcCurvature */
  TEST_F(TrackletFiltersTest, TestCalcCurvature)
  {
    TrackletFilters aFilter = TrackletFilters();
    PositionInfo
    hit1, hit2, hit3, hit4, hit5;

    vector<PositionInfo*> v;

    hit1.hitPosition = TVector3(1. - 2., 0., 0.); hit1.hitSigma = TVector3(.1, .1, .1); //hits are clockwise circles of radius 2
    hit2.hitPosition = TVector3(1., 2., 1.);  hit2.hitSigma = TVector3(.1, .1, .1);
    hit3.hitPosition = TVector3(1. + 2., 0., 2.); hit3.hitSigma = TVector3(.1, .1, .1);

    v.clear();
    v.push_back(&hit3);
    v.push_back(&hit2);
    v.push_back(&hit1);     //we must fill them backwards because circlefit expects them this way.
    aFilter.resetValues(&v);

    EXPECT_TRUE(aFilter.CalcCurvature());

    v.clear();
    v.push_back(&hit1);
    v.push_back(&hit2);
    v.push_back(&hit3);
    aFilter.resetValues(&v);

    EXPECT_FALSE(aFilter.CalcCurvature());  //now the circle is counterclockwise.

    hit1.hitPosition = TVector3(0., 0., 0.);  hit1.hitSigma = TVector3(.1, .1, .1); //hits are clockwise circles of radius 2
    hit2.hitPosition = TVector3(0., 1., 1.);  hit2.hitSigma = TVector3(.1, .1, .1);
    hit3.hitPosition = TVector3(-1., 2., 2.); hit3.hitSigma = TVector3(.1, .1, .1);
    hit4.hitPosition = TVector3(-1., 3., 1.); hit4.hitSigma = TVector3(.1, .1, .1);
    hit5.hitPosition = TVector3(-2., 4., 2.); hit5.hitSigma = TVector3(.1, .1, .1);

    v.clear();
    v.push_back(&hit5);
    v.push_back(&hit4);
    v.push_back(&hit3);
    v.push_back(&hit2);
    v.push_back(&hit1);     //we must fill them backwards because circlefit expects them this way.

    aFilter.resetValues(&v);

    EXPECT_FALSE(aFilter.CalcCurvature());
  }
}  // namespace
