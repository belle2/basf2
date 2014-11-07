/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import b2daq.database.ConfigObject;
import b2daq.database.DBObject;
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
public class ROPCMonitorPane extends VBox implements NSMObserver {

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
    private HistogramCanvas canvasRecvStream0;
    @FXML
    private HistogramCanvas canvasEB0;
    @FXML
    private HistogramCanvas canvasRecvStream1;
    @FXML
    private HistogramCanvas canvasEB1TX;
    @FXML
    private Label labelNode02;
    @FXML
    private Label labelNode01;
    @FXML
    private HistogramCanvas canvasNode02;
    @FXML
    private HistogramCanvas canvasNode01;
    @FXML
    private HistogramCanvas canvasNode07;
    @FXML
    private HistogramCanvas canvasNode11;
    @FXML
    private HistogramCanvas canvasNode12;
    @FXML
    private HistogramCanvas canvasNode03;
    @FXML
    private HistogramCanvas canvasNode06;
    @FXML
    private HistogramCanvas canvasNode08;
    @FXML
    private HistogramCanvas canvasNode13;
    @FXML
    private HistogramCanvas canvasNode04;
    @FXML
    private HistogramCanvas canvasNode09;
    @FXML
    private HistogramCanvas canvasNode14;
    @FXML
    private HistogramCanvas canvasNode05;
    @FXML
    private HistogramCanvas canvasNode10;
    @FXML
    private HistogramCanvas canvasNode15;
    @FXML
    private Label labelNode03;
    @FXML
    private Label labelNode04;
    @FXML
    private Label labelNode05;
    @FXML
    private Label labelNode06;
    @FXML
    private Label labelNode07;
    @FXML
    private Label labelNode08;
    @FXML
    private Label labelNode09;
    @FXML
    private Label labelNode10;
    @FXML
    private Label labelNode11;
    @FXML
    private Label labelNode12;
    @FXML
    private Label labelNode13;
    @FXML
    private Label labelNode14;
    @FXML
    private Label labelNode15;
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

    public HistogramCanvas canvasRecvStream0Property() {
        return canvasRecvStream0;
    }

    public HistogramCanvas canvasEB0Property() {
        return canvasEB0;
    }

    public HistogramCanvas canvasRecvStream1Property() {
        return canvasRecvStream1;
    }

    public HistogramCanvas canvasEB1TXProperty() {
        return canvasEB1TX;
    }

    public Label labelNode02Property() {
        return labelNode02;
    }

    public Label labelNode01Property() {
        return labelNode01;
    }

    public HistogramCanvas canvasNode02Property() {
        return canvasNode02;
    }

    public HistogramCanvas canvasNode01Property() {
        return canvasNode01;
    }

    public HistogramCanvas canvasNode07Property() {
        return canvasNode07;
    }

    public HistogramCanvas canvasNode11Property() {
        return canvasNode11;
    }

    public HistogramCanvas canvasNode12Property() {
        return canvasNode12;
    }

    public HistogramCanvas canvasNode03Property() {
        return canvasNode03;
    }

    public HistogramCanvas canvasNode06Property() {
        return canvasNode06;
    }

    public HistogramCanvas canvasNode08Property() {
        return canvasNode08;
    }

    public HistogramCanvas canvasNode13Property() {
        return canvasNode13;
    }

    public HistogramCanvas canvasNode04Property() {
        return canvasNode04;
    }

    public HistogramCanvas canvasNode09Property() {
        return canvasNode09;
    }

    public HistogramCanvas canvasNode14Property() {
        return canvasNode14;
    }

    public HistogramCanvas canvasNode05Property() {
        return canvasNode05;
    }

    public HistogramCanvas canvasNode10Property() {
        return canvasNode10;
    }

    public HistogramCanvas canvasNode15Property() {
        return canvasNode15;
    }

    public Label labelNode03Property() {
        return labelNode03;
    }

