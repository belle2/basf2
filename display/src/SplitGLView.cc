#include <display/SplitGLView.h>
#include <framework/logging/Logger.h>

#include "TGFrame.h"
#include "TEveGeoNode.h"
#include "TEveScene.h"
#include "TEveEventManager.h"
#include "TEveSelection.h"
#include "TEveViewer.h"
#include "TEveWindowManager.h"
#include "TGButton.h"
#include "TGMenu.h"
#include "TGStatusBar.h"
#include "TGLPhysicalShape.h"
#include "TGLLogicalShape.h"
#include "TGeoManager.h"
#include "TString.h"

using namespace Belle2;


ClassImp(SplitGLView)

SplitGLView::SplitGLView() :
  m_activeViewer(-1)
{
  TEveWindowSlot* slot = TEveWindow::CreateWindowMainFrame();
  TEveWindowPack* pack = slot->MakePack();
  pack->SetVertical();
  pack->SetShowTitleBar(kFALSE);

  const char* projectionName[3] = { "3D", "Rho/Z", "R/Phi" };
  TGLViewer::ECameraType cameraType[3] = { TGLViewer::kCameraPerspXOZ, TGLViewer::kCameraOrthoXOY, TGLViewer::kCameraOrthoXOY };

  //0: 3d, 1: rhoz, 2: rphi
  for (int iFrame = 0; iFrame < 3; iFrame++) {
    if (iFrame == 1) {
      //split bottom region again
      slot = pack->NewSlot();
      pack = slot->MakePack();
      pack->SetHorizontal();
      pack->SetShowTitleBar(kFALSE);
    }
    slot = pack->NewSlot();

    TEveViewer* viewer = new TEveViewer(TString::Format("%s viewer", projectionName[iFrame]));
    m_glViewer[iFrame] = viewer->SpawnGLEmbeddedViewer();
    slot->ReplaceWindow(viewer);
    slot = 0; //invalid after ReplaceWindow()
    viewer->SetShowTitleBar(kFALSE); //might want to show these?
    m_window[iFrame] = viewer;

    m_glViewer[iFrame]->SetCurrentCamera(cameraType[iFrame]);
    //create projection managers
    TEveProjectionManager* projectionMgr = 0;
    if (iFrame == 1) {
      m_rhozManager = new TEveProjectionManager(TEveProjection::kPT_RhoZ);
      projectionMgr = m_rhozManager;
    } else if (iFrame == 2) {
      m_rphiManager = new TEveProjectionManager(TEveProjection::kPT_RPhi);
      projectionMgr = m_rphiManager;
    }

    // connect signals we are interested in
    m_glViewer[iFrame]->Connect("MouseOver(TGLPhysicalShape*)", "Belle2::SplitGLView", this, "onMouseOver(TGLPhysicalShape*)");
    m_glViewer[iFrame]->Connect("Clicked(TObject*)", "Belle2::SplitGLView", this, "onClicked(TObject*)");
    if (iFrame == 0) {
      viewer->AddScene(gEve->GetGlobalScene());
      viewer->AddScene(gEve->GetEventScene());
    }
    TEveScene* s = 0;
    if (projectionMgr) {
      s = gEve->SpawnNewScene(TString::Format("%s projection", projectionName[iFrame]));
      viewer->AddScene(s);
    }

    gEve->GetViewers()->AddElement(viewer);
    if (projectionMgr) {
      s->AddElement(projectionMgr);
      gEve->AddToListTree(projectionMgr, kTRUE);
    }
  }

  // create the "camera" popup menu
  m_cameraMenu = new TGPopupMenu(gClient->GetDefaultRoot());
  m_cameraMenu->AddEntry("Perspective (Floor X/Z)", kGLPerspXOZ);
  m_cameraMenu->AddEntry("Perspective (Floor Y/Z)", kGLPerspYOZ);
  m_cameraMenu->AddEntry("Perspective (Floor X/Y)", kGLPerspXOY);
  m_cameraMenu->AddEntry("Orthographic (X/Y)", kGLXOY);
  m_cameraMenu->AddEntry("Orthographic (X/Z)", kGLXOZ);
  m_cameraMenu->AddEntry("Orthographic (Z/Y)", kGLZOY);
  m_cameraMenu->AddSeparator();
  m_cameraMenu->AddEntry("Orthographic: Allow rotating", kGLOrthoRotate);
  m_cameraMenu->AddEntry("Orthographic: Right click to dolly", kGLOrthoDolly);

  TGPopupMenu* sceneMenu = new TGPopupMenu(gClient->GetDefaultRoot());
  sceneMenu->AddEntry("&Update Current", kSceneUpdate);
  sceneMenu->AddEntry("Update &All", kSceneUpdateAll);
  sceneMenu->AddSeparator();
  sceneMenu->AddEntry("Save &Geometry Extract", kSaveGeometryExtract);

  TGMenuBar* menuBar = gEve->GetBrowser()->GetMenuBar();
  menuBar->AddPopup("&Camera", m_cameraMenu, new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0));
  menuBar->AddPopup("&Scene", sceneMenu, new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0));

  // connect menu signals to our menu handler slot
  m_cameraMenu->Connect("Activated(Int_t)", "Belle2::SplitGLView", this, "handleMenu(Int_t)");
  sceneMenu->Connect("Activated(Int_t)", "Belle2::SplitGLView", this, "handleMenu(Int_t)");

  // use status bar from the browser
  m_statusBar = gEve->GetBrowser()->GetStatusBar();

  //needs to come after menu
  setActiveViewer(m_glViewer[0]);

  gEve->GetListTree()->Connect("Clicked(TGListTreeItem*, Int_t, Int_t, Int_t)",
                               "Belle2::SplitGLView", this, "itemClicked(TGListTreeItem*, Int_t, Int_t, Int_t)");

  //Without this, our own menu bar entries are not drawn (might appear later)
  gEve->GetBrowser()->Resize(gEve->GetBrowser()->GetDefaultSize());
}

