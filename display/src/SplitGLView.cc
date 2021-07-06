/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <display/SplitGLView.h>
#include <display/EveGeometry.h>
#include <display/InfoWidget.h>
#include <framework/logging/Logger.h>

#include "TEveScene.h"
#include "TEveManager.h"
#include "TEveBrowser.h"
#include "TEveProjectionAxes.h"
#include "TEveViewer.h"
#include "TEveWindowManager.h"
#include "TGMenu.h"
#include "TGStatusBar.h"
#include "TGTab.h"
#include "TGLPhysicalShape.h"
#include "TGLLogicalShape.h"
#include "TGLWidget.h"
#include "TVirtualX.h"

#include "TString.h"

using namespace Belle2;

namespace {
  /** toggles menu entry with given id, returns whether it is checked in new state. */
  bool toggleMenuEntry(TGPopupMenu* menu, int entryid)
  {
    if (menu->IsEntryChecked(entryid))
      menu->UnCheckEntry(entryid);
    else
      menu->CheckEntry(entryid);
    return menu->IsEntryChecked(entryid);
  }
}

SplitGLView::SplitGLView() :
  m_activeViewer(-1),
  m_infoWidget(nullptr)
{
  const char* projectionName[3] = { "3D", "Rho/Z", "R/Phi" };
  TGLViewer::ECameraType cameraType[3] = { TGLViewer::kCameraPerspXOZ, TGLViewer::kCameraOrthoXOY, TGLViewer::kCameraOrthoXOY };

  TEveWindowSlot* slot = TEveWindow::CreateWindowMainFrame();
  TEveWindowPack* pack = slot->MakePack();
  pack->SetVertical();
  pack->SetShowTitleBar(kFALSE);

  TEveWindowSlot* viewerslots[3];
  auto pack1 = pack->NewSlot()->MakePack();
  pack1->SetHorizontal();
  pack1->SetShowTitleBar(kFALSE);
  viewerslots[0] = pack1->NewSlotWithWeight(0.7);
  TEveWindowSlot* infoslot = pack1->NewSlotWithWeight(0.3);

  auto pack2 = pack->NewSlot()->MakePack();
  pack2->SetHorizontal();
  pack2->SetShowTitleBar(kFALSE);
  viewerslots[1] = pack2->NewSlot();
  viewerslots[2] = pack2->NewSlot();


  //0: 3d, 1: rhoz, 2: rphi
  for (int iFrame = 0; iFrame < 3; iFrame++) {
    TEveViewer* viewer = new TEveViewer(TString::Format("%s viewer", projectionName[iFrame]));
    m_glViewer[iFrame] = viewer->SpawnGLEmbeddedViewer();
    viewerslots[iFrame]->ReplaceWindow(viewer);
    viewerslots[iFrame] = nullptr; // invalid after ReplaceWindow()

    viewer->SetShowTitleBar(kFALSE); //might want to show these?
    m_window[iFrame] = viewer;

    m_glViewer[iFrame]->SetCurrentCamera(cameraType[iFrame]);
    //create projection managers
    TEveProjectionManager* projectionMgr = 0;
    if (iFrame == 1) {
      m_rhozManager = new TEveProjectionManager(TEveProjection::kPT_RhoZ);
      projectionMgr = m_rhozManager;
      m_glViewer[iFrame]->CurrentCamera().Zoom(+300, false, false);
    } else if (iFrame == 2) {
      m_rphiManager = new TEveProjectionManager(TEveProjection::kPT_RPhi);
      projectionMgr = m_rphiManager;
      m_glViewer[iFrame]->CurrentCamera().Zoom(-50, false, false);
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
      TEveProjectionAxes* axes = new TEveProjectionAxes(projectionMgr);
      projectionMgr->AddElement(axes);
    }
  }

  m_infoWidget = new InfoWidget(gClient->GetRoot());

  TEveWindowFrame* eveFrame = new TEveWindowFrame(m_infoWidget, "DataStore Info");
  infoslot->ReplaceWindow(eveFrame);
  eveFrame->SetShowTitleBar(kFALSE);
  infoslot = nullptr; // invalid after ReplaceWindow()


  // create the "camera" popup menu
  m_cameraMenu = new TGPopupMenu(gClient->GetDefaultRoot());
  m_cameraMenu->AddEntry("Perspective (Floor X/Z)", kGLPerspXOZ);
  m_cameraMenu->AddEntry("Perspective (Floor Y/Z)", kGLPerspYOZ);
  m_cameraMenu->AddEntry("Perspective (Floor X/Y)", kGLPerspXOY);
  m_cameraMenu->AddEntry("Orthographic (X/Y)", kGLXOY);
  m_cameraMenu->AddEntry("Orthographic (X/Z)", kGLXOZ);
  m_cameraMenu->AddEntry("Orthographic (Z/Y)", kGLZOY);
  m_cameraMenu->AddSeparator();
  m_cameraMenu->AddEntry("Orthographic: Allow Rotating", kGLOrthoRotate);
  m_cameraMenu->AddEntry("Orthographic: Right Click to Dolly", kGLOrthoDolly);
  m_cameraMenu->AddSeparator();
  m_cameraMenu->AddEntry("Perspective: Enable Stereographic 3D (Requires Hardware or stereowrap)", kGLStereo);

  m_sceneMenu = new TGPopupMenu(gClient->GetDefaultRoot());
  m_sceneMenu->AddEntry("&Update Current", kSceneUpdate);
  m_sceneMenu->AddEntry("Update &All", kSceneUpdateAll);
  m_sceneMenu->AddSeparator();
  m_sceneMenu->AddEntry("&Show Scale for Projections", kShowScale);
  m_sceneMenu->CheckEntry(kShowScale);
  m_sceneMenu->AddSeparator();
  m_sceneMenu->AddEntry("Save &Geometry Extract", kSaveGeometryExtract);

  TGMenuBar* menuBar = gEve->GetBrowser()->GetMenuBar();
  menuBar->AddPopup("&Camera", m_cameraMenu, new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0));
  menuBar->AddPopup("&Scene", m_sceneMenu, new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0));

  // connect menu signals to our menu handler slot
  m_cameraMenu->Connect("Activated(Int_t)", "Belle2::SplitGLView", this, "handleMenu(Int_t)");
  m_sceneMenu->Connect("Activated(Int_t)", "Belle2::SplitGLView", this, "handleMenu(Int_t)");

  // use status bar from the browser
  m_statusBar = gEve->GetBrowser()->GetStatusBar();

  //needs to come after menu
  setActiveViewer(m_glViewer[0]);

  gEve->GetListTree()->Connect("Clicked(TGListTreeItem*, Int_t, Int_t, Int_t)",
                               "Belle2::SplitGLView", this, "itemClicked(TGListTreeItem*, Int_t, Int_t, Int_t)");
}

