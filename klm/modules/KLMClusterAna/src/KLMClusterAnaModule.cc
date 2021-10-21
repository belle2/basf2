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
#include <TVector3.h>
#include <TMatrixT.h>
#include <TMatrixDSymEigen.h>
#include <TMath.h>

#include <klm/modules/KLMClusterAna/KLMClusterAnaModule.h>

/* KLM headers. */
#include <mdst/dataobjects/KLMCluster.h>
#include <klm/dataobjects/bklm/BKLMHit2d.h>
#include <klm/dataobjects/eklm/EKLMHit2d.h>
#include <klm/modules/KLMClustersReconstructor/KLMHit2d.h>

/* Other Belle 2 headers. */
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>



using namespace Belle2;

//foward declarations here

double expectation(std::vector<double> vec);
std::vector<double> addition(std::vector<double> vec1, std::vector<double> vec2);
std::vector<double> product(std::vector<double> vec1, std::vector<double> vec2);
std::vector<double> covariance_matrix3x3(std::vector<double> xcoord, std::vector<double> ycoord, std::vector<double> zcoord);
TMatrixT<double> eigenvectors3x3(std::vector<double> matrix);
TMatrixT<double> spatialVariances(std::vector<double> xcoord, std::vector<double> ycoord, std::vector<double> zcoord);


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
  m_KLMClusterShape.registerInDataStore();

  //Do I want to have both BKLM and EKLM hits in one module?
  m_KLMClusters.isRequired();
  m_bklmHit2ds.isOptional();
  m_eklmHit2ds.isOptional();


  m_KLMClusters.registerRelationTo(m_KLMClusterShape);
  m_KLMClusterShape.registerRelationTo(m_bklmHit2ds);
  m_KLMClusterShape.registerRelationTo(m_eklmHit2ds);

  if (m_bklmHit2ds.isValid() == true || m_eklmHit2ds.isValid() == true) {}
  else {
    B2ERROR("The KLMClusterAna module requires either BKLMHit2ds or EKLMHit2ds. ");
  }



}

void KLMClusterAnaModule::beginRun()
{
}

void KLMClusterAnaModule::event()
{
  for (KLMCluster& klmcluster : m_KLMClusters) {
    //Obtain BKLMHit2D information
    TVector3 hitPosition;

    //Obtain BKLMHit2D Information
    RelationVector<BKLMHit2d> bHit2ds = klmcluster.getRelationsTo<BKLMHit2d>();
    int nBKLMHits = bHit2ds.size();

    //Obtain EKLMHit2D information
    RelationVector<EKLMHit2d> eHit2ds = klmcluster.getRelationsTo<EKLMHit2d>();
    int nEKLMHits = eHit2ds.size();

    int nHits = nBKLMHits + nEKLMHits;

    std::vector<double> xHits(nHits);
    std::vector<double> yHits(nHits);
    std::vector<double> zHits(nHits);


    for (int i = 0; i < nBKLMHits; i++) {
      hitPosition = bHit2ds[i]->getGlobalPosition();
      xHits[i] = (double) hitPosition.X();
      yHits[i] = (double) hitPosition.Y();
      zHits[i] = (double) hitPosition.Z();
    }
    hitPosition.Clear();
    //After cycling through bklmHits, inspect eklmHits
    for (int j = nBKLMHits; j < nHits; j++) {
      hitPosition = eHit2ds[j - nBKLMHits]->getPosition();
      xHits[j] = (double) hitPosition.X();
      yHits[j] = (double) hitPosition.Y();
      zHits[j] = (double) hitPosition.Z();
    }


    KLMClusterShape* clusterShape = m_KLMClusterShape.appendNew();
    clusterShape->setNHits(nHits);
    if (nHits > 1) {

      //Use BKLMHit2D information to obtain relevant cluster information
      TMatrixT<double> output = spatialVariances(xHits, yHits, zHits);
      clusterShape->setEigen(output);

    } else {
      //pass: just initialize and keep empty/default values
    }


    klmcluster.addRelationTo(clusterShape);

    for (int i = 0; i < nBKLMHits; i++) {
      clusterShape->addRelationTo(bHit2ds[i]);
    }
    for (int i = 0; i < nEKLMHits; i++) {
      clusterShape->addRelationTo(eHit2ds[i]);
    }



  }//klmcluster loop

}



