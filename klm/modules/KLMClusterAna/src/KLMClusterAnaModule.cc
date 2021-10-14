/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//#pragma once
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <numeric>
#include <TVectorT.h>
#include <TMatrixT.h>
#include <TVector3.h>
#include <TMatrixDSymEigen.h>
#include <TLorentzVector.h>

#include <klm/modules/KLMClusterAna/KLMClusterAnaModule.h>

/* KLM headers. */
#include <mdst/dataobjects/KLMCluster.h>
#include <klm/dataobjects/bklm/BKLMHit2d.h>
#include <klm/dataobjects/eklm/EKLMHit2d.h>

/* Other Belle 2 headers. */
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>



using namespace Belle2;
using namespace std;

//foward declarations here

float expectation(vector<float> vec);
vector<float> addition(vector<float> vec1, vector<float> vec2);
vector<float> product(vector<float> vec1, vector<float> vec2);
vector<double> covariance_matrix3x3(vector<float> xcoord, vector<float> ycoord, vector<float> zcoord);
TMatrixT<float> eigenvectors3x3(vector<double> matrix);
TMatrixT<float> spatialVariances(vector<float> xcoord, vector<float> ycoord, vector<float> zcoord);


//Code for Module

REG_MODULE(KLMClusterAna)

KLMClusterAnaModule::KLMClusterAnaModule() : Module()
{
  setDescription("Module for extracting KLM cluster shape information via PCA.");
}

KLMClusterAnaModule::~KLMClusterAnaModule()
{
}


void KLMClusterAnaModule::initialize()
{


  //We want this variable ready to register
  m_KLMClusterShape.registerInDataStore("KLMClusterShape");

  //Do I want to have both BKLM and EKLM hits in one module?
  m_KLMClusters.isRequired();
  bklmHit2ds.isOptional();
  eklmHit2ds.isOptional();


  m_KLMClusters.registerRelationTo(m_KLMClusterShape);
  m_KLMClusterShape.registerRelationTo(bklmHit2ds);
  m_KLMClusterShape.registerRelationTo(eklmHit2ds);
  bklmHit2ds.registerRelationTo(m_KLMClusterShape);
  eklmHit2ds.registerRelationTo(m_KLMClusterShape);

  if (bklmHit2ds.isValid() == true || eklmHit2ds.isValid() == true) {}
  else {
    B2WARNING("The KLMClusterAna module requires either BKLMHit2ds or EKLMHit2ds. ");
  }



}

void KLMClusterAnaModule::beginRun()
{
}

void KLMClusterAnaModule::event()
{
  m_KLMClusterShape.clear();

  for (KLMCluster& klmcluster : m_KLMClusters) {
    runClusterAna(klmcluster);

  }//klmcluster loop


}


void KLMClusterAnaModule::runClusterAna(KLMCluster& klmcluster)
{
  //m_KLMClusterShape.clear();
  //Obtain BKLMHit2D information
  TVector3 hitPosition;

  RelationVector<BKLMHit2d> bHit2ds = klmcluster.getRelationsTo<BKLMHit2d>();
  int nBKLMHits = bHit2ds.size(); //add a constraint

  //Obtain EKLMHit2D information
  RelationVector<EKLMHit2d> eHit2ds = klmcluster.getRelationsTo<EKLMHit2d>();
  int nEKLMHits = eHit2ds.size(); //add a constraint

  //int nHits = nBKLMHits;// + nEKLMHits;
  int nHits = nBKLMHits + nEKLMHits;

  vector<float> xHits(nHits);
  vector<float> yHits(nHits);
  vector<float> zHits(nHits);

  for (int i = 0; i < nBKLMHits; i++) {
    hitPosition = bHit2ds[i]->getGlobalPosition();
    xHits[i] = hitPosition.X();
    yHits[i] = hitPosition.Y();
    zHits[i] = hitPosition.Z();
  }
  hitPosition.Clear();
  //After cycling through bklmHits, inspect eklmHits
  for (int j = nBKLMHits; j < nHits; j++) {
    hitPosition = eHit2ds[j - nBKLMHits]->getPosition();
    xHits[j] = (float) hitPosition.X();
    yHits[j] = (float) hitPosition.Y();
    zHits[j] = (float) hitPosition.Z();
  }


  KLMClusterShape* clusterShape = m_KLMClusterShape.appendNew();
  clusterShape->setNHits(nHits);
  if (nHits > 1) {

    //Use BKLMHit2D information to obtain relevant cluster information
    TMatrixT<float> output = spatialVariances(xHits, yHits, zHits);
    clusterShape->setEigen(output);

  } else {
    //pass: just initialize and keep empty/default values
  }


  klmcluster.addRelationTo(clusterShape);

  for (int i = 0; i < nBKLMHits; i++) {
    bHit2ds[i]->addRelationTo(clusterShape);
    clusterShape->addRelationTo(bHit2ds[i]);
  }
  for (int i = 0; i < nEKLMHits; i++) {
    eHit2ds[i]->addRelationTo(clusterShape);
    clusterShape->addRelationTo(eHit2ds[i]);
  }


}




