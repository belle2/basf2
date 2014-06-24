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
import java.util.ResourceBundle;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Tab;
import javafx.scene.control.TabPane;
import javafx.scene.layout.HBox;
import javafx.scene.layout.Priority;
import javafx.scene.layout.VBox;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class DQMMainPaneController implements Initializable {

    @FXML
    private TabPane tabpane;

    private ArrayList<HistoPackage> _pack_v = null;
    private ArrayList<CanvasPanel> _canvas_v = null;
    private DQMSidePaneController _side_panel;

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
        _pack_v = new ArrayList<>();
        _canvas_v = new ArrayList<>();
    }

    public ArrayList<HistoPackage> getPackages() {
        return _pack_v;
    }

    public void initPanels(ArrayList<HistoPackage> pack_v,
            DQMSidePaneController side_panel) {
        //removeAll();
        _pack_v = pack_v;
        _side_panel = side_panel;
        _side_panel.setMainPanel(this);
        for (HistoPackage pack : _pack_v) {
            TabPane pane = new TabPane();
            Tab tab = new Tab();
            tab.setClosable(false);
            tab.setText(pack.getName());
            tab.setContent(pane);
            VBox.setVgrow(pane, Priority.ALWAYS);
            HBox.setHgrow(pane, Priority.ALWAYS);
            tabpane.getTabs().add(tab);
            ArrayList<Histo> histo_v = new ArrayList<>();
            for (int i = 0; i < pack.getNHistos(); i++) {
                Histo h = (Histo) pack.getHisto(i);
                System.out.println(h.getName());
                CanvasPanel canvas = new CanvasPanel(h.getName(), h.getTitle());
                canvas.getCanvas().setStat(true);
                canvas.setMinSize(400, 400);
                canvas.getCanvas().addHisto(h);
                canvas.getCanvas().resetPadding();
                VBox.setVgrow(canvas, Priority.ALWAYS);
                HBox.setHgrow(canvas, Priority.ALWAYS);
                _canvas_v.add(canvas);
                TabPane subpane = new TabPane();
                VBox.setVgrow(subpane, Priority.ALWAYS);
                HBox.setHgrow(subpane, Priority.ALWAYS);
                Tab subtab = new Tab();
                subtab.setClosable(false);
                //subtab.setText(h.getName());
                subtab.setText(h.getTitle());
                pane.getTabs().add(subtab);
                subtab.setContent(subpane);
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
                _canvas_v.add(canvas);
                subsubtab = new Tab();
                subtab.setClosable(false);
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
                pack.addHisto(h);
            }
        }
    }

    public void update() {
        for (CanvasPanel canvas : _canvas_v) {
            canvas.update();
        }
    }

    public TabPane getPane() {
        return tabpane;
    }

    public void setPane(TabPane pane) {
        tabpane = pane;
    }

}
