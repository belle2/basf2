/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/modules/KLMClusterAna/KLMClusterAnaModule.h>

/* Other Belle 2 headers. */
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

/* ROOT headers. */
#include <TMatrixT.h>
#include <TMatrixDSymEigen.h>
#include <TVectorT.h>
#include <TVector3.h>

/* C++ headers. */
#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <vector>


using namespace Belle2;

REG_MODULE(KLMClusterAna);


static double expectation(std::vector<double> vec)
{
  //Note that this assumes uniform probability
  //accumulate from <numeric>
  return accumulate(vec.begin(), vec.end(), 0.0) / vec.size();
}

static std::vector<double> addition(std::vector<double> vec1, std::vector<double> vec2)
{
  std::vector<double> output(vec1.size());
  if (vec1.size() != vec2.size()) {
    B2ERROR("Vector lengths don't match so error. (addition)");
    for (unsigned int i = 0; i < (unsigned int) vec1.size(); ++i) {
      output[i] = 0.0;
    }
    return output;
  }

  for (unsigned int i = 0; i < (unsigned int) vec1.size(); ++i) {
    output[i] = vec1[i] + vec2[i];
  }
  return output;
}



static std::vector<double> product(std::vector<double> vec1, std::vector<double> vec2)
{
  std::vector<double> output(vec1.size());
  if (vec1.size() != vec2.size())  {
    B2ERROR("Vector lengths don't match so error. (product)");
    for (unsigned int i = 0; i < (unsigned int) vec1.size(); ++i) {
      output[i] = 0.0;
    }
    return output;
  }

  for (unsigned int i = 0; i < (unsigned int) vec1.size(); ++i) {
    output[i] = vec1[i] * vec2[i];
  }
  return output;
}


static std::vector<double> covariance_matrix3x3(std::vector<double> xcoord, std::vector<double> ycoord, std::vector<double> zcoord)
{

  if (xcoord.size() != ycoord.size() || (ycoord.size() != zcoord.size()))  {
    B2ERROR("Vector lengths don't match so error. (Covariance Matrix)");
    double array[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    std::vector <double>output(std::begin(array), std::end(array));
    return output;
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


static TMatrixT<double> eigenvectors3x3(std::vector<double> matrix)
{
  //[rows][columns]
  TMatrixT<double> output(4, 3);
  if (matrix.size() != 9) {
    B2ERROR("Error! For eigenvalue3x3 calc, invalid matrix size");
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        output[i][j] = 0.0;
      }
      output[3][i] = 0.0;
    }
    return output;
  }

  TMatrixDSym covar(3);
  for (int i = 0; i < 9; i++) {
    covar[i % 3][i / 3] = matrix[i];
  }
  const TMatrixDSymEigen eigen(covar);
  const TVectorT<double> eigenList = eigen.GetEigenValues();
  const TMatrixT<double> eigenvecs = eigen.GetEigenVectors();

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      output[i][j] = eigenvecs[i][j];
    }
    output[3][i] = eigenList[i];
  }
  return output;
}




static TMatrixT<double> spatialVariances(std::vector<double> xcoord, std::vector<double> ycoord, std::vector<double> zcoord)
{
  /**
   Takes lists of x/y/z coordinates (as vectors) and converts that to provide a list of eigenvectors
   Columns of TMatrix provide eigenvectors were |e_i| = eigenvalue
   */

  if (xcoord.size() != ycoord.size() || (ycoord.size() != zcoord.size())) {
    B2FATAL("Vector lengths don't match so error.");
  }
  std::vector<double> covar = covariance_matrix3x3(xcoord, ycoord, zcoord);

  TMatrixT<double> output = eigenvectors3x3(covar);
  return output;
}


//Code for Module

KLMClusterAnaModule::KLMClusterAnaModule() : Module()
{
  setDescription("Module for extracting KLM cluster shape information via PCA.");
}

KLMClusterAnaModule::~KLMClusterAnaModule()
{
}


void KLMClusterAnaModule::initialize()
{
  m_KLMClusterShape.registerInDataStore();
  m_KLMClusters.isRequired();
  m_klmHit2ds.isRequired();
  m_KLMClusters.registerRelationTo(m_KLMClusterShape);
  m_KLMClusterShape.registerRelationTo(m_klmHit2ds);
}

void KLMClusterAnaModule::beginRun()
{
}

void KLMClusterAnaModule::event()
{
  for (KLMCluster& klmcluster : m_KLMClusters) {
    //Obtain KLMHit2D information
    ROOT::Math::XYZVector hitPosition;

    //Obtain KLMHit2D Information
    RelationVector<KLMHit2d> hit2ds = klmcluster.getRelationsTo<KLMHit2d>();
    int nHits = hit2ds.size();

    std::vector<double> xHits(nHits);
    std::vector<double> yHits(nHits);
    std::vector<double> zHits(nHits);

    std::vector<KLMHit2d*> klmHit2ds;
    std::vector<KLMHit2d*>::iterator it;

    for (int i = 0; i < nHits; i++) {
      klmHit2ds.push_back(hit2ds[i]);
      hitPosition = hit2ds[i]->getPosition();
      xHits[i] = (double) hitPosition.X();
      yHits[i] = (double) hitPosition.Y();
      zHits[i] = (double) hitPosition.Z();
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

    for (it = klmHit2ds.begin(); it != klmHit2ds.end(); ++it) {
      clusterShape->addRelationTo(*it);
    }

  }

}


