/**
 * FORTRAN Interface for Photos++
 *
 * @author Tomasz Przedzinski
 * @date 20 June 2021
 */
#include <cstdlib>
#include "Photos.h"
#include "PhotosHEPEVTEvent.h"
using namespace Photospp;
using std::cout;
using std::endl;
extern "C" {
  void dark_init_(int& idspecial, int& ifspecial, int& normfact, int& normfmu, double& speciallife, double& mxx, double& gxx,
                  int& iddarkmother, int& iboson, int& iforce)
  {
    darkr.IDspecial = idspecial;   // dark PDG identifier
    darkr.ifspecial = ifspecial;   // to switch resonance decaying to lepton pair
    darkr.NormFact = normfact;  // enhance with respect to QED for electron pairs.
    darkr.NormFmu = normfmu;       //20;// extra enhancement for muon pairs.
    darkr.SpecialLife = speciallife; // its lifetime
    darkr.MXX = mxx;               // its mass
    darkr.GXX = gxx;               // and width
    darkr.IDdarkMother = iddarkmother; // PDGid of particle decaying with dark
    darkr.iboson = iboson;         // to switch from scalar to vector
    darkr.ifforce = iforce;        //
    darkr.Fel = 0.0;
    darkr.Fmu = 0.0;
  }
  void photos_init_(int& ifpairs, int& ifphotons)
  {
    Photos::initialize();
    Photos::forceMassFromEventRecord(11);

    // Turn on pair emission and turn off photon emission
    // Note that this example loops over the same event until at least one
    // particle is added, so uncommenting these two flags can be used
    // to test if a pair is correctly emitted
    if (ifpairs == 1)  Photos::setPairEmission(true);
    else            Photos::setPairEmission(false);

    if (ifphotons == 1) Photos::setPhotonEmission(true);
    else             Photos::setPhotonEmission(false);
    // darkr.IDspecial=22;     // dark PDG identifier
    //  darkr.ifspecial=1;      // to switch resonance decaying to lepton pair
    // darkr.NormFact=100;     // enhance with respect to QED for electron pairs.
    // darkr.NormFmu=1;        //20;// extra enhancement for muon pairs.
    // darkr.SpecialLife=0.0;  // its lifetime
    // darkr.MXX=0.05;         // its mass
    // darkr.GXX=0.0001;       // and width
    // darkr.IDdarkMother=23;  // PDGid of particle decaying with dark

    // Turn on NLO corrections
    //Photos::setMeCorrectionWtForW(true);
    //Photos::setMeCorrectionWtForZ(true);
  }

  void photos_process_()
  {
    PhotosHEPEVTEvent* event = new PhotosHEPEVTEvent();

    PhotosHEPEVTEvent::read_event_from_HEPEVT(event);
    //event->print();

    event->process();
    //event->print();

    PhotosHEPEVTEvent::write_event_to_HEPEVT(event);

    delete event;
  }

  void photos_process_particle_(int* i)
  {
    PhotosHEPEVTEvent* event = new PhotosHEPEVTEvent();

    PhotosHEPEVTEvent::read_event_from_HEPEVT(event);
    //event->print();

    PhotosParticle* p = event->getParticle(*i - 1);

    Photos::processParticle(p);
    //event->print();

    PhotosHEPEVTEvent::write_event_to_HEPEVT(event);

    delete event;
  }

  void photos_process_branch_(int* i)
  {
    PhotosHEPEVTEvent* event = new PhotosHEPEVTEvent();

    PhotosHEPEVTEvent::read_event_from_HEPEVT(event);
    //event->print();

    PhotosParticle* p = event->getParticle(*i - 1);

    Photos::processBranch(p);
    //event->print();

    PhotosHEPEVTEvent::write_event_to_HEPEVT(event);

    delete event;
  }
}
