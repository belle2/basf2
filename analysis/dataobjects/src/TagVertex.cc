/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014-2019 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Li Gioi, Stefano Lacaprara                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/dataobjects/TagVertex.h>

#include <framework/datastore/StoreArray.h>

#include <framework/logging/Logger.h>
#include <iostream>

using namespace Belle2;


TVector3 TagVertex::getTagVertex()
{
  return m_tagVertex;
}

TMatrixFSym TagVertex::getTagVertexErrMatrix()
{
  return m_tagVertexErrMatrix;
}

float TagVertex::getTagVertexPval()
{
  return m_tagVertexPval;
}

float TagVertex::getDeltaT()
{
  return m_deltaT;
}

float TagVertex::getDeltaTErr()
{
  return m_deltaTErr;
}

TVector3 TagVertex::getMCTagVertex()
{
  return m_MCtagV;
}

int TagVertex::getMCTagBFlavor()
{
  return m_mcPDG;
}

float TagVertex::getMCDeltaT()
{
  return m_MCdeltaT;
}

int TagVertex::getFitType()
{
  return m_FitType;
}

int TagVertex::getNTracks()
{
  return m_NTracks;
}

float TagVertex::getTagVl()
{
  return m_tagVl;
}

float TagVertex::getTruthTagVl()
{
  return m_truthTagVl;
}

float TagVertex::getTagVlErr()
{
  return m_tagVlErr;
}

float TagVertex::getTagVol()
{
  return m_tagVol;
}

float TagVertex::getTruthTagVol()
{
  return m_truthTagVol;
}

float TagVertex::getTagVolErr()
{
  return m_tagVolErr;
}



void TagVertex::setTagVertex(const TVector3& tagVertex)
{
  m_tagVertex = tagVertex;
}

void TagVertex::setTagVertexErrMatrix(const TMatrixFSym& TagVertexErrMatrix)
{
  m_tagVertexErrMatrix = TagVertexErrMatrix;
}

void TagVertex::setTagVertexPval(float TagVertexPval)
{
  m_tagVertexPval = TagVertexPval;
}

void TagVertex::setDeltaT(float DeltaT)
{
  m_deltaT = DeltaT;
}

void TagVertex::setDeltaTErr(float DeltaTErr)
{
  m_deltaTErr = DeltaTErr;
}

void TagVertex::setMCTagVertex(const TVector3& MCTagVertex)
{
  m_MCtagV = MCTagVertex;
}

void TagVertex::setMCTagBFlavor(int MCTagBFlavor)
{
  m_mcPDG = MCTagBFlavor;
}

void TagVertex::setMCDeltaT(float MCDeltaT)
{
  m_MCdeltaT = MCDeltaT;
}

void TagVertex::setFitType(float FitType)
{
  m_FitType = FitType;
}

void TagVertex::setNTracks(int NTracks)
{
  m_NTracks = NTracks;
}

void TagVertex::setTagVl(float TagVl)
{
  m_tagVl = TagVl;
}

void TagVertex::setTruthTagVl(float TruthTagVl)
{
  m_truthTagVl = TruthTagVl;
}

void TagVertex::setTagVlErr(float TagVlErr)
{
  m_tagVlErr = TagVlErr;
}

void TagVertex::setTagVol(float TagVol)
{
  m_tagVol = TagVol;
}

void TagVertex::setTruthTagVol(float TruthTagVol)
{
  m_truthTagVol = TruthTagVol;
}

void TagVertex::setTagVolErr(float TagVolErr)
{
  m_tagVolErr = TagVolErr;
}



void  TagVertex::resetTagVertexErrorMatrix()
{
  TMatrixFSym temp(3);
  m_tagVertexErrMatrix.ResizeTo(temp);
  m_tagVertexErrMatrix = temp;
}
