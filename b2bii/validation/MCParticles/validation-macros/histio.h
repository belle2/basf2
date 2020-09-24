#pragma once

void histio() ;
void saveHist(const char* filename = "out.root", const char* pat = "*") ;
void loadHist(const char* filename = "in.root", const char* pfx = 0,
              const char* pat = "*", Bool_t doAdd = kFALSE, Double_t scaleFactor = -1.0) ;