SplitGLView::~SplitGLView()
{
  // Clean up main frame...
  for (int i = 0; i < 3; i++) {
    delete m_glViewer[i];
  }
  delete m_cameraMenu;
}

void SplitGLView::saveExtract()
{
  TGeoManager* my_tgeomanager = gGeoManager;
  TEveGeoTopNode* eve_top_node = dynamic_cast<TEveGeoTopNode*>(gEve->GetGlobalScene()->FirstChild());
  if (!eve_top_node) {
    B2ERROR("Couldn't find TEveGeoTopNode");
    return;
  }
  eve_top_node->ExpandIntoListTrees();
  eve_top_node->SaveExtract("geometry_extract.root", "Extract", false);

  //this doesn't work too well (i.e. crashes when geometry is drawn)
  //eve_top_node->ExpandIntoListTreesRecursively();
  //eve_top_node->SaveExtract("display_geometry_full.root", "Extract", false);
  gGeoManager = my_tgeomanager;
}

void SplitGLView::handleMenu(Int_t id)
{
  // Handle menu items.

  switch (id) {
    case kGLPerspYOZ:
      if (getActiveGLViewer())
        getActiveGLViewer()->SetCurrentCamera(TGLViewer::kCameraPerspYOZ);
      break;
    case kGLPerspXOZ:
      if (getActiveGLViewer())
        getActiveGLViewer()->SetCurrentCamera(TGLViewer::kCameraPerspXOZ);
      break;
    case kGLPerspXOY:
      if (getActiveGLViewer())
        getActiveGLViewer()->SetCurrentCamera(TGLViewer::kCameraPerspXOY);
      break;
    case kGLXOY:
      if (getActiveGLViewer())
        getActiveGLViewer()->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);
      break;
    case kGLXOZ:
      if (getActiveGLViewer())
        getActiveGLViewer()->SetCurrentCamera(TGLViewer::kCameraOrthoXOZ);
      break;
    case kGLZOY:
      if (getActiveGLViewer())
        getActiveGLViewer()->SetCurrentCamera(TGLViewer::kCameraOrthoZOY);
      break;
    case kGLOrthoRotate:
      toggleOrthoRotate();
      break;
    case kGLOrthoDolly:
      toggleOrthoDolly();
      break;

    case kSceneUpdate:
      if (getActiveGLViewer())
        getActiveGLViewer()->UpdateScene();
      break;

    case kSceneUpdateAll:
      for (int i = 0; i < 3; i++)
        m_glViewer[i]->UpdateScene();
      break;

    case kSaveGeometryExtract:
      saveExtract();
      break;

    default:
      break;
  }
}