float expectation(vector<float> vec)
{
  //Note that this assumes uniform probability
  //accumulate from <numeric>
  return accumulate(vec.begin(), vec.end(), 0.0) / vec.size();
}

vector<float> addition(vector<float> vec1, vector<float> vec2)
{

  if (vec1.size() != vec2.size()) {
    //replace with B2FATAL
    B2ERROR("Vector lengths don't match so error. (addition)");
  }

  vector<float> output(vec1.size());
  for (int i = 0; i < (int) vec1.size(); ++i) {
    output[i] = vec1[i] + vec2[i];
  }
  return output;
}



vector<float>  product(vector<float> vec1, vector<float> vec2)
{

  if (vec1.size() != vec2.size())  {
    //replace with B2FATAL
    B2ERROR("Vector lengths don't match so error. (product)");
  }

  vector<float> output(vec1.size());
  for (int i = 0; i < (int) vec1.size(); ++i) {
    output[i] = vec1[i] * vec2[i];
  }
  return output;
}


vector<double> covariance_matrix3x3(vector<float> xcoord, vector<float> ycoord, vector<float> zcoord)
{

  if (xcoord.size() != ycoord.size() || (ycoord.size() != zcoord.size()))  {
    //replace with B2FATAL
    B2ERROR("Vector lengths don't match so error. (Covariance Matrix)");
  }

  int length = xcoord.size();
  float xmean = expectation(xcoord); float ymean = expectation(ycoord); float zmean = expectation(zcoord);
  //minus sign here is purposeful
  vector<float> xmeanV(length, -1 * xmean);
  vector<float> ymeanV(length, -1 * ymean);
  vector<float> zmeanV(length, -1 * zmean);

  vector<float> deltax = addition(xcoord, xmeanV);
  vector<float> deltay = addition(ycoord, ymeanV);
  vector<float> deltaz = addition(zcoord, zmeanV);

  double xxterm = expectation(product(deltax, deltax));
  double xyterm = expectation(product(deltax, deltay));
  double xzterm = expectation(product(deltax, deltaz));
  double yyterm = expectation(product(deltay, deltay));
  double yzterm = expectation(product(deltay, deltaz));
  double zzterm = expectation(product(deltaz, deltaz));

  double array[] = {xxterm, xyterm, xzterm, xyterm, yyterm, yzterm, xzterm, yzterm, zzterm};
  vector <double>output(begin(array), end(array));

  return output;


}


TMatrixT<float> eigenvectors3x3(vector<double> matrix)
{
  if (matrix.size() != 9) {
    B2ERROR("Error! For eigenvalue3x3 calc, invalid matrix size");
  }
  //If I don't need a/b/c, then use a different function.

  TMatrixDSym covar(3);
  for (int i = 0; i < 9; i++) {
    covar[i % 3][i / 3] = matrix[i];
  }
  const TMatrixDSymEigen eigen(covar);
  const TVectorT<double> eigenList = eigen.GetEigenValues();
  const TMatrixT<double> eigenvecs = eigen.GetEigenVectors();


  //const TVectorT<float> priVec; const TVectorT<float> secVec; const TVectorT<float> terVec;
  //looking at TMatrixDEigen source file, sorting based on |eigenVal|^2 is built in.
  //TMatrixDSymEigen:https://root.cern.ch/doc/master/TMatrixDSymEigen_8cxx_source.html

  //[rows][columns]
  TMatrixT<float> output(4, 3); // convert from double to float
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      output[i][j] = (float) eigenvecs[i][j];
    }
    output[3][i] = (float) eigenList[i];
  }

  return output;

}




TMatrixT<float> spatialVariances(vector<float> xcoord, vector<float> ycoord, vector<float> zcoord)
{
  /**
   Takes lists of x/y/z coordinates (as vectors) and converts that to provide a list of eigenvectors
   Columns of TMatrix provide eigenvectors were |e_i| = eigenvalue
   */

  if (xcoord.size() != ycoord.size() || (ycoord.size() != zcoord.size())) {
    //replace with B2FATAL
    B2ERROR("Vector lengths don't match so error.");
  }
  vector<double> covar = covariance_matrix3x3(xcoord, ycoord, zcoord);


  TMatrixT<float> output = eigenvectors3x3(covar);
  return output;

}