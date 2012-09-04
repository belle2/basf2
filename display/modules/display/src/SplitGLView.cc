#include <display/modules/display/SplitGLView.h>
#include <framework/logging/Logger.h>

#include "TApplication.h"
#include "TGButton.h"
#include "TGeoManager.h"
#include "TSystem.h"

using namespace Belle2;


ClassImp(SplitGLView)


SplitGLView::SplitGLView(const TGWindow* p, UInt_t w, UInt_t h) :
  TGMainFrame(p, w, h), m_activeViewer(0)
{
  // Main frame constructor.

  // create the "camera" popup menu
  m_cameraMenu = new TGPopupMenu(gClient->GetRoot());
  m_cameraMenu->AddEntry("Perspective (Floor XOZ)", kGLPerspXOZ);
  m_cameraMenu->AddEntry("Perspective (Floor YOZ)", kGLPerspYOZ);
  m_cameraMenu->AddEntry("Perspective (Floor XOY)", kGLPerspXOY);
  m_cameraMenu->AddEntry("Orthographic (XOY)", kGLXOY);
  m_cameraMenu->AddEntry("Orthographic (XOZ)", kGLXOZ);
  m_cameraMenu->AddEntry("Orthographic (ZOY)", kGLZOY);
  m_cameraMenu->AddSeparator();
  m_cameraMenu->AddEntry("Ortho allow rotate", kGLOrthoRotate);
  m_cameraMenu->AddEntry("Ortho allow dolly",  kGLOrthoDolly);

  TGPopupMenu* sceneMenu = new TGPopupMenu(gClient->GetRoot());
  sceneMenu->AddEntry("&Update Current", kSceneUpdate);
  sceneMenu->AddEntry("Update &All", kSceneUpdateAll);

  // create the "help" popup menu
  TGPopupMenu* helpMenu = new TGPopupMenu(gClient->GetRoot());
  helpMenu->AddEntry("&About", kHelpAbout);

  // create the main menu bar
  TGMenuBar* menuBar = new TGMenuBar(this, 1, 1, kHorizontalFrame);
  menuBar->AddPopup("&Camera", m_cameraMenu, new TGLayoutHints(kLHintsTop |
                    kLHintsLeft, 0, 4, 0, 0));
  menuBar->AddPopup("&Scene", sceneMenu, new TGLayoutHints(kLHintsTop |
                                                           kLHintsLeft, 0, 4, 0, 0));
  menuBar->AddPopup("&Help", helpMenu, new TGLayoutHints(kLHintsTop |
                                                         kLHintsRight));

  AddFrame(menuBar, new TGLayoutHints(kLHintsTop | kLHintsExpandX));

  // connect menu signals to our menu handler slot
  m_cameraMenu->Connect("Activated(Int_t)", "Belle2::SplitGLView", this,
                        "handleMenu(Int_t)");
  sceneMenu->Connect("Activated(Int_t)", "Belle2::SplitGLView", this,
                     "handleMenu(Int_t)");
  helpMenu->Connect("Activated(Int_t)", "Belle2::SplitGLView", this,
                    "handleMenu(Int_t)");

  if (gEve) {
    // use status bar from the browser
    m_statusBar = gEve->GetBrowser()->GetStatusBar();
  } else {
    // create the status bar
    Int_t parts[] = {45, 15, 10, 30};
    m_statusBar = new TGStatusBar(this, 50, 10);
    m_statusBar->SetParts(parts, 4);
    AddFrame(m_statusBar, new TGLayoutHints(kLHintsBottom | kLHintsExpandX,
                                            0, 0, 10, 0));
  }

  // create eve pad (our geometry container)
  m_pad = new TEvePad();
  m_pad->SetFillColor(kBlack);

  // create the split frames
  m_splitFrame = new TGSplitFrame(this, 800, 600);
  AddFrame(m_splitFrame, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
  // split it once
  m_splitFrame->HSplit(434);
  // then split each part again (this will make four parts)
  m_splitFrame->GetSecond()->VSplit(266);
  //m_splitFrame->GetSecond()->GetSecond()->VSplit(266);

  for (int iFrame = 0; iFrame < 3; iFrame++) {
    TGSplitFrame* frm;
    if (iFrame == 0) {
      // get top (main) split frame
      frm = m_splitFrame->GetFirst();
      frm->SetName("Main_View");
    } else if (iFrame == 1) {
      // get bottom left split frame
      frm = m_splitFrame->GetSecond()->GetFirst();
      frm->SetName("Bottom_Left");
    } else {
      // get bottom center split frame
      frm = m_splitFrame->GetSecond()->GetSecond();
      frm->SetName("Bottom_Center");
    }

    // create (embed) a GL viewer inside
    m_glViewer[iFrame] = new TGLEmbeddedViewer(frm, m_pad);
    /*
    TGLOverlayButton* but1, *but2;
    but1 = new TGLOverlayButton(m_glViewer[iFrame], "Swap", 10.0, -10.0, 55.0, 16.0);
    but1->Connect("Clicked(TGLViewerBase*)", "Belle2::SplitGLView", this, "swapToMainView(TGLViewerBase*)");
    but2 = new TGLOverlayButton(m_glViewer[iFrame], "Undock", 70.0, -10.0, 55.0, 16.0);
    but2->Connect("Clicked(TGLViewerBase*)", "Belle2::SplitGLView", this, "unDock(TGLViewerBase*)");
    */
    frm->AddFrame(m_glViewer[iFrame]->GetFrame(), new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
    TEveProjectionManager* projectionMgr = 0;
    TString projectionName;
    if (iFrame == 0) {
      // set the camera to perspective (XOZ) for this viewer
      m_glViewer[iFrame]->SetCurrentCamera(TGLViewer::kCameraPerspXOZ);
      projectionName = "3D";
    } else if (iFrame == 1) {
      // set the camera to orthographic (XOY) for this viewer
      m_glViewer[iFrame]->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);
      m_rhozManager = new TEveProjectionManager(TEveProjection::kPT_RhoZ);
      projectionMgr = m_rhozManager;
      projectionName = "Rho-Z";
    } else {
      // set the camera to orthographic (XOY) for this viewer
      m_glViewer[iFrame]->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);
      m_rphiManager = new TEveProjectionManager(TEveProjection::kPT_RPhi);
      projectionMgr = m_rphiManager;
      projectionName = "R-Phi";
    }

    // connect signals we are interested in
    m_glViewer[iFrame]->Connect("MouseOver(TGLPhysicalShape*)", "Belle2::SplitGLView", this,
                                "onMouseOver(TGLPhysicalShape*)");
    m_glViewer[iFrame]->Connect("Clicked(TObject*)", "Belle2::SplitGLView", this,
                                "onClicked(TObject*)");
    m_viewer[iFrame] = new TEveViewer(TString::Format("%s viewer", projectionName.Data()));
    m_viewer[iFrame]->SetGLViewer(m_glViewer[iFrame], m_glViewer[iFrame]->GetFrame());
    m_viewer[iFrame]->IncDenyDestroy();
    if (gEve) {
      if (iFrame == 0) {
        m_viewer[iFrame]->AddScene(gEve->GetGlobalScene());
        m_viewer[iFrame]->AddScene(gEve->GetEventScene());
      }
      TEveScene* s = 0;
      if (projectionMgr) {
        s = gEve->SpawnNewScene(TString::Format("%s projection", projectionName.Data()));
        m_viewer[iFrame]->AddScene(s);
      }

      gEve->GetViewers()->AddElement(m_viewer[iFrame]);
      if (projectionMgr) {
        s->AddElement(projectionMgr);
        gEve->AddToListTree(projectionMgr, kTRUE);
      }
    }

  }
  setActiveViewer(m_glViewer[0]);

  if (gEve) {
    gEve->GetListTree()->Connect("Clicked(TGListTreeItem*, Int_t, Int_t, Int_t)",
                                 "Belle2::SplitGLView", this, "itemClicked(TGListTreeItem*, Int_t, Int_t, Int_t)");
  }

  Resize(GetDefaultSize());
  MapSubwindows();
  MapWindow();
}