void SplitGLView::onClicked(TObject* obj)
{
  // Handle click events in GL viewer

  if (obj) {
    m_statusBar->SetText(Form("User clicked on: \"%s\"", obj->GetName()), 1);

    TEveElement* elem = dynamic_cast<TEveElement*>(obj);
    if (!elem)
      return;

    TGListTreeItem* eventItem = gEve->GetListTree()->FindItemByPathname("Event");
    TGListTreeItem* item = gEve->GetListTree()->FindItemByObj(eventItem, elem);
    if (item) {
      //open all parent nodes
      TGListTreeItem* parent = item;
      while ((parent = parent->GetParent()) != nullptr)
        gEve->GetListTree()->OpenItem(parent);
    }
  } else {
    m_statusBar->SetText("", 1);
  }


  // change the active GL viewer to the one who emitted the signal
  TGLEmbeddedViewer* sender = dynamic_cast<TGLEmbeddedViewer*>(static_cast<TQObject*>(gTQSender));

  if (!sender) {
    B2WARNING("onClicked() signal not from a TGLEmbeddedViewer?");
    return;
  }
  setActiveViewer(sender);
}

TGLEmbeddedViewer* SplitGLView::getActiveGLViewer()
{
  const TEveWindow* currentWindow = gEve->GetWindowManager()->GetCurrentWindow();
  if (m_activeViewer < 0 or m_window[m_activeViewer] != currentWindow) {
    //check if some other viewer was selected
    for (int i = 0; i < 3; i++) {
      if (m_window[i] == currentWindow) {
        m_activeViewer = i;
      }
    }
  }
  if (m_activeViewer < 0)
    return NULL;

  return m_glViewer[m_activeViewer];
}

void SplitGLView::setActiveViewer(TGLEmbeddedViewer* v)
{
  bool found = false;
  for (int i = 0; i < 3; i++) {
    if (m_glViewer[i] == v) {
      m_activeViewer = i;
      found = true;
    }
  }
  if (!found) {
    B2WARNING("setActiveViewer(): viewer not found!");
    m_activeViewer = -1;
  }
  if (m_activeViewer >= 0) {
    //activate corresponding window
    m_window[m_activeViewer]->MakeCurrent();

    // update menu entries to match active viewer's options
    if (getActiveGLViewer()->GetOrthoXOYCamera()->GetDollyToZoom() &&
        getActiveGLViewer()->GetOrthoXOZCamera()->GetDollyToZoom() &&
        getActiveGLViewer()->GetOrthoZOYCamera()->GetDollyToZoom())
      m_cameraMenu->UnCheckEntry(kGLOrthoDolly);
    else
      m_cameraMenu->CheckEntry(kGLOrthoDolly);

    if (getActiveGLViewer()->GetOrthoXOYCamera()->GetEnableRotate() &&
        getActiveGLViewer()->GetOrthoXOZCamera()->GetEnableRotate() &&
        getActiveGLViewer()->GetOrthoZOYCamera()->GetEnableRotate())
      m_cameraMenu->CheckEntry(kGLOrthoRotate);
    else
      m_cameraMenu->UnCheckEntry(kGLOrthoRotate);
  }
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
  if (getActiveGLViewer()) {
    getActiveGLViewer()->GetOrthoXOYCamera()->SetEnableRotate(state);
    getActiveGLViewer()->GetOrthoXOZCamera()->SetEnableRotate(state);
    getActiveGLViewer()->GetOrthoZOYCamera()->SetEnableRotate(state);
  }
}

void SplitGLView::toggleOrthoDolly()
{
  // Toggle state of the 'Ortho allow dolly' menu entry.

  if (m_cameraMenu->IsEntryChecked(kGLOrthoDolly))
    m_cameraMenu->UnCheckEntry(kGLOrthoDolly);
  else
    m_cameraMenu->CheckEntry(kGLOrthoDolly);
  Bool_t state = !m_cameraMenu->IsEntryChecked(kGLOrthoDolly);
  if (getActiveGLViewer()) {
    getActiveGLViewer()->GetOrthoXOYCamera()->SetDollyToZoom(state);
    getActiveGLViewer()->GetOrthoXOZCamera()->SetDollyToZoom(state);
    getActiveGLViewer()->GetOrthoZOYCamera()->SetDollyToZoom(state);
  }
}

void SplitGLView::itemClicked(TGListTreeItem* item, Int_t, Int_t, Int_t)
{
  // Item has been clicked, based on mouse button do:

  static const TEveException eh("SplitGLView::itemClicked ");
  TEveElement* re = static_cast<TEveElement*>(item->GetUserData());
  if (re == 0) return;
  TObject* obj = re->GetObject(eh);
  if (obj->InheritsFrom("TEveViewer")) {
    TGLViewer* v = static_cast<TEveViewer*>(obj)->GetGLViewer();
    //v->Activated();
    if (v->InheritsFrom("TGLEmbeddedViewer")) {
      TGLEmbeddedViewer* ev = (TGLEmbeddedViewer*)v;
      gVirtualX->SetInputFocus(ev->GetGLWidget()->GetId());
    }
  }
}