    public Label labelNode04Property() {
        return labelNode04;
    }

    public Label labelNode05Property() {
        return labelNode05;
    }

    public Label labelNode06Property() {
        return labelNode06;
    }

    public Label labelNode07Property() {
        return labelNode07;
    }

    public Label labelNode08Property() {
        return labelNode08;
    }

    public Label labelNode09Property() {
        return labelNode09;
    }

    public Label labelNode10Property() {
        return labelNode10;
    }

    public Label labelNode11Property() {
        return labelNode11;
    }

    public Label labelNode12Property() {
        return labelNode12;
    }

    public Label labelNode13Property() {
        return labelNode13;
    }

    public Label labelNode14Property() {
        return labelNode14;
    }

    public Label labelNode15Property() {
        return labelNode15;
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

    private HistogramCanvas[] canvasNode;
    private Label[] labelNode;

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
                int [] reserved_i = new int [] {data.getInt("reserved_i", 0), data.getInt("reserved_i", 1)}; 
                setStatus(eflag, reserved_i[0], reserved_i[1], 0, canvasRecvStream0, null);
                setStatus(eflag, reserved_i[0], reserved_i[1], 1, canvasEB0, null);
                setStatus(eflag, reserved_i[0], reserved_i[1], 2, canvasRecvStream1, null);
                setStatus(eflag, reserved_i[0], reserved_i[1], 3, canvasEB1TX, null);
                for (int i = 0; i < 15; i++) {
                    setStatus(eflag, reserved_i[0], reserved_i[1], i + 4, canvasNode[i], labelNode[i]);
                    if (cobj != null && 
                        cobj.hasObject("copper_from") && i < cobj.getNObjects("copper_from")) {
                        DBObject ccobj = cobj.getObject("copper_from", i);
                        labelNode[i].setText(ccobj.getText("hostname"));
                    }
                }
            }
        } catch (Exception e) {
            Logger.getLogger(ROPCMonitorPane.class.getName()).log(Level.SEVERE, null, e);
        }
    }

    private void setStatus(int eflag, int reserved0, int reserved1, int bit, HistogramCanvas canvas, Label label) {
        if (((eflag >> bit) & 0x1) > 0) {
            canvas.setFillColor(Color.RED);
            canvas.setLineColor(Color.PINK);
            ((GText) canvas.getShapes().get(0)).setText("ERROR");
            if (label != null) label.setVisible(true);
        } else if (((reserved0 >> bit) & 0x1) > 0) {
            canvas.setFillColor(Color.LIMEGREEN);
            canvas.setLineColor(Color.LIGHTGREEN);
            ((GText) canvas.getShapes().get(0)).setText("OK");
            if (label != null) label.setVisible(true);
        } else {
            canvas.setFillColor(Color.TRANSPARENT);
            canvas.setLineColor(Color.TRANSPARENT);
            ((GText) canvas.getShapes().get(0)).setText("");
            if (label != null) label.setVisible(false);
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

    public ROPCMonitorPane() {
        FXMLLoader loader = new FXMLLoader(ROPCMonitorPane.class.getResource("ROPCMonitorPane.fxml"));
        loader.setRoot(this);
        loader.setController(this);
        try {
            loader.load();
        } catch (IOException exception) {
            throw new RuntimeException(exception);
        }
        canvasNode = new HistogramCanvas[] {canvasNode01, canvasNode02, canvasNode03, canvasNode04, canvasNode05,
            canvasNode06, canvasNode07, canvasNode08, canvasNode09, canvasNode10,
            canvasNode11, canvasNode12, canvasNode13, canvasNode14, canvasNode15};
        labelNode = new Label[] {labelNode01, labelNode02, labelNode03, labelNode04, labelNode05,
            labelNode06, labelNode07, labelNode08, labelNode09, labelNode10,
            labelNode11, labelNode12, labelNode13, labelNode14, labelNode15};
    }

    public ROPCMonitorPane(String name) {
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
