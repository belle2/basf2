// @(#)root/eve:$Id: SplitGLView.C 32183 2010-02-02 11:18:27Z bellenot $
// Author: Bertrand Bellenot


#include <display/modules/display/SplitGLView.h>
#include <framework/logging/Logger.h>

#include "TFormula.h"
#include "TF1.h"
#include "TH1F.h"

using namespace Belle2;

const char* filetypes[] = {
  "ROOT files",    "*.root",
  "All files",     "*",
  0,               0
};

const char* rcfiletypes[] = {
  "All files",     "*",
  0,               0
};


ClassImp(SplitGLView)



SplitGLView::SplitGLView(const TGWindow* p, UInt_t w, UInt_t h) :
  TGMainFrame(p, w, h), fActViewer(0)
{
  // Main frame constructor.

  // create the "file" popup menu
  fMenuFile = new TGPopupMenu(gClient->GetRoot());
  fMenuFile->AddEntry("E&xit", kFileExit);

  // create the "camera" popup menu
  fMenuCamera = new TGPopupMenu(gClient->GetRoot());
  fMenuCamera->AddEntry("Perspective (Floor XOZ)", kGLPerspXOZ);
  fMenuCamera->AddEntry("Perspective (Floor YOZ)", kGLPerspYOZ);
  fMenuCamera->AddEntry("Perspective (Floor XOY)", kGLPerspXOY);
  fMenuCamera->AddEntry("Orthographic (XOY)", kGLXOY);
  fMenuCamera->AddEntry("Orthographic (XOZ)", kGLXOZ);
  fMenuCamera->AddEntry("Orthographic (ZOY)", kGLZOY);
  fMenuCamera->AddSeparator();
  fMenuCamera->AddEntry("Ortho allow rotate", kGLOrthoRotate);
  fMenuCamera->AddEntry("Ortho allow dolly",  kGLOrthoDolly);

  fMenuScene = new TGPopupMenu(gClient->GetRoot());
  fMenuScene->AddEntry("&Update Current", kSceneUpdate);
  fMenuScene->AddEntry("Update &All", kSceneUpdateAll);

  // create the "help" popup menu
  fMenuHelp = new TGPopupMenu(gClient->GetRoot());
  fMenuHelp->AddEntry("&About", kHelpAbout);

  // create the main menu bar
  fMenuBar = new TGMenuBar(this, 1, 1, kHorizontalFrame);
  fMenuBar->AddPopup("&File", fMenuFile, new TGLayoutHints(kLHintsTop |
                                                           kLHintsLeft, 0, 4, 0, 0));
  fMenuBar->AddPopup("&Camera", fMenuCamera, new TGLayoutHints(kLHintsTop |
                     kLHintsLeft, 0, 4, 0, 0));
  fMenuBar->AddPopup("&Scene", fMenuScene, new TGLayoutHints(kLHintsTop |
                                                             kLHintsLeft, 0, 4, 0, 0));
  fMenuBar->AddPopup("&Help", fMenuHelp, new TGLayoutHints(kLHintsTop |
                                                           kLHintsRight));

  AddFrame(fMenuBar, new TGLayoutHints(kLHintsTop | kLHintsExpandX));

  // connect menu signals to our menu handler slot
  fMenuFile->Connect("Activated(Int_t)", "Belle2::SplitGLView", this,
                     "HandleMenu(Int_t)");
  fMenuCamera->Connect("Activated(Int_t)", "Belle2::SplitGLView", this,
                       "HandleMenu(Int_t)");
  fMenuScene->Connect("Activated(Int_t)", "Belle2::SplitGLView", this,
                      "HandleMenu(Int_t)");
  fMenuHelp->Connect("Activated(Int_t)", "Belle2::SplitGLView", this,
                     "HandleMenu(Int_t)");

  if (gEve) {
    // use status bar from the browser
    fStatusBar = gEve->GetBrowser()->GetStatusBar();
  } else {
    // create the status bar
    Int_t parts[] = {45, 15, 10, 30};
    fStatusBar = new TGStatusBar(this, 50, 10);
    fStatusBar->SetParts(parts, 4);
    AddFrame(fStatusBar, new TGLayoutHints(kLHintsBottom | kLHintsExpandX,
                                           0, 0, 10, 0));
  }

  // create eve pad (our geometry container)
  fPad = new TEvePad();
  fPad->SetFillColor(kBlack);

  // create the split frames
  fSplitFrame = new TGSplitFrame(this, 800, 600);
  AddFrame(fSplitFrame, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
  // split it once
  fSplitFrame->HSplit(434);
  // then split each part again (this will make four parts)
  fSplitFrame->GetSecond()->VSplit(266);
  //fSplitFrame->GetSecond()->GetSecond()->VSplit(266);

  for (int iFrame = 0; iFrame < 3; iFrame++) {
    TGSplitFrame* frm;
    if (iFrame == 0) {
      // get top (main) split frame
      frm = fSplitFrame->GetFirst();
      frm->SetName("Main_View");
    } else if (iFrame == 1) {
      // get bottom left split frame
      frm = fSplitFrame->GetSecond()->GetFirst();
      frm->SetName("Bottom_Left");
    } else {
      // get bottom center split frame
      frm = fSplitFrame->GetSecond()->GetSecond();
      frm->SetName("Bottom_Center");
    }

    // create (embed) a GL viewer inside
    fGLViewer[iFrame] = new TGLEmbeddedViewer(frm, fPad);
    /*
    TGLOverlayButton* but1, *but2;
    but1 = new TGLOverlayButton(fGLViewer[iFrame], "Swap", 10.0, -10.0, 55.0, 16.0);
    but1->Connect("Clicked(TGLViewerBase*)", "Belle2::SplitGLView", this, "SwapToMainView(TGLViewerBase*)");
    but2 = new TGLOverlayButton(fGLViewer[iFrame], "Undock", 70.0, -10.0, 55.0, 16.0);
    but2->Connect("Clicked(TGLViewerBase*)", "Belle2::SplitGLView", this, "UnDock(TGLViewerBase*)");
    */
    frm->AddFrame(fGLViewer[iFrame]->GetFrame(), new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
    TEveProjectionManager* projectionMgr = 0;
    if (iFrame == 0) {
      // set the camera to perspective (XOZ) for this viewer
      fGLViewer[iFrame]->SetCurrentCamera(TGLViewer::kCameraPerspXOZ);
    } else if (iFrame == 1) {
      // set the camera to orthographic (XOY) for this viewer
      fGLViewer[iFrame]->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);
      fRhoZMgr = new TEveProjectionManager(TEveProjection::kPT_RhoZ);
      projectionMgr = fRhoZMgr;
    } else {
      // set the camera to orthographic (XOY) for this viewer
      fGLViewer[iFrame]->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);
      fRPhiMgr = new TEveProjectionManager(TEveProjection::kPT_RPhi);
      projectionMgr = fRPhiMgr;
    }

    // connect signals we are interested in
    fGLViewer[iFrame]->Connect("MouseOver(TGLPhysicalShape*)", "Belle2::SplitGLView", this,
                               "OnMouseOver(TGLPhysicalShape*)");
    fGLViewer[iFrame]->Connect("Clicked(TObject*)", "Belle2::SplitGLView", this,
                               "OnClicked(TObject*)");
    fViewer[iFrame] = new TEveViewer(TString::Format("SplitGLViewer[%d]", iFrame));
    fViewer[iFrame]->SetGLViewer(fGLViewer[iFrame], fGLViewer[iFrame]->GetFrame());
    fViewer[iFrame]->IncDenyDestroy();
    if (gEve) {
      if (iFrame == 0) {
        fViewer[iFrame]->AddScene(gEve->GetGlobalScene());
        fViewer[iFrame]->AddScene(gEve->GetEventScene());
      }
      TEveScene* s = 0;
      if (projectionMgr) {
        if (iFrame == 1)
          s = gEve->SpawnNewScene("Rho-Z projection");
        if (iFrame == 2)
          s = gEve->SpawnNewScene("R-Phi projection");
        fViewer[iFrame]->AddScene(s);
      }

      gEve->GetViewers()->AddElement(fViewer[iFrame]);
      if (projectionMgr) {
        s->AddElement(projectionMgr);
        gEve->AddToListTree(projectionMgr, kTRUE);
      }
    }

  }
  setActiveViewer(fGLViewer[0]);

  if (gEve) {
    gEve->GetListTree()->Connect("Clicked(TGListTreeItem*, Int_t, Int_t, Int_t)",
                                 "Belle2::SplitGLView", this, "ItemClicked(TGListTreeItem*, Int_t, Int_t, Int_t)");
  }

  Resize(GetDefaultSize());
  MapSubwindows();
  MapWindow();
}

