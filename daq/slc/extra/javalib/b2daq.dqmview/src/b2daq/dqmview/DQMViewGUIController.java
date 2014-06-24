/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.dqmview;

import b2daq.dqm.core.HistoPackage;
import b2daq.dqm.io.DQMObserver;
import b2daq.dqm.io.PackageInfo;
import b2daq.hvcontrol.ui.StateLabel;
import java.net.URL;
import java.util.ArrayList;
import java.util.ResourceBundle;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Accordion;
import javafx.scene.control.Label;
import javafx.scene.control.TabPane;
import javafx.scene.paint.Color;

/**
 *
 * @author tkonno
 */
public class DQMViewGUIController implements Initializable, DQMObserver {

    @FXML
    private Accordion accordion;
    
    @FXML
    private Label label_runno;
    @FXML
    private StateLabel slabel;
    
    @FXML
    private TabPane tabpane;

    private final DQMMainPaneController _main_panel = new DQMMainPaneController();
    private final DQMSidePaneController _side_panel = new DQMSidePaneController();

    @Override
    public void initialize(URL url, ResourceBundle rb) {
        _main_panel.setPane(tabpane);
        _side_panel.setPane(accordion);
    }

    public void init(ArrayList<HistoPackage> pack_v,
            ArrayList<PackageInfo> info_v) {
        _main_panel.initPanels(pack_v, _side_panel);
        _side_panel.init(pack_v);
        slabel.set("RUNNING", Color.CYAN, Color.CYAN, Color.BLACK);
        _main_panel.getPane().setDisable(false);
        _side_panel.getPane().setDisable(false);
    }

    public void update() {
        _main_panel.update();
        _side_panel.update();
    }

    @Override
    public void reset() {
        _main_panel.getPane().getTabs().clear();
        _side_panel.getPane().getPanes().clear();
    }

    @Override
    public void update(int expno, int runno, int stateno) {

    }

}
