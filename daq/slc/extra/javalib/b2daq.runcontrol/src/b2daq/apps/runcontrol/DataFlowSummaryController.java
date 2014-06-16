/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import b2daq.database.ConfigObject;
import b2daq.dqm.graphics.HistogramCanvas;
import b2daq.graphics.GCircle;
import b2daq.graphics.GLine;
import b2daq.graphics.GRect;
import b2daq.graphics.GShape;
import b2daq.graphics.GText;
import b2daq.logger.core.LogMessage;
import b2daq.nsm.NSMMessage;
import b2daq.nsm.NSMObserver;
import java.io.IOException;
import java.net.URL;
import java.util.ResourceBundle;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.fxml.Initializable;
import javafx.scene.layout.Pane;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;
import javafx.scene.text.FontWeight;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class DataFlowSummaryController implements Initializable, NSMObserver {

    @FXML
    private VBox pane;
    @FXML
    private HistogramCanvas c_map;
    private ConfigObject obj;

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        c_map.addCShape(new GRect(0.02, 0.02, 0.2, 0.1, Color.LIGHTGRAY, Color.BLACK));
        c_map.addCShape(new GText("CPR010", 0.12, 0.07, "center middle", Color.BLACK, 0.8, FontWeight.NORMAL));
        c_map.addCShape(new GLine(0.22, 0.07, 0.30, 0.16, Color.LIMEGREEN, 3));
        c_map.addCShape(new GCircle(0.22, 0.07, 0.008, 0.008, Color.LIMEGREEN, Color.TRANSPARENT));
        
        c_map.addCShape(new GRect(0.02, 0.22, 0.2, 0.1, Color.LIGHTGRAY, Color.BLACK));
        c_map.addCShape(new GText("CPR011", 0.12, 0.27, "center middle", Color.BLACK, 0.8, FontWeight.NORMAL));
        c_map.addCShape(new GLine(0.22, 0.27, 0.30, 0.18, Color.LIMEGREEN, 3));
        c_map.addCShape(new GCircle(0.22, 0.27, 0.008, 0.008, Color.LIMEGREEN, Color.TRANSPARENT));

        c_map.addCShape(new GRect(0.30, 0.12, 0.24, 0.1, Color.LIGHTGRAY, Color.BLACK));
        c_map.addCShape(new GText("EB0(ropc01)", 0.42, 0.17, "center middle", Color.BLACK, 0.8, FontWeight.NORMAL));
        c_map.addCShape(new GCircle(0.30, 0.16, 0.008, 0.008, Color.LIMEGREEN, Color.TRANSPARENT));
        c_map.addCShape(new GCircle(0.30, 0.18, 0.008, 0.008, Color.LIMEGREEN, Color.TRANSPARENT));
        c_map.addCShape(new GLine(0.54, 0.17, 0.61, 0.17, Color.LIMEGREEN, 3));
        c_map.addCShape(new GCircle(0.54, 0.17, 0.008, 0.008, Color.LIMEGREEN, Color.TRANSPARENT));

        c_map.addCShape(new GRect(0.61, 0.12, 0.2, 0.1, Color.LIGHTGRAY, Color.BLACK));
        c_map.addCShape(new GText("ROPC01", 0.71, 0.17, "center middle", Color.BLACK, 0.8, FontWeight.NORMAL));
        c_map.addCShape(new GCircle(0.61, 0.17, 0.008, 0.008, Color.LIMEGREEN, Color.TRANSPARENT));
        c_map.update();
    }

    @Override
    public void handleOnConnected() {
        //h1.setLineColor(Color.LIMEGREEN);
    }

    @Override
    public void handleOnReceived(NSMMessage msg) {
        c_map.update();
        if (msg == null) {
            return;
        }
    }

    @Override
    public void handleOnDisConnected() {
        //h1.setLineColor(Color.DEEPPINK);
    }

    @Override
    public void log(LogMessage log) {
    }

    void setDB(ConfigObject cobj) {
        this.obj = cobj;
    }

    void setNodeName(String nodename) {
    }

    String getNodeName() {
        return "";
    }

    static final DataFlowSummaryController create(String name) {
        try {
            FXMLLoader loader = new FXMLLoader(DataFlowSummaryController.class.getResource("DataFlowSummary.fxml"));
            loader.load();
            DataFlowSummaryController controller = loader.getController();
            controller.setNodeName(name);
            return controller;
        } catch (IOException e) {
            Logger.getLogger(DataFlowSummaryController.class.getName()).log(Level.SEVERE, null, e);
        }
        return null;
    }

    public Pane getPane() {
        return pane;
    }

    private void setColor(HistogramCanvas c, Color linecolor, Color fillcolor, boolean visible) {
        int i = 0;
        for (GShape s : c.getShapes()) {
            s.setLineColor(linecolor);
            s.setFillColor(fillcolor);
            if (i > 0) {
                s.setVisible(visible);
            }
            i++;
        }
    }

    private void setConnection(HistogramCanvas c, int port) {
        if (port < 0) {
            setColor(c, Color.RED, Color.TRANSPARENT, true);
        } else if (port == 0) {
            setColor(c, Color.LIGHTGRAY, Color.TRANSPARENT, false);
        } else if (port > 0) {
            setColor(c, Color.LIGHTGREEN, Color.LIMEGREEN, false);
        }

    }

}