SplitGLView::~SplitGLView()
{
  // Clean up main frame...

  fMenuFile->Disconnect("Activated(Int_t)", this, "HandleMenu(Int_t)");
  fMenuCamera->Disconnect("Activated(Int_t)", this, "HandleMenu(Int_t)");
  fMenuScene->Disconnect("Activated(Int_t)", this, "HandleMenu(Int_t)");
  fMenuHelp->Disconnect("Activated(Int_t)", this, "HandleMenu(Int_t)");
  for (int i = 0; i < 3; i++) {
    fGLViewer[i]->Disconnect("MouseOver(TGLPhysicalShape*)", this,
                             "OnMouseOver(TGLPhysicalShape*)");
    delete fGLViewer[i];
  }
  delete fMenuFile;
  delete fMenuScene;
  delete fMenuCamera;
  delete fMenuHelp;
  delete fSplitFrame;
  delete fPad;
}

void SplitGLView::HandleMenu(Int_t id)
{
  // Handle menu items.

  static const TString rcdir(".");
  static const TString rcfile(".everc");

  switch (id) {

    case kFileExit:
      CloseWindow();
      break;

    case kGLPerspYOZ:
      if (fActViewer)
        fActViewer->SetCurrentCamera(TGLViewer::kCameraPerspYOZ);
      break;
    case kGLPerspXOZ:
      if (fActViewer)
        fActViewer->SetCurrentCamera(TGLViewer::kCameraPerspXOZ);
      break;
    case kGLPerspXOY:
      if (fActViewer)
        fActViewer->SetCurrentCamera(TGLViewer::kCameraPerspXOY);
      break;
    case kGLXOY:
      if (fActViewer)
        fActViewer->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);
      break;
    case kGLXOZ:
      if (fActViewer)
        fActViewer->SetCurrentCamera(TGLViewer::kCameraOrthoXOZ);
      break;
    case kGLZOY:
      if (fActViewer)
        fActViewer->SetCurrentCamera(TGLViewer::kCameraOrthoZOY);
      break;
    case kGLOrthoRotate:
      ToggleOrthoRotate();
      break;
    case kGLOrthoDolly:
      ToggleOrthoDolly();
      break;

    case kSceneUpdate:
      if (fActViewer)
        fActViewer->UpdateScene();
      break;

    case kSceneUpdateAll:
      for (int i = 0; i < 3; i++)
        fGLViewer[i]->UpdateScene();

      break;

    case kHelpAbout: {
#ifdef R__UNIX
      TString rootx;
# ifdef ROOTBINDIR
      rootx = ROOTBINDIR;
# else
      rootx = gSystem->Getenv("ROOTSYS");
      if (!rootx.IsNull()) rootx += "/bin";
# endif
      rootx += "/root -a &";
      gSystem->Exec(rootx);
#else
#ifdef WIN32
      new TWin32SplashThread(kTRUE);
#else
      char str[32];
      sprintf(str, "About ROOT %s...", gROOT->GetVersion());
      hd = new TRootHelpDialog(this, str, 600, 400);
      hd->SetText(gHelpAbout);
      hd->Popup();
#endif
#endif
    }
    break;

    default:
      break;
  }
}

