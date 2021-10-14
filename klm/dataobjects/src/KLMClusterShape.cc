/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


/* Own header. */
#include <klm/dataobjects/KLMClusterShape.h> //change path eventually 



using namespace Belle2;

//! Empty constructor for ROOT IO

KLMClusterShape::KLMClusterShape() :
  RelationsObject(),
  m_nHits(0)
{
  for (int i = 0; i < 4; i++) {
    m_primaryVec[i] = 0.;
    m_secondaryVec[i] = 0.;
    m_tertiaryVec[i] = 0.;
  }
}



//! Copy constructor
KLMClusterShape::KLMClusterShape(const KLMClusterShape& clusterShape) :
  RelationsObject(clusterShape),
  m_nHits(clusterShape.m_nHits)
{
  for (int i = 0; i < 4; i++) {
    m_primaryVec[i] = clusterShape.m_primaryVec[i];
    m_secondaryVec[i] = clusterShape.m_secondaryVec[i];
    m_tertiaryVec[i] = clusterShape.m_tertiaryVec[i];
  }
}



//! Assignment operator
KLMClusterShape& KLMClusterShape::operator=(const KLMClusterShape& clusterShape)
{
  m_nHits = clusterShape.m_nHits;

  for (int i = 0; i < 4; i++) {
    m_primaryVec[i] = clusterShape.m_primaryVec[i];
    m_secondaryVec[i] = clusterShape.m_secondaryVec[i];
    m_tertiaryVec[i] = clusterShape.m_tertiaryVec[i];
  }
  return *this;
}

//GETTERS (for vectors)
TVector3 KLMClusterShape::getPrimaryVector()
{
  //TVector3 output = m_primaryVec.Vect();
  TVector3 output(m_primaryVec[0], m_primaryVec[1], m_primaryVec[2]);
  return output;

}

TVector3 KLMClusterShape::getSecondaryVector()
{
  //TVector3 output = m_secondaryVec.Vect();
  TVector3 output(m_secondaryVec[0], m_secondaryVec[1], m_secondaryVec[2]);
  return output;

}

TVector3 KLMClusterShape::getTertiaryVector()
{
  TVector3 output(m_tertiaryVec[0], m_tertiaryVec[1], m_tertiaryVec[2]);
  //TVector3 output = m_tertiaryVec.Vect();
  return output;

}

//setters
void KLMClusterShape::setEigen(TMatrixT<float> eigenList)
{
  for (int i = 0; i < 4; i++) {
    m_primaryVec[i] = eigenList[i][0];
    m_secondaryVec[i] = eigenList[i][1];
    m_tertiaryVec[i] = eigenList[i][2];
  }

}