SplitGLView::~SplitGLView()
{
  // Clean up main frame...
  for (int i = 0; i < 3; i++) {
    delete m_glViewer[i];
  }
  delete m_cameraMenu;
  delete m_sceneMenu;
  delete m_infoWidget;
}

void SplitGLView::updateCamera(int cameraAction)
{
  TGLEmbeddedViewer* viewer = getActiveGLViewer();
  if (!viewer)
    return;

  TGLViewer::ECameraType cameraType;
  switch (cameraAction) {
    case kGLPerspYOZ:
      cameraType = TGLViewer::kCameraPerspYOZ;
      break;
    case kGLPerspXOZ:
      cameraType = TGLViewer::kCameraPerspXOZ;
      break;
    case kGLPerspXOY:
      cameraType = TGLViewer::kCameraPerspXOY;
      break;
    case kGLXOY:
      cameraType = TGLViewer::kCameraOrthoXOY;
      break;
    case kGLXOZ:
      cameraType = TGLViewer::kCameraOrthoXOZ;
      break;
    case kGLZOY:
      cameraType = TGLViewer::kCameraOrthoZOY;
      break;
    default:
      B2ERROR("unknown camera action " << cameraAction);
      return;
  }
  viewer->SetCurrentCamera(cameraType);
}

void SplitGLView::handleMenu(Int_t menuCommand)
{
  // Handle menu items.

  switch (menuCommand) {
    case kGLPerspYOZ:
    case kGLPerspXOZ:
    case kGLPerspXOY:
    case kGLXOY:
    case kGLXOZ:
    case kGLZOY:
      updateCamera(menuCommand);
      break;
    case kGLOrthoRotate:
      toggleOrthoRotate();
      break;
    case kGLOrthoDolly:
      toggleOrthoDolly();
      break;

    case kGLStereo:
      toggleStereo();
      break;

    case kSceneUpdate:
      if (getActiveGLViewer())
        getActiveGLViewer()->UpdateScene();
      break;

    case kSceneUpdateAll:
      for (int i = 0; i < 3; i++)
        m_glViewer[i]->UpdateScene();
      break;

    case kShowScale:
      toggleShowScale();
      break;

    case kSaveGeometryExtract:
      EveGeometry::saveExtract();
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
    if (TEveProjected* projected = dynamic_cast<TEveProjected*>(obj)) {
      elem = dynamic_cast<TEveElement*>(projected->GetProjectable());
    }
    if (!elem)
      return;

    TGListTreeItem* eventItem = gEve->GetListTree()->FindItemByPathname("Event");
    TGListTreeItem* item = gEve->GetListTree()->FindItemByObj(eventItem, elem);
    if (item) {
      //focus Eve tab
      gEve->GetBrowser()->GetTabLeft()->SetTab("Eve");

      //open all parent nodes
      TGListTreeItem* parent = item;
      while ((parent = parent->GetParent()) != nullptr)
        gEve->GetListTree()->OpenItem(parent);

      //scroll to clicked item
      gEve->GetListTree()->AdjustPosition(item);
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

    if (getActiveGLViewer()->GetStereo())
      m_cameraMenu->CheckEntry(kGLStereo);
    else
      m_cameraMenu->UnCheckEntry(kGLStereo);
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
  Bool_t state = toggleMenuEntry(m_cameraMenu, kGLOrthoRotate);
  if (getActiveGLViewer()) {
    getActiveGLViewer()->GetOrthoXOYCamera()->SetEnableRotate(state);
    getActiveGLViewer()->GetOrthoXOZCamera()->SetEnableRotate(state);
    getActiveGLViewer()->GetOrthoZOYCamera()->SetEnableRotate(state);
  }
}

void SplitGLView::toggleOrthoDolly()
{
  Bool_t state = !toggleMenuEntry(m_cameraMenu, kGLOrthoDolly);
  if (getActiveGLViewer()) {
    getActiveGLViewer()->GetOrthoXOYCamera()->SetDollyToZoom(state);
    getActiveGLViewer()->GetOrthoXOZCamera()->SetDollyToZoom(state);
    getActiveGLViewer()->GetOrthoZOYCamera()->SetDollyToZoom(state);
  }
}

void SplitGLView::toggleStereo()
{
  if (!getActiveGLViewer())
    return;

  getActiveGLViewer()->SetStereo(toggleMenuEntry(m_cameraMenu, kGLStereo));
}

void SplitGLView::toggleShowScale()
{
  bool state = toggleMenuEntry(m_sceneMenu, kShowScale);

  std::vector<TEveProjectionManager*> projections = {m_rhozManager, m_rphiManager};
  for (auto mgr : projections) {
    TEveElement::List_ci end_it = mgr->EndChildren();
    for (TEveElement::List_i it = mgr->BeginChildren(); it != end_it; ++it) {
      TEveProjectionAxes* a = dynamic_cast<TEveProjectionAxes*>(*it);
      if (a) {
        a->SetRnrSelf(state);
      }
    }
  }
  gEve->Redraw3D(false); //do not reset camera when redrawing
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
