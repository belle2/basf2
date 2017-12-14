/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.dqmview;

import b2daq.dqm.core.Histo;
import b2daq.dqm.core.HistoPackage;
import b2daq.dqm.ui.CanvasPanel;
import java.io.File;
import java.net.URL;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.ResourceBundle;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.fxml.FXMLLoader;
import javafx.fxml.Initializable;
import javafx.scene.Node;
import javafx.scene.Parent;
import javafx.scene.control.ScrollPane;
import javafx.scene.control.Tab;
import javafx.scene.control.TabPane;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.HBox;
import javafx.scene.layout.Priority;
import javafx.scene.layout.StackPane;
import javafx.scene.layout.VBox;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class DQMMainPaneController implements Initializable {

    private StackPane tabpane;
    private ArrayList<HistoPackage> m_pack_v = null;
    private ArrayList<CanvasPanel> m_canvas_v = null;
    private DQMSidePaneController m_sidepanel;

    static public final String getCSSPath() {
        return DQMMainPaneController.class.getResource("DQMMainPane.css").toExternalForm();
    }
    private final DQMViewGUIController m_ctl;

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        // TODO
    }

    public DQMMainPaneController(DQMViewGUIController ctl) {
        super();
        m_ctl = ctl;
        m_pack_v = new ArrayList<>();
        m_canvas_v = new ArrayList<>();
    }

    public ArrayList<HistoPackage> getPackages() {
        return m_pack_v;
    }

    public void initPanels(ArrayList<HistoPackage> pack_v,
            DQMSidePaneController side_panel) {
        m_pack_v = pack_v;
        m_sidepanel = side_panel;
        m_sidepanel.setMainPanel(this);
        final String filedir = System.getProperty("user.home") + "/.dqmview";
        for (HistoPackage pack : m_pack_v) {
            HashMap<String, Parent> pane_v = new HashMap<>();
            HashMap<String, Boolean> isdefault_v = new HashMap<>();
            for (int i = 0; i < pack.getNHistos(); i++) {
                String dir = pack.getDirectory(pack.getHisto(i));
                if (!pane_v.containsKey(dir)) {
                    String filepath = filedir + "/" + dir + ".fxml";
                    if (new File(filepath).exists()) {
                        try {
                            FXMLLoader loader = new FXMLLoader(new URL("file:"+filepath));
                            loader.load();
                            Parent pane = loader.getRoot();
                            tabpane.getChildren().add(pane);
                            pane_v.put(dir, pane);
                            isdefault_v.put(dir, false);
                        } catch (Exception ex) {
                            Logger.getLogger(DQMMainPaneController.class.getName()).log(Level.SEVERE, null, ex);
                            StackPane pane = new StackPane();
                            VBox.setVgrow(pane, Priority.ALWAYS);
                            HBox.setHgrow(pane, Priority.ALWAYS);
                            tabpane.getChildren().add(pane);
                            pane_v.put(dir, pane);
                            isdefault_v.put(dir, true);
                        }
                    } else {
                        StackPane pane = new StackPane();
                        System.out.println("No file exists : " + filepath);
                        VBox.setVgrow(pane, Priority.ALWAYS);
                        HBox.setHgrow(pane, Priority.ALWAYS);
                        tabpane.getChildren().add(pane);
                        pane_v.put(dir, pane);
                        isdefault_v.put(dir, true);
                    }
                }
            }
            ArrayList<Histo> histo_v = new ArrayList<>();
            for (int i = 0; i < pack.getNHistos(); i++) {
                Histo h = (Histo) pack.getHisto(i);
                Histo h_diff = addClone(histo_v, h, ":diff");
                addClone(histo_v, h, ":tmp");
                String dir = pack.getDirectory(pack.getHisto(i));
                System.out.println(dir + "/" + h.getName());
                if (!isdefault_v.get(dir)) {
                    if (findCanvas(h.getName(), h, pane_v.get(dir)) == null) {
                        System.out.println("No canvas found for "+ h.getName());
                    }
                    if (findCanvas(h_diff.getName(), h_diff, pane_v.get(dir)) == null) {
                        System.out.println("No canvas found for "+ h.getName());
                    }
                } else {
                    StackPane pane = (StackPane)pane_v.get(dir);
                    Parent subpane = createDefaultPane(h, h_diff);
                    pane.getChildren().add(subpane);
                }
            }
            for (Histo h : histo_v) {
                pack.addHisto("", h);
            }
        }
    }

    public void update() {
        for (CanvasPanel canvas : m_canvas_v) {
            canvas.update();
        }
    }

    public StackPane getPane() {
        return tabpane;
    }

    public void setPane(StackPane pane) {
        tabpane = pane;
    }

    private Histo addClone(ArrayList<Histo> histo_v, Histo h, String newname) {
        Histo h_clone = (Histo) h.clone();
        h_clone.reset();
        h_clone.setName(h.getName() + newname);
        histo_v.add(h_clone);
        return h_clone;
    }

    private Parent createDefaultPane(Histo h, Histo h_diff) {
        CanvasPanel canvas = new CanvasPanel("c_"+h.getName(), h.getTitle());
        canvas.getCanvas().setStat(true);
        canvas.getCanvas().addHisto(h);
        canvas.getCanvas().resetPadding();
        VBox.setVgrow(canvas, Priority.ALWAYS);
        HBox.setHgrow(canvas, Priority.ALWAYS);
        m_canvas_v.add(canvas);
        TabPane subpane = new TabPane();
        VBox.setVgrow(subpane, Priority.ALWAYS);
        HBox.setHgrow(subpane, Priority.ALWAYS);
        Tab subsubtab = new Tab();
        subsubtab.setText("Integrated");
        subsubtab.setContent(canvas);
        subsubtab.setClosable(false);
        subpane.getTabs().add(subsubtab);
        canvas = new CanvasPanel("c_"+h_diff.getName(), h_diff.getTitle());
        canvas.getCanvas().setStat(true);
        canvas.getCanvas().addHisto(h_diff);
        canvas.getCanvas().resetPadding();
        VBox.setVgrow(canvas, Priority.ALWAYS);
        HBox.setHgrow(canvas, Priority.ALWAYS);
        m_canvas_v.add(canvas);
        subsubtab = new Tab();
        subsubtab.setText("Differential");
        subsubtab.setContent(canvas);
        subsubtab.setClosable(false);
        subpane.getTabs().add(subsubtab);
        return subpane;
    }

    private CanvasPanel findCanvas(String name, Histo h, Node parent) {
        CanvasPanel canvasnode;
        if (parent instanceof TabPane) {
            TabPane tpane = (TabPane) parent;
            for (Tab tab : tpane.getTabs()) {
                if ((canvasnode = findCanvas(name, h, tab.getContent())) != null) {
                    tpane.getSelectionModel().select(tab);
                    return canvasnode;
                }
            }
        } else if (parent instanceof StackPane) {
            StackPane spane = (StackPane) parent;
            for (Node node : spane.getChildren()) {
                if ((canvasnode = findCanvas(name, h, node)) != null) {
                    return canvasnode;
                }
            }
        } else if (parent instanceof VBox) {
            VBox vpane = (VBox) parent;
            for (Node node : vpane.getChildren()) {
                if ((canvasnode = findCanvas(name, h, node)) != null) {
                    return canvasnode;
                }
            }
        } else if (parent instanceof HBox) {
            HBox hpane = (HBox) parent;
            for (Node node : hpane.getChildren()) {
                if ((canvasnode = findCanvas(name, h, node)) != null) {
                    return canvasnode;
                }
            }
        } else if (parent instanceof GridPane) {
            GridPane gpane = (GridPane) parent;
            for (Node node : gpane.getChildren()) {
                if ((canvasnode = findCanvas(name, h, node)) != null) {
                    return canvasnode;
                }
            }
        } else if (parent instanceof ScrollPane) {
            ScrollPane spane = (ScrollPane) parent;
            if ((canvasnode = findCanvas(name, h, spane.getContent())) != null) {
                return canvasnode;
            }
        } else if (parent instanceof CanvasPanel) {
            CanvasPanel canvas = (CanvasPanel) parent;
            for (Histo hist : canvas.getCanvas().getHistograms()) {
                if (hist.getName().equals(name)) {
                    System.out.println("found " + name);
                    canvas.addHisto(h);
                    canvas.getCanvas().setName("c_"+h.getName());
                    canvas.getCanvas().setTitle(h.getTitle());
                    m_canvas_v.add(canvas);
                    return canvas;
                }
            }
        } else if (parent != null) {
            System.out.println(parent.getClass().getName());
        }
        return null;
    }
}