SplitGLView::~SplitGLView()
{
  // Clean up main frame...
  for (int i = 0; i < 3; i++) {
    delete m_glViewer[i];
  }
  delete m_cameraMenu;
  delete m_splitFrame;
  delete m_pad;
}

void SplitGLView::handleMenu(Int_t id)
{
  // Handle menu items.

  static const TString rcdir(".");
  static const TString rcfile(".everc");

  switch (id) {
    case kGLPerspYOZ:
      if (m_activeViewer)
        m_activeViewer->SetCurrentCamera(TGLViewer::kCameraPerspYOZ);
      break;
    case kGLPerspXOZ:
      if (m_activeViewer)
        m_activeViewer->SetCurrentCamera(TGLViewer::kCameraPerspXOZ);
      break;
    case kGLPerspXOY:
      if (m_activeViewer)
        m_activeViewer->SetCurrentCamera(TGLViewer::kCameraPerspXOY);
      break;
    case kGLXOY:
      if (m_activeViewer)
        m_activeViewer->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);
      break;
    case kGLXOZ:
      if (m_activeViewer)
        m_activeViewer->SetCurrentCamera(TGLViewer::kCameraOrthoXOZ);
      break;
    case kGLZOY:
      if (m_activeViewer)
        m_activeViewer->SetCurrentCamera(TGLViewer::kCameraOrthoZOY);
      break;
    case kGLOrthoRotate:
      toggleOrthoRotate();
      break;
    case kGLOrthoDolly:
      toggleOrthoDolly();
      break;

    case kSceneUpdate:
      if (m_activeViewer)
        m_activeViewer->UpdateScene();
      break;

    case kSceneUpdateAll:
      for (int i = 0; i < 3; i++)
        m_glViewer[i]->UpdateScene();

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

void SplitGLView::onClicked(TObject* obj)
{
  // Handle click events in GL viewer

  if (obj)
    m_statusBar->SetText(Form("User clicked on: \"%s\"", obj->GetName()), 1);
  else
    m_statusBar->SetText("", 1);

  // change the active GL viewer to the one who emitted the signal
  TGLEmbeddedViewer* sender = dynamic_cast<TGLEmbeddedViewer*>(static_cast<TQObject*>(gTQSender));

  if (!sender) {
    B2WARNING("onClicked() signal not from a TGLEmbeddedViewer?");
    return;
  }
  setActiveViewer(sender);
}

void SplitGLView::setActiveViewer(TGLEmbeddedViewer* v)
{
  // set the last active GL viewer frame to default color
  if (m_activeViewer && m_activeViewer->GetFrame())
    m_activeViewer->GetFrame()->ChangeBackground(GetDefaultFrameBackground());

  m_activeViewer = v;

  static Pixel_t green = 0;
  // get the highlight color (only once)
  if (green == 0) {
    gClient->GetColorByName("green", green);
  }
  // set the new active GL viewer frame to highlight color
  if (m_activeViewer->GetFrame())
    m_activeViewer->GetFrame()->ChangeBackground(green);

  // update menu entries to match active viewer's options
  if (m_activeViewer->GetOrthoXOYCamera()->GetDollyToZoom() &&
      m_activeViewer->GetOrthoXOZCamera()->GetDollyToZoom() &&
      m_activeViewer->GetOrthoZOYCamera()->GetDollyToZoom())
    m_cameraMenu->UnCheckEntry(kGLOrthoDolly);
  else
    m_cameraMenu->CheckEntry(kGLOrthoDolly);

  if (m_activeViewer->GetOrthoXOYCamera()->GetEnableRotate() &&
      m_activeViewer->GetOrthoXOYCamera()->GetEnableRotate() &&
      m_activeViewer->GetOrthoXOYCamera()->GetEnableRotate())
    m_cameraMenu->CheckEntry(kGLOrthoRotate);
  else
    m_cameraMenu->UnCheckEntry(kGLOrthoRotate);
}


void SplitGLView::onMouseOver(TGLPhysicalShape* shape)
{
  // Slot used to handle "onMouseOver" signal coming from any GL viewer.
  // We receive a pointer on the physical shape in which the mouse cursor is.

  // display informations on the physical shape in the status bar
  if (shape && shape->GetLogical() && shape->GetLogical()->GetExternal())
    m_statusBar->SetText(Form("Mouse Over: \"%s\"",
                              shape->GetLogical()->GetExternal()->GetName()), 0);
  else
    m_statusBar->SetText("", 0);
}


void SplitGLView::toggleOrthoRotate()
{
  // Toggle state of the 'Ortho allow rotate' menu entry.

  if (m_cameraMenu->IsEntryChecked(kGLOrthoRotate))
    m_cameraMenu->UnCheckEntry(kGLOrthoRotate);
  else
    m_cameraMenu->CheckEntry(kGLOrthoRotate);
  Bool_t state = m_cameraMenu->IsEntryChecked(kGLOrthoRotate);
  if (m_activeViewer) {
    m_activeViewer->GetOrthoXOYCamera()->SetEnableRotate(state);
    m_activeViewer->GetOrthoXOYCamera()->SetEnableRotate(state);
    m_activeViewer->GetOrthoXOYCamera()->SetEnableRotate(state);
  }
}

void SplitGLView::toggleOrthoDolly()
{
  // Toggle state of the 'Ortho allow dolly' menu entry.

  if (m_cameraMenu->IsEntryChecked(kGLOrthoDolly))
    m_cameraMenu->UnCheckEntry(kGLOrthoDolly);
  else
    m_cameraMenu->CheckEntry(kGLOrthoDolly);
  Bool_t state = ! m_cameraMenu->IsEntryChecked(kGLOrthoDolly);
  if (m_activeViewer) {
    m_activeViewer->GetOrthoXOYCamera()->SetDollyToZoom(state);
    m_activeViewer->GetOrthoXOZCamera()->SetDollyToZoom(state);
    m_activeViewer->GetOrthoZOYCamera()->SetDollyToZoom(state);
  }
}

void SplitGLView::itemClicked(TGListTreeItem* item, Int_t, Int_t, Int_t)
{
  // Item has been clicked, based on mouse button do:

  static const TEveException eh("SplitGLView::itemClicked ");
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

void SplitGLView::swapToMainView(TGLViewerBase* viewer)
{
  // Swap frame embedded in a splitframe to the main view (slot method).

  TGCompositeFrame* parent = 0;
  if (!m_splitFrame->GetFirst()->GetFrame())
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

void SplitGLView::unDock(TGLViewerBase* viewer)
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
