/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.dqmview;

import b2daq.dqm.core.Histo;
import b2daq.dqm.core.HistoPackage;
import b2daq.dqm.ui.CanvasPanel;
import java.net.URL;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.ResourceBundle;
import javafx.fxml.Initializable;
import javafx.scene.control.Tab;
import javafx.scene.control.TabPane;
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

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        // TODO
    }

    public DQMMainPaneController() {
        super();
        m_pack_v = new ArrayList<>();
        m_canvas_v = new ArrayList<>();
    }

    public ArrayList<HistoPackage> getPackages() {
        return m_pack_v;
    }

    public void initPanels(ArrayList<HistoPackage> pack_v,
            DQMSidePaneController side_panel) {
        //removeAll();
        m_pack_v = pack_v;
        m_sidepanel = side_panel;
        m_sidepanel.setMainPanel(this);
        for (HistoPackage pack : m_pack_v) {
            HashMap<String, StackPane> pane_v = new HashMap<>();
            for (int i = 0; i < pack.getNHistos(); i++) {
                String dir = pack.getDirectory(pack.getHisto(i));
                if (!pane_v.containsKey(dir)) {
                    StackPane pane = new StackPane();
                    /*
                    Tab tab = new Tab();
                    tab.setClosable(false);
                    tab.setText(dir);
                    tab.setContent(pane);
                    */
                    VBox.setVgrow(pane, Priority.ALWAYS);
                    HBox.setHgrow(pane, Priority.ALWAYS);
                    tabpane.getChildren().add(pane);
                    pane_v.put(dir, pane);
                }
            }
            ArrayList<Histo> histo_v = new ArrayList<>();
            for (int i = 0; i < pack.getNHistos(); i++) {
                Histo h = (Histo) pack.getHisto(i);
                String dir = pack.getDirectory(pack.getHisto(i));
                System.out.println(dir + "/" + h.getName());
                StackPane pane = pane_v.get(dir);
                CanvasPanel canvas = new CanvasPanel(h.getName(), h.getTitle());
                canvas.getCanvas().setStat(true);
                canvas.setMinSize(400, 400);
                canvas.getCanvas().addHisto(h);
                canvas.getCanvas().resetPadding();
                VBox.setVgrow(canvas, Priority.ALWAYS);
                HBox.setHgrow(canvas, Priority.ALWAYS);
                m_canvas_v.add(canvas);
                TabPane subpane = new TabPane();
                VBox.setVgrow(subpane, Priority.ALWAYS);
                HBox.setHgrow(subpane, Priority.ALWAYS);
                /*
                Tab subtab = new Tab();
                subtab.setClosable(false);
                subtab.setText(h.getTitle());
                pane.getTabs().add(subtab);
                subtab.setContent(subpane);
                */
                pane.getChildren().add(subpane);
                Tab subsubtab = new Tab();
                subsubtab.setText("Total");
                subsubtab.setContent(canvas);
                subsubtab.setClosable(false);
                subpane.getTabs().add(subsubtab);
                Histo h1 = (Histo) h.clone();
                h1.setName(h.getName() + ":diff");
                canvas = new CanvasPanel(h1.getName(), h1.getTitle());
                canvas.getCanvas().setStat(true);
                canvas.getCanvas().addHisto(h1);
                canvas.getCanvas().resetPadding();
                VBox.setVgrow(canvas, Priority.ALWAYS);
                HBox.setHgrow(canvas, Priority.ALWAYS);
                m_canvas_v.add(canvas);
                subsubtab = new Tab();
                subsubtab.setText("Difference");
                subsubtab.setContent(canvas);
                subsubtab.setClosable(false);
                histo_v.add(h1);
                h1 = (Histo) h.clone();
                h1.setName(h.getName() + ":tmp");
                subpane.getTabs().add(subsubtab);
                histo_v.add(h1);
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

}