void SplitGLView::OnClicked(TObject* obj)
{
  // Handle click events in GL viewer

  if (obj)
    fStatusBar->SetText(Form("User clicked on: \"%s\"", obj->GetName()), 1);
  else
    fStatusBar->SetText("", 1);

  // change the active GL viewer to the one who emitted the signal
  TGLEmbeddedViewer* sender = dynamic_cast<TGLEmbeddedViewer*>(static_cast<TQObject*>(gTQSender));

  if (!sender) {
    B2WARNING("OnClicked() signal not from a TGLEmbeddedViewer?");
    return;
  }
  setActiveViewer(sender);
}

void SplitGLView::setActiveViewer(TGLEmbeddedViewer* v)
{
  // set the last active GL viewer frame to default color
  if (fActViewer && fActViewer->GetFrame())
    fActViewer->GetFrame()->ChangeBackground(GetDefaultFrameBackground());

  fActViewer = v;

  static Pixel_t green = 0;
  // get the highlight color (only once)
  if (green == 0) {
    gClient->GetColorByName("green", green);
  }
  // set the new active GL viewer frame to highlight color
  if (fActViewer->GetFrame())
    fActViewer->GetFrame()->ChangeBackground(green);

  // update menu entries to match active viewer's options
  if (fActViewer->GetOrthoXOYCamera()->GetDollyToZoom() &&
      fActViewer->GetOrthoXOZCamera()->GetDollyToZoom() &&
      fActViewer->GetOrthoZOYCamera()->GetDollyToZoom())
    fMenuCamera->UnCheckEntry(kGLOrthoDolly);
  else
    fMenuCamera->CheckEntry(kGLOrthoDolly);

  if (fActViewer->GetOrthoXOYCamera()->GetEnableRotate() &&
      fActViewer->GetOrthoXOYCamera()->GetEnableRotate() &&
      fActViewer->GetOrthoXOYCamera()->GetEnableRotate())
    fMenuCamera->CheckEntry(kGLOrthoRotate);
  else
    fMenuCamera->UnCheckEntry(kGLOrthoRotate);
}


