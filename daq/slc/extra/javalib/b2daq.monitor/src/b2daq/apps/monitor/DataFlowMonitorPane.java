/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.monitor;

import b2daq.dqm.core.TimedGraph1D;
import b2daq.dqm.graphics.HistogramCanvas;
import java.io.IOException;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.control.Label;
import javafx.scene.layout.VBox;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class DataFlowMonitorPane extends VBox {

    @FXML
    private HistogramCanvas c_rate;
    @FXML
    private HistogramCanvas c_size;
    @FXML
    private TimedGraph1D gr_rate_in;
    @FXML
    private TimedGraph1D gr_rate_out;
    @FXML
    private TimedGraph1D gr_size_in;
    @FXML
    private TimedGraph1D gr_size_out;
    @FXML
    private Label label_nodename;

    public DataFlowMonitorPane() {
        FXMLLoader fxmlLoader = new FXMLLoader(getClass().getResource("DataFlowMonitorPane.fxml"));
        fxmlLoader.setRoot(this);
        fxmlLoader.setController(this);
        try {
            fxmlLoader.load();
        } catch (IOException exception) {
            throw new RuntimeException(exception);
        }
    }

    public void update(double evtrate_in, double evtsize_in, double evtrate_out, double evtsize_out) {
        gr_rate_in.addPoint(evtrate_in);
        gr_size_in.addPoint(evtsize_in);
        gr_rate_out.addPoint(evtrate_out);
        gr_size_out.addPoint(evtsize_out);
        c_rate.update();
        c_size.update();
    }

    void setNodeName(String nodename) {
        this.label_nodename.setText(nodename);
    }

    String getNodeName() {
        return label_nodename.getText();
    }

}
