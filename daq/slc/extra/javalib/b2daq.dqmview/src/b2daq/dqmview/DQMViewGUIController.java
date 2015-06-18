/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.dqmview;

import b2daq.dqm.core.HistoPackage;
import b2daq.dqm.io.DQMObserver;
import b2daq.dqm.io.PackageInfo;
import b2daq.hvcontrol.ui.HVStateLabel;
import java.net.URL;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.ResourceBundle;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Accordion;
import javafx.scene.control.Label;
import javafx.scene.layout.StackPane;
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
    private Label label_update;
    @FXML
    private Label label_directory;
    //@FXML
    //private HVStateLabel slabel;
    @FXML
    private StackPane tabpane;

    private final DQMMainPaneController _main_panel = new DQMMainPaneController(this);
    private final DQMSidePaneController _side_panel = new DQMSidePaneController(this);

    @Override
    public void initialize(URL url, ResourceBundle rb) {
        _main_panel.setPane(tabpane);
        _side_panel.setPane(accordion);
    }

    public void init(ArrayList<HistoPackage> pack_v,
            ArrayList<PackageInfo> info_v) {
        _main_panel.initPanels(pack_v, _side_panel);
        _side_panel.init(pack_v);
        //slabel.set("RUNNING", Color.CYAN, Color.CYAN, Color.BLACK);
        _main_panel.getPane().setDisable(false);
        _side_panel.getPane().setDisable(false);
    }

    @Override
    public void reset() {
        _main_panel.getPane().getChildren().clear();
        _side_panel.getPane().getPanes().clear();
    }

    @Override
    public void update(int expno, int runno, int stateno) {
        label_runno.setText(String.format("%04d.%04d", expno, runno));
        final SimpleDateFormat dateformat = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss");
        Date date = new Date();
        label_update.setText(dateformat.format(date));
        //slabel.set("RUNNING", Color.CYAN, Color.CYAN, Color.BLACK);
        _main_panel.update();
        _side_panel.update();
    }

    public void SetDirectory(String dir) {
        label_directory.setText(dir);
    }
            
    
}
