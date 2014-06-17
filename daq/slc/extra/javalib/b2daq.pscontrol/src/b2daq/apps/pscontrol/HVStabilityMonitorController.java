/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.pscontrol;

import b2daq.dqm.core.TimedGraph1D;
import b2daq.dqm.graphics.HistogramCanvas;
import java.net.URL;
import java.util.ResourceBundle;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.layout.VBox;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class HVStabilityMonitorController implements Initializable {

    @FXML
    private VBox pane;
    @FXML
    private HistogramCanvas c_vol_gr;
    @FXML
    private TimedGraph1D gr_vol_ch0;
    @FXML
    private TimedGraph1D gr_vol_ch1;
    @FXML
    private TimedGraph1D gr_vol_ch2;
    @FXML
    private TimedGraph1D gr_vol_ch3;
    @FXML
    private TimedGraph1D gr_vol_ch4;
    @FXML
    private TimedGraph1D gr_vol_ch5;
    @FXML
    private TimedGraph1D gr_vol_ch6;
    @FXML
    private TimedGraph1D gr_vol_ch7;
    @FXML
    private TimedGraph1D gr_vol_ch8;

    private TimedGraph1D gr_vol_node0[];

    @FXML
    private HistogramCanvas c_cur_gr;
    @FXML
    private TimedGraph1D gr_cur_ch0;
    @FXML
    private TimedGraph1D gr_cur_ch1;
    @FXML
    private TimedGraph1D gr_cur_ch2;
    @FXML
    private TimedGraph1D gr_cur_ch3;
    @FXML
    private TimedGraph1D gr_cur_ch4;
    @FXML
    private TimedGraph1D gr_cur_ch5;
    @FXML
    private TimedGraph1D gr_cur_ch6;
    @FXML
    private TimedGraph1D gr_cur_ch7;
    @FXML
    private TimedGraph1D gr_cur_ch8;

    private TimedGraph1D gr_cur_node0[];

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        gr_vol_node0 = new TimedGraph1D[]{
            gr_vol_ch0, gr_vol_ch1, gr_vol_ch2,
            gr_vol_ch3, gr_vol_ch4, gr_vol_ch5,
            gr_vol_ch6, gr_vol_ch7, gr_vol_ch8,};
        gr_cur_node0 = new TimedGraph1D[]{
            gr_cur_ch0, gr_cur_ch1, gr_cur_ch2,
            gr_cur_ch3, gr_cur_ch4, gr_cur_ch5,
            gr_cur_ch6, gr_cur_ch7, gr_cur_ch8,};
    }

    public void update(int i, double voltage, double current) {
        if (i < gr_vol_node0.length) {
            gr_vol_node0[i].addPoint(voltage);
        }
        if (i < gr_cur_node0.length) {
            gr_cur_node0[i].addPoint(current);
        }
    }

    public VBox getPane() {
        return pane;
    }

    void update() {
        c_vol_gr.update();
        c_cur_gr.update();
    }

}