double expectation(std::vector<double> vec)
{
  //Note that this assumes uniform probability
  //accumulate from <numeric>
  return accumulate(vec.begin(), vec.end(), 0.0) / vec.size();
}

std::vector<double> addition(std::vector<double> vec1, std::vector<double> vec2)
{

  if (vec1.size() != vec2.size()) {
    //replace with B2FATAL
    B2ERROR("Vector lengths don't match so error. (addition)");
  }

  std::vector<double> output(vec1.size());
  for (int i = 0; i < (int) vec1.size(); ++i) {
    output[i] = vec1[i] + vec2[i];
  }
  return output;
}



std::vector<double> product(std::vector<double> vec1, std::vector<double> vec2)
{

  if (vec1.size() != vec2.size())  {
    //replace with B2FATAL
    B2ERROR("Vector lengths don't match so error. (product)");
  }

  std::vector<double> output(vec1.size());
  for (int i = 0; i < (int) vec1.size(); ++i) {
    output[i] = vec1[i] * vec2[i];
  }
  return output;
}


std::vector<double> covariance_matrix3x3(std::vector<double> xcoord, std::vector<double> ycoord, std::vector<double> zcoord)
{

  if (xcoord.size() != ycoord.size() || (ycoord.size() != zcoord.size()))  {
    //replace with B2FATAL
    B2ERROR("Vector lengths don't match so error. (Covariance Matrix)");
  }

  int length = xcoord.size();
  double xmean = expectation(xcoord); double ymean = expectation(ycoord); double zmean = expectation(zcoord);
  //minus sign here is purposeful
  std::vector<double> xmeanV(length, -1 * xmean);
  std::vector<double> ymeanV(length, -1 * ymean);
  std::vector<double> zmeanV(length, -1 * zmean);

  std::vector<double> deltax = addition(xcoord, xmeanV);
  std::vector<double> deltay = addition(ycoord, ymeanV);
  std::vector<double> deltaz = addition(zcoord, zmeanV);

  double xxterm = expectation(product(deltax, deltax));
  double xyterm = expectation(product(deltax, deltay));
  double xzterm = expectation(product(deltax, deltaz));
  double yyterm = expectation(product(deltay, deltay));
  double yzterm = expectation(product(deltay, deltaz));
  double zzterm = expectation(product(deltaz, deltaz));

  double array[] = {xxterm, xyterm, xzterm, xyterm, yyterm, yzterm, xzterm, yzterm, zzterm};
  std::vector <double>output(std::begin(array), std::end(array));

  return output;


}


TMatrixT<double> eigenvectors3x3(std::vector<double> matrix)
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

  //looking at TMatrixDEigen source file, sorting based on |eigenVal|^2 is built in.
  //TMatrixDSymEigen:https://root.cern.ch/doc/master/TMatrixDSymEigen_8cxx_source.html

  //[rows][columns]
  TMatrixT<double> output(4, 3);
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      output[i][j] = eigenvecs[i][j];
    }
    output[3][i] = eigenList[i];
  }

  return output;

}




TMatrixT<double> spatialVariances(std::vector<double> xcoord, std::vector<double> ycoord, std::vector<double> zcoord)
{
  /**
   Takes lists of x/y/z coordinates (as vectors) and converts that to provide a list of eigenvectors
   Columns of TMatrix provide eigenvectors were |e_i| = eigenvalue
   */

  if (xcoord.size() != ycoord.size() || (ycoord.size() != zcoord.size())) {
    //replace with B2FATAL
    B2ERROR("Vector lengths don't match so error.");
  }
  std::vector<double> covar = covariance_matrix3x3(xcoord, ycoord, zcoord);


  TMatrixT<double> output = eigenvectors3x3(covar);
  return output;

}