void SplitGLView::OnMouseOver(TGLPhysicalShape* shape)
{
  // Slot used to handle "OnMouseOver" signal coming from any GL viewer.
  // We receive a pointer on the physical shape in which the mouse cursor is.

  // display informations on the physical shape in the status bar
  if (shape && shape->GetLogical() && shape->GetLogical()->GetExternal())
    fStatusBar->SetText(Form("Mouse Over: \"%s\"",
                             shape->GetLogical()->GetExternal()->GetName()), 0);
  else
    fStatusBar->SetText("", 0);
}


void SplitGLView::ToggleOrthoRotate()
{
  // Toggle state of the 'Ortho allow rotate' menu entry.

  if (fMenuCamera->IsEntryChecked(kGLOrthoRotate))
    fMenuCamera->UnCheckEntry(kGLOrthoRotate);
  else
    fMenuCamera->CheckEntry(kGLOrthoRotate);
  Bool_t state = fMenuCamera->IsEntryChecked(kGLOrthoRotate);
  if (fActViewer) {
    fActViewer->GetOrthoXOYCamera()->SetEnableRotate(state);
    fActViewer->GetOrthoXOYCamera()->SetEnableRotate(state);
    fActViewer->GetOrthoXOYCamera()->SetEnableRotate(state);
  }
}

void SplitGLView::ToggleOrthoDolly()
{
  // Toggle state of the 'Ortho allow dolly' menu entry.

  if (fMenuCamera->IsEntryChecked(kGLOrthoDolly))
    fMenuCamera->UnCheckEntry(kGLOrthoDolly);
  else
    fMenuCamera->CheckEntry(kGLOrthoDolly);
  Bool_t state = ! fMenuCamera->IsEntryChecked(kGLOrthoDolly);
  if (fActViewer) {
    fActViewer->GetOrthoXOYCamera()->SetDollyToZoom(state);
    fActViewer->GetOrthoXOZCamera()->SetDollyToZoom(state);
    fActViewer->GetOrthoZOYCamera()->SetDollyToZoom(state);
  }
}

void SplitGLView::ItemClicked(TGListTreeItem* item, Int_t, Int_t, Int_t)
{
  // Item has been clicked, based on mouse button do:

  static const TEveException eh("SplitGLView::ItemClicked ");
  TEveElement* re = (TEveElement*)item->GetUserData();
  if (re == 0) return;
  TObject* obj = re->GetObject(eh);
  if (obj->InheritsFrom("TEveViewer")) {
    TGLViewer* v = ((TEveViewer*)obj)->GetGLViewer();
    //v->Activated();
    if (v->InheritsFrom("TGLEmbeddedViewer")) {
      TGLEmbeddedViewer* ev = (TGLEmbeddedViewer*)v;
      gVirtualX->SetInputFocus(ev->GetGLWidget()->GetId());
    }
  }
}

void SplitGLView::SwapToMainView(TGLViewerBase* viewer)
{
  // Swap frame embedded in a splitframe to the main view (slot method).

  TGCompositeFrame* parent = 0;
  if (!fSplitFrame->GetFirst()->GetFrame())
    return;
  if (viewer == 0) {
    TGPictureButton* src = (TGPictureButton*)gTQSender;
    parent = (TGCompositeFrame*)src->GetParent();
    while (parent && !parent->InheritsFrom("TGSplitFrame")) {
      parent = (TGCompositeFrame*)parent->GetParent();
    }
  } else {
    TGCompositeFrame* src = ((TGLEmbeddedViewer*)viewer)->GetFrame();
    if (!src) return;
    //TGLOverlayButton* but = (TGLOverlayButton*)((TQObject*)gTQSender);
    //but->ResetState();
    parent = (TGCompositeFrame*)src->GetParent();
  }
  if (parent && parent->InheritsFrom("TGSplitFrame"))
    ((TGSplitFrame*)parent)->SwitchToMain();
}

void SplitGLView::UnDock(TGLViewerBase* viewer)
{
  // Undock frame embedded in a splitframe (slot method).

  TGCompositeFrame* src = ((TGLEmbeddedViewer*)viewer)->GetFrame();
  if (!src) return;
  //TGLOverlayButton* but = (TGLOverlayButton*)((TQObject*)gTQSender);
  //but->ResetState();
  TGCompositeFrame* parent = (TGCompositeFrame*)src->GetParent();
  if (parent && parent->InheritsFrom("TGSplitFrame"))
    ((TGSplitFrame*)parent)->ExtractFrame();
}
