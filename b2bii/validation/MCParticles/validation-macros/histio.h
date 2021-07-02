/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

void histio() ;
void saveHist(const char* filename = "out.root", const char* pat = "*") ;
void loadHist(const char* filename = "in.root", const char* pfx = 0,
              const char* pat = "*", Bool_t doAdd = kFALSE, Double_t scaleFactor = -1.0) ;
