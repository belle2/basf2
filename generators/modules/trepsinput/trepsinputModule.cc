/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kiyoshi Hayasaka                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/trepsinput/trepsinputModule.h>
#include <generators/treps/Sutool.h>
#include <generators/treps/UtrepsB.h>
#include <string.h>
#include <string>
#include <boost/filesystem.hpp>
#include <TFile.h>
/* --------------- WARNING ---------------------------------------------- *
   If you have more complex parameter types in your class then simple int,
   double or std::vector of those you might need to uncomment the following
   include directive to avoid an undefined reference on compilation.
   * ---------------------------------------------------------------------- */
// #include <framework/core/ModuleParam.templateDetails.h>



using namespace Belle2;



//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(trepsinput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

trepsinputModule::trepsinputModule() : Module(), UtrepsB()
{
  // Set module properties
  setDescription("Input from TREPS generator (No-tag), Input from TREPS generator for ee->ee hadrons");

  // setDescription(R"DOC("Input from TREPS generator (No-tag)

  //     Input from TREPS generator for ee->ee hadrons
  //     )DOC");

  // Parameter definitions
  //   dscr->define_param ( "W", "W, gamma-gamma cm energy", &module->wf );
  addParam("W", wf, "W, gamma-gamma cm energy", (float)2.0);
  std::string dfname("treps_par.dat");
  std::string fname;
  addParam("InputFileName", fname, "filename for TREPS input", dfname);
  strncpy(filename, fname.c_str(), 130);

  std::string dfname2("wlist_table.dat");
  std::string fname2;
  addParam("InputFileName2", fname2, "filename for W-List input", dfname2);
  strncpy(filename2, fname2.c_str(), 130);

  std::string nocheck("");
  addParam("RootFileNameForCheck", rfnfc, "filename for TREPS W-Listbehavior check", nocheck);

  //

  TrepsB::initp();
  TrepsB::create_hist();
  // set default value for w
  // wf = 2.0;

  //Initialize generator;
  UtrepsB::initg();

  //201903
  TrepsB::wtcount = 0;
  double dum = TrepsB::wtable(0);
  //201903E

}

void trepsinputModule::initialize()
{

  TrepsB::w = (double)TrepsB::wf;
  trepsinputModule::updateW();
  m_mcparticles.registerInDataStore();
}

void trepsinputModule::event()
{

  if (TrepsB::inmode != 0) return;

  // check the change of W value
  //201903
  // get W from table
  TrepsB::wtcount++;
  TrepsB::wf = (float)(wtable(1));
  //201903E

  // check the change of W value

  //201903
  //DEBUG if( abs((double)wf - TrepsB::w ) >=0.001 ){
  if (abs((double)TrepsB::wf - TrepsB::w) >= 0.001 && TrepsB::wf > 0.01) {
    //201903E
    B2INFO(" W value changed. " << TrepsB::w << " to " << TrepsB::wf);
    //initialize();
    TrepsB::w = (double)TrepsB::wf;
    trepsinputModule::updateW();

  }
  //   char ctemp;
  int idummy = 0;
  // cin >> ctemp;
  int iret = TrepsB::event_gen(idummy);
  mpg.clear();

  if (iret >= 1) {

    const Part_gen* part = TrepsB::partgen;

    //  cout <<TrepsB::npart<<TrepsB::pp<<endl;

    // fill data of the final-state particles
    for (int i = 0; i < npart ; i++) {
      auto& p = mpg.addParticle();
      //  hepevt.isthep( 1 );
      //  hepevt.idhep( part[i].part_prop.icode);
      p.setPDG(part[i].part_prop.icode);
      //  hepevt.mother( 0 );
      //  hepevt.mo(0,0);
      //  hepevt.daFirst( 0 );
      //  hepevt.daLast( 0 );
      //  hepevt.PX( (float)(part[i].p.x()) );
      //  hepevt.PY( (float)(part[i].p.y()) );
      //  hepevt.PZ( (float)(part[i].p.z()) );
      //  hepevt.E( (float)(part[i].p.t()) );
      p.set4Vector(part[i].p);
      //  hepevt.M( (float)(part[i].part_prop.pmass) );
      p.setMass(part[i].part_prop.pmass);
      //  hepevt.VX( 0.0 );
      //  hepevt.VY( 0.0 );
      //  hepevt.VZ( 0.0 );
      //  hepevt.T( 0.0 );
      p.setStatus(MCParticle::c_PrimaryParticle | MCParticle::c_StableInGenerator);
    }
    // fill data of the recoil electron and positron
    auto& p1 = mpg.addParticle();
    //  hepevt1.isthep( 1 );
    //  hepevt1.idhep( 11 );
    p1.setPDG(11);
    //  hepevt1.mother( 0 );
    //  hepevt1.mo( 0, 0 );
    //  hepevt1.daFirst( 0 );
    //  hepevt1.daLast( 0 );
    //  hepevt1.PX( (float)(TrepsB::pe.x()) );
    //  hepevt1.PY( (float)(TrepsB::pe.y()) );
    //  hepevt1.PZ( (float)(TrepsB::pe.z()) );
    //  hepevt1.E( (float)(TrepsB::pe.t()) );
    p1.set4Vector(TrepsB::pe);
    //  hepevt1.M( (float)(TrepsB::me) );
    p1.setMass(TrepsB::me);
    //  hepevt1.VX( 0.0 );
    //  hepevt1.VY( 0.0 );
    //  hepevt1.VZ( 0.0 );
    //  hepevt1.T( 0.0 );
    p1.setStatus(MCParticle::c_PrimaryParticle | MCParticle::c_StableInGenerator);

    auto& p2 = mpg.addParticle();
    //  hepevt2.isthep( 1 );
    //  hepevt2.idhep( -11 );
    p2.setPDG(-11);
    //    hepevt2.mother( 0 );
    //  hepevt2.mo( 0, 0 );
    //  hepevt2.daFirst( 0 );
    //  hepevt2.daLast( 0 );
    //  hepevt2.PX( (float)(TrepsB::pp.x()) );
    //  hepevt2.PY( (float)(TrepsB::pp.y()) );
    //  hepevt2.PZ( (float)(TrepsB::pp.z()) );
    //  hepevt2.E( (float)(TrepsB::pp.t()) );
    p2.set4Vector(TrepsB::pp);
    //  hepevt2.M( (float)(TrepsB::me) );
    p2.setMass(TrepsB::me);
    //  hepevt2.VX( 0.0 );
    //  hepevt2.VY( 0.0 );
    //  hepevt2.VZ( 0.0 );
    //  hepevt2.T( 0.0 );
    p2.setStatus(MCParticle::c_PrimaryParticle | MCParticle::c_StableInGenerator);


  }
  //Fill MCParticle List
  mpg.generateList(m_mcparticles.getName(), MCParticleGraph::c_setDecayInfo);
}
void trepsinputModule::terminate()
{
  if (boost::filesystem::path(rfnfc).extension().string() == std::string(".root")) {
    TFile* f = new TFile(rfnfc.c_str(), "recreate");
    if (f) {
      B2DEBUG(10, "Write histograms for check into " << rfnfc);
      f->cd();
      treh1->Write();
      treh2->Write();
      treh3->Write();
      treh4->Write();
      treh5->Write();
      treh6->Write();
      f->Flush();
      f->Close();
    }
  }
  TrepsB::terminate();
}

