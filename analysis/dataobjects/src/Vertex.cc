/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Li Gioi                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/dataobjects/Vertex.h>

#include <framework/datastore/StoreArray.h>

#include <framework/logging/Logger.h>
#include <iostream>

using namespace Belle2;


TVector3 Vertex::getTagVertex()
{
  return m_tagVertex;
}

TMatrixFSym Vertex::getTagVertexErrMatrix()
{
  return m_tagVertexErrMatrix;
}

float Vertex::getTagVertexPval()
{
  return m_tagVertexPval;
}

float Vertex::getDeltaT()
{
  return m_deltaT;
}

TVector3 Vertex::getMCTagVertex()
{
  return m_MCtagV;
}

int Vertex::getMCTagBFlavor()
{
  return m_mcPDG;
}

float Vertex::getMCDeltaT()
{
  return m_MCdeltaT;
}


void Vertex::setTagVertex(TVector3 TagVertex)
{
  m_tagVertex = TagVertex;
}

void Vertex::setTagVertexErrMatrix(TMatrixFSym TagVertexErrMatrix)
{
  m_tagVertexErrMatrix = TagVertexErrMatrix;
}

void Vertex::setTagVertexPval(float TagVertexPval)
{
  m_tagVertexPval = TagVertexPval;
}

void Vertex::setDeltaT(float DeltaT)
{
  m_deltaT = DeltaT;
}

void Vertex::setMCTagVertex(TVector3 MCTagVertex)
{
  m_MCtagV = MCTagVertex;
}

void Vertex::setMCTagBFlavor(int MCTagBFlavor)
{
  m_mcPDG = MCTagBFlavor;
}

void Vertex::setMCDeltaT(float MCDeltaT)
{
  m_MCdeltaT = MCDeltaT;
}


void  Vertex::resetTagVertexErrorMatrix()
{
  TMatrixFSym temp(3);
  m_tagVertexErrMatrix.ResizeTo(temp);
  m_tagVertexErrMatrix = temp;
}
