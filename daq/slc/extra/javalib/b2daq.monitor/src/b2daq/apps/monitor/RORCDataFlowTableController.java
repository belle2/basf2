/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.monitor;

import b2daq.database.ConfigObject;
import b2daq.dqm.graphics.HistogramCanvas;
import b2daq.graphics.GShape;
import b2daq.logger.core.LogMessage;
import b2daq.nsm.NSMCommand;
import b2daq.nsm.NSMData;
import b2daq.nsm.NSMListenerService;
import b2daq.nsm.NSMMessage;
import b2daq.nsm.NSMObserver;
import b2daq.runcontrol.core.RCState;
import b2daq.runcontrol.ui.RCStateLabel;
import java.io.IOException;
import java.net.URL;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.ResourceBundle;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.fxml.Initializable;
import javafx.scene.control.Label;
import javafx.scene.control.TableView;
import javafx.scene.layout.Pane;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class RORCDataFlowTableController implements Initializable, NSMObserver {

    @FXML
    private VBox pane;
    @FXML
    private HistogramCanvas c_in;
    @FXML
    private HistogramCanvas c_out;
    @FXML
    private RCStateLabel state;
    @FXML
    private Label label_src;
    @FXML
    private Label label_dest;
    @FXML
    private Label label_runno;
    @FXML
    private Label label_ctime;
    @FXML
    private TableView table_stat;
    @FXML
    private Label label_nodename;
    static private final SimpleDateFormat dateformat = new SimpleDateFormat("HH:mm:ss yyyy/MM/dd");

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        table_stat.getStyleClass().add("dataflow-table");
        label_src.setText("eb0");
        label_dest.setText("eb1tx");
    }

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
            if (command.equals(NSMCommand.DATASET)) {
                NSMData data = NSMListenerService.getData(label_nodename.getText()+"_STATUS");
                ConfigObject cobj = NSMListenerService.getDB(label_nodename.getText());
                if (cobj == null) {
                    //NSMListenerService.requestDBGet(label_nodename.getText(), 
                    //        data.getInt("configid", 0));
                }
                if (data == null || !data.getFormat().matches("rorc_status")) {
                    return;
                }
                label_runno.setText(String.format("%04d.%04d.%03d", data.getInt("expno"),
                        data.getInt("runno"), data.getInt("subno")));
                switch (data.getInt("state")) {
                    default:
                    state.update(RCState.NOTREADY_S);
                    break;
                    case 3:
                    state.update(RCState.READY_S);
                    break;
                    case 4:
                    state.update(RCState.RUNNING_S);
                    break;
                }
                if (table_stat.getItems().size() < data.getInt("nnodes")) {
                    table_stat.getItems().clear();
                    for (int i = 0; i < data.getInt("nnodes"); i++) {
                        table_stat.getItems().add(new DataFlow());
                    }
                }
                long ctime = 1000l * data.getInt("ctime");
                label_ctime.setText(dateformat.format(new Date(ctime)));
                int connected_in = 1;
                int connected_out = -1;
                for (int i = 0; i < data.getInt("nnodes"); i++) {
                    NSMData cdata = (NSMData)data.getObject("ro", i);
                    NSMData node = (NSMData)data.getObject("node", i);
                    if (i == 0) {
                        connected_out = (cdata.getInt("connection_out") == 1)?1:-1;
                    } else {
                        if (cdata.getInt("connection_out") != 1 && node.getInt("excluded") == 0) {
                            connected_in = -1;
                        }
                    }
                    DataFlow flow = (DataFlow)table_stat.getItems().get(i);
                    if (cobj != null && flow.getNode().isEmpty()) {
                        //flow.setNode(cobj.getObject("node", i).getObject("runtype").getNode());
                    }
                    flow.setLoadavg(cdata.getFloat("loadavg"));
                    flow.setNqueueIn(cdata.getInt("nqueue_in"));
                    flow.setNeventIn(cdata.getInt("nevent_in"));
                    flow.setEvtrateIn(cdata.getFloat("evtrate_in"));
                    flow.setFlowrateIn(cdata.getFloat("flowrate_in"));
                    flow.setEvtsizeIn(cdata.getFloat("evtsize_in"));
                    flow.setNqueueOut(cdata.getInt("nqueue_out"));
                    flow.setNeventOut(cdata.getInt("nevent_out"));
                    flow.setEvtrateOut(cdata.getFloat("evtrate_out"));
                    flow.setFlowrateOut(cdata.getFloat("flowrate_out"));
                    flow.setEvtsizeOut(cdata.getFloat("evtsize_out"));
                }
                setConnection(c_in, connected_in);
                setConnection(c_out, connected_out);
            }
        } catch (Exception e) {
            e.printStackTrace();
            Logger.getLogger(RORCDataFlowTableController.class.getName()).log(Level.SEVERE, null, e);
        }
    }

    @Override
    public void handleOnDisConnected() {
        //h1.setLineColor(Color.DEEPPINK);
    }

    @Override
    public void log(LogMessage log) {
    }

    void setNodeName(String nodename) {
        this.label_nodename.setText(nodename.replace("_STATUS", ""));
    }

    String getNodeName() {
        return label_nodename.getText();
    }

    static RORCDataFlowTableController create(String name) {
        try {
            FXMLLoader loader = new FXMLLoader(RORCDataFlowTableController.class.getResource("RORCDataFlowTable.fxml"));
            loader.load();
            RORCDataFlowTableController controller = loader.getController();
            controller.setNodeName(name);
            return controller;
        } catch (IOException e) {
            e.printStackTrace();
            Logger.getLogger(RORCDataFlowTableController.class.getName()).log(Level.SEVERE, null, e);
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
        c.update();
    }

}
