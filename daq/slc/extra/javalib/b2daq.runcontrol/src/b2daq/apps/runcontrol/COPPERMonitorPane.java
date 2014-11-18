/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import b2daq.database.ConfigObject;
import b2daq.dqm.core.TimedGraph1D;
import b2daq.dqm.graphics.HistogramCanvas;
import b2daq.graphics.GShape;
import b2daq.graphics.GText;
import b2daq.logger.core.LogMessage;
import b2daq.nsm.NSMCommand;
import b2daq.nsm.NSMData;
import b2daq.nsm.NSMListenerService;
import b2daq.nsm.NSMMessage;
import b2daq.nsm.NSMObserver;
import b2daq.runcontrol.core.RCState;
import b2daq.runcontrol.ui.RCStateLabel;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class COPPERMonitorPane extends VBox implements NSMObserver {

    @FXML
    private RCStateLabel rclabel;
    @FXML
    private HistogramCanvas canvasArrawFrom;
    @FXML
    private Label labelNodeName;
    @FXML
    private HistogramCanvas canvasArrawTo;
    @FXML
    private Label labelUpdateTime;
    @FXML
    private TextField fieldEvtRate;
    @FXML
    private Label labelEvtRateunit;
    @FXML
    private TextField fieldFlowRate;
    @FXML
    private Label labelFlowRateUnit;
    @FXML
    private HistogramCanvas canvasCprFIFO;
    @FXML
    private HistogramCanvas canvasLenFIFO;
    @FXML
    private HistogramCanvas canvasTTRXB2Link;
    @FXML
    private HistogramCanvas canvasTTRXLinkUp;
    @FXML
    private HistogramCanvas canvasHSLBaB2Link;
    @FXML
    private HistogramCanvas canvasHSLBaCprFIFO;
    @FXML
    private HistogramCanvas canvasHSLBaLenFIFO;
    @FXML
    private HistogramCanvas canvasHSLBaFIFO;
    @FXML
    private HistogramCanvas canvasHSLBaCRC;
    @FXML
    private HistogramCanvas canvasHSLBbB2Link;
    @FXML
    private HistogramCanvas canvasHSLBbCprFIFO;
    @FXML
    private HistogramCanvas canvasHSLBbLenFIFO;
    @FXML
    private HistogramCanvas canvasHSLBbFIFO;
    @FXML
    private HistogramCanvas canvasHSLBbCRC;
    @FXML
    private HistogramCanvas canvasHSLBcB2Link;
    @FXML
    private HistogramCanvas canvasHSLBcCprFIFO;
    @FXML
    private HistogramCanvas canvasHSLBcLenFIFO;
    @FXML
    private HistogramCanvas canvasHSLBcFIFO;
    @FXML
    private HistogramCanvas canvasHSLBcCRC;
    @FXML
    private HistogramCanvas canvasHSLBdB2Link;
    @FXML
    private HistogramCanvas canvasHSLBdCprFIFO;
    @FXML
    private HistogramCanvas canvasHSLBdLenFIFO;
    @FXML
    private HistogramCanvas canvasHSLBdFIFO;
    @FXML
    private HistogramCanvas canvasHSLBdCRC;
    @FXML
    private HistogramCanvas canvasEvtRate;
    @FXML
    private TimedGraph1D graphEvtRateIn;
    @FXML
    private TimedGraph1D graphEvtRateOut;
    @FXML
    private HistogramCanvas canvasFlowRate;
    @FXML
    private TimedGraph1D graphFlowRateIn;
    @FXML
    private TimedGraph1D graphFlowRateOut;

    public RCStateLabel rclabelProperty() {
        return rclabel;
    }

    public HistogramCanvas canvasArrawFromProperty() {
        return canvasArrawFrom;
    }

    public Label labelNodeNameProperty() {
        return labelNodeName;
    }

    public HistogramCanvas canvasArrawToProperty() {
        return canvasArrawTo;
    }

    public Label labelUpdateTimeProperty() {
        return labelUpdateTime;
    }

    public TextField fieldEvtRateProperty() {
        return fieldEvtRate;
    }

    public Label labelEvtRateunitProperty() {
        return labelEvtRateunit;
    }

    public TextField fieldFlowRateProperty() {
        return fieldFlowRate;
    }

    public Label labelFlowRateUnitProperty() {
        return labelFlowRateUnit;
    }

    public HistogramCanvas canvasCprFIFOProperty() {
        return canvasCprFIFO;
    }

    public HistogramCanvas canvasLenFIFOProperty() {
        return canvasLenFIFO;
    }

    public HistogramCanvas canvasTTRXB2LinkProperty() {
        return canvasTTRXB2Link;
    }

    public HistogramCanvas canvasTTRXLinkUpProperty() {
        return canvasTTRXLinkUp;
    }

    public HistogramCanvas canvasHSLBaB2LinkProperty() {
        return canvasHSLBaB2Link;
    }

    public HistogramCanvas canvasHSLBaCprFIFOProperty() {
        return canvasHSLBaCprFIFO;
    }

    public HistogramCanvas canvasHSLBaLenFIFOProperty() {
        return canvasHSLBaLenFIFO;
    }

    public HistogramCanvas canvasHSLBaFIFOProperty() {
        return canvasHSLBaFIFO;
    }

    public HistogramCanvas canvasHSLBaCRCProperty() {
        return canvasHSLBaCRC;
    }

    public HistogramCanvas canvasHSLBbB2LinkProperty() {
        return canvasHSLBbB2Link;
    }

    public HistogramCanvas canvasHSLBbCprFIFOProperty() {
        return canvasHSLBbCprFIFO;
    }

    public HistogramCanvas canvasHSLBbLenFIFOProperty() {
        return canvasHSLBbLenFIFO;
    }

    public HistogramCanvas canvasHSLBbFIFOProperty() {
        return canvasHSLBbFIFO;
    }

    public HistogramCanvas canvasHSLBbCRCProperty() {
        return canvasHSLBbCRC;
    }

    public HistogramCanvas canvasHSLBcB2LinkProperty() {
        return canvasHSLBcB2Link;
    }

    public HistogramCanvas canvasHSLBcCprFIFOProperty() {
        return canvasHSLBcCprFIFO;
    }

    public HistogramCanvas canvasHSLBcLenFIFOProperty() {
        return canvasHSLBcLenFIFO;
    }

    public HistogramCanvas canvasHSLBcFIFOProperty() {
        return canvasHSLBcFIFO;
    }

    public HistogramCanvas canvasHSLBcCRCProperty() {
        return canvasHSLBcCRC;
    }

    public HistogramCanvas canvasHSLBdB2LinkProperty() {
        return canvasHSLBdB2Link;
    }

    public HistogramCanvas canvasHSLBdCprFIFOProperty() {
        return canvasHSLBdCprFIFO;
    }

    public HistogramCanvas canvasHSLBdLenFIFOProperty() {
        return canvasHSLBdLenFIFO;
    }

    public HistogramCanvas canvasHSLBdFIFOProperty() {
        return canvasHSLBdFIFO;
    }

    public HistogramCanvas canvasHSLBdCRCProperty() {
        return canvasHSLBdCRC;
    }

    public HistogramCanvas canvasEvtRateProperty() {
        return canvasEvtRate;
    }

    public TimedGraph1D graphEvtRateInProperty() {
        return graphEvtRateIn;
    }

    public TimedGraph1D graphEvtRateOutProperty() {
        return graphEvtRateOut;
    }

    public HistogramCanvas canvasFlowRateProperty() {
        return canvasFlowRate;
    }

    public TimedGraph1D graphFlowRateInProperty() {
        return graphFlowRateIn;
    }

    public TimedGraph1D graphFlowRateOutProperty() {
        return graphFlowRateOut;
    }

    static private final SimpleDateFormat dateformat = new SimpleDateFormat("HH:mm:ss yyyy/MM/dd");

    private HistogramCanvas[] canvasHSLBB2Link;
    private HistogramCanvas[] canvasHSLBCprFIFO;
    private HistogramCanvas[] canvasHSLBLenFIFO;
    private HistogramCanvas[] canvasHSLBFIFO;
    private HistogramCanvas[] canvasHSLBCRC;

    @Override
    public void handleOnConnected() {
        //h1.setLineColor(Color.LIMEGREEN);
    }

    @Override
    public void handleOnReceived(NSMMessage msg) {
        if (msg == null) {
            return;
        }
        try {
            NSMCommand command = new NSMCommand(msg.getReqName());
            if (command.equals(NSMCommand.NSMSET)) {
                ConfigObject cobj = NSMListenerService.getDB(labelNodeName.getText());
                NSMData data = NSMListenerService.getData(labelNodeName.getText() + "_STATUS");
                if (data == null || !data.getFormat().matches("ronode_status")) {
                    return;
                }
                switch (data.getInt("state")) {
                    case 0:
                        rclabel.update(RCState.NOTREADY_S);
                        break;
                    case 1:
                        rclabel.update(RCState.READY_S);
                        break;
                    case 2:
                        rclabel.update(RCState.RUNNING_S);
                        break;
                }
                long ctime = 1000l * data.getInt("ctime");
                labelUpdateTime.setText(dateformat.format(new Date(ctime)));
                graphEvtRateIn.addPoint(data.getFloat("evtrate_in"));
                graphFlowRateIn.addPoint(data.getFloat("flowrate_in"));
                setConnection(canvasArrawFrom, data.getInt("connection_in"));
                /*
                 flow.setNqueueIn(data.getInt("nqueue_in"));
                 flow.setNeventIn(data.getInt("nevent_in"));
                 flow.setEvtrateIn(data.getFloat("evtrate_in"));
                 flow.setFlowrateIn(data.getFloat("flowrate_in"));
                 flow.setEvtsizeIn(data.getFloat("evtsize_in"));
                 flow.setNqueueIn(data.getInt("nqueue_out"));
                 flow.setNeventIn(data.getInt("nevent_out"));
                 flow.setEvtrateIn(data.getFloat("evtrate_out"));
                 flow.setFlowrateIn(data.getFloat("flowrate_out"));
                 flow.setEvtsizeIn(data.getFloat("evtsize_out"));
                 */
                fieldEvtRate.setText("" + data.getFloat("evtrate_out"));
                fieldFlowRate.setText("" + data.getFloat("flowrate_out"));
                graphEvtRateOut.addPoint(data.getFloat("evtrate_out"));
                graphFlowRateOut.addPoint(data.getFloat("flowrate_out"));
                setConnection(canvasArrawTo, data.getInt("connection_out"));
                canvasArrawFrom.update();
                canvasArrawTo.update();
                canvasEvtRate.update();
                canvasFlowRate.update();
                int eflag = data.getInt("eflag");
                /*
                 for (int i = 0; i < 32; i++) {
                 if (((eflag >> i) & 0x01) == 1) {
                 System.out.println(i);
                 }
                 }
                 */
                if (((eflag >> 5) & 0x1) > 0) {
                    canvasCprFIFO.setFillColor(Color.RED);
                    canvasCprFIFO.setLineColor(Color.PINK);
                    ((GText) canvasCprFIFO.getShapes().get(0)).setText("FULL");
                }
                if (((eflag >> 6) & 0x1) > 0) {
                    canvasCprFIFO.setFillColor(Color.ORANGE);
                    canvasCprFIFO.setLineColor(Color.GOLD);
                    ((GText) canvasCprFIFO.getShapes().get(0)).setText("EMPTY");
                } else {
                    canvasCprFIFO.setFillColor(Color.LIMEGREEN);
                    canvasCprFIFO.setLineColor(Color.LIGHTGREEN);
                    ((GText) canvasCprFIFO.getShapes().get(0)).setText("OK");
                }
                canvasCprFIFO.update();
                setStatus(eflag, 7, canvasLenFIFO);
                for (int i = 0; i < 4; i++) {
                    boolean used = cobj.getBool(String.format("hslb_%c", 'a'+i));
                    int eeflag = (!used) ? -1 : eflag;
                    setStatus(eeflag, 8 + i, canvasHSLBB2Link[i]);
                    setStatus(eeflag, 12 + i, canvasHSLBCprFIFO[i]);
                    setStatus(eeflag, 16 + i, canvasHSLBLenFIFO[i]);
                    setStatus(eeflag, 20 + i, canvasHSLBFIFO[i]);
                    setStatus(eeflag, 24 + i, canvasHSLBCRC[i]);
                }
                setStatus(eflag, 28, canvasTTRXB2Link);
                setStatus(eflag, 29, canvasTTRXLinkUp);
            }
        } catch (Exception e) {
            Logger.getLogger(COPPERMonitorPane.class.getName()).log(Level.SEVERE, null, e);
        }
    }

    private void setStatus(int eflag, int bit, HistogramCanvas canvas) {
        if (eflag == -1) {
            canvas.setFillColor(Color.TRANSPARENT);
            canvas.setLineColor(Color.LIGHTGRAY);
            ((GText) canvas.getShapes().get(0)).setText("");
        } else if (((eflag >> bit) & 0x1) > 0) {
            canvas.setFillColor(Color.RED);
            canvas.setLineColor(Color.PINK);
            ((GText) canvas.getShapes().get(0)).setText("ERROR");
        } else {
            canvas.setFillColor(Color.LIMEGREEN);
            canvas.setLineColor(Color.LIGHTGREEN);
            ((GText) canvas.getShapes().get(0)).setText("OK");
        }
        canvas.update();
    }

    @Override
    public void handleOnDisConnected() {
        //h1.setLineColor(Color.DEEPPINK);
    }

    @Override
    public void log(LogMessage log) {
    }

    void setNodeName(String nodename) {
        labelNodeName.setText(nodename.replace("_STATUS", ""));
    }

    String getNodeName() {
        return labelNodeName.getText();
    }

    public COPPERMonitorPane() {
        FXMLLoader loader = new FXMLLoader(COPPERMonitorPane.class.getResource("COPPERMonitorPane.fxml"));
        loader.setRoot(this);
        loader.setController(this);
        try {
            loader.load();
        } catch (IOException exception) {
            throw new RuntimeException(exception);
        }
        canvasHSLBB2Link = new HistogramCanvas[]{canvasHSLBaB2Link, canvasHSLBbB2Link, canvasHSLBcB2Link, canvasHSLBdB2Link};
        canvasHSLBCprFIFO = new HistogramCanvas[]{canvasHSLBaCprFIFO, canvasHSLBbCprFIFO, canvasHSLBcCprFIFO, canvasHSLBdCprFIFO};
        canvasHSLBLenFIFO = new HistogramCanvas[]{canvasHSLBaLenFIFO, canvasHSLBbLenFIFO, canvasHSLBcLenFIFO, canvasHSLBdLenFIFO};
        canvasHSLBFIFO = new HistogramCanvas[]{canvasHSLBaFIFO, canvasHSLBbFIFO, canvasHSLBcFIFO, canvasHSLBdFIFO};
        canvasHSLBCRC = new HistogramCanvas[]{canvasHSLBaCRC, canvasHSLBbCRC, canvasHSLBcCRC, canvasHSLBdCRC};
    }

    public COPPERMonitorPane(String name) {
        this();
        setNodeName(name);
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
