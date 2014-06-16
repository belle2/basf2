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
public class DataFlowMonitorController implements Initializable, NSMObserver {

    @FXML
    private VBox pane;
    @FXML
    private HistogramCanvas c_in;
    @FXML
    private HistogramCanvas c_out;
    @FXML
    private HistogramCanvas c_rate;
    @FXML
    private HistogramCanvas c_size;
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
    private TimedGraph1D gr_rate_in;
    @FXML
    private TimedGraph1D gr_rate_out;
    @FXML
    private TimedGraph1D gr_size_in;
    @FXML
    private TimedGraph1D gr_size_out;
    @FXML
    private Label label_nodename;
    private ConfigObject obj;
    private HistogramCanvas[] c;
    private TimedGraph1D[] gr_rate;
    private TimedGraph1D[] gr_size;
    static private final SimpleDateFormat dateformat = new SimpleDateFormat("HH:mm:ss yyyy/MM/dd");

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        c = new HistogramCanvas [] {c_in, c_out};
        gr_rate = new TimedGraph1D[] {gr_rate_in, gr_rate_out};
        gr_size = new TimedGraph1D[] {gr_size_in, gr_size_out};
        table_stat.getStyleClass().add("dataflow-table");
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
            if (command.equals(NSMCommand.NSMSET)) {
                ConfigObject cobj = NSMListenerService.getDB(label_nodename.getText());
                if (cobj != null) {
                    if (cobj.getTable().matches("copper")) {
                        c_in.setVisible(false);
                        label_src.setVisible(false);
                        label_dest.setText("EB0");
                    } else if (cobj.getTable().matches("ropc")) {
                        label_src.setText("EB0");
                        //c_in.setVisible(false);
                        label_dest.setText("EB1TX");
                    }
                }
                NSMData data = NSMListenerService.getData("STATUS_" + label_nodename.getText());
                if (data == null || !data.getFormat().matches("ronode_status")) {
                    return;
                }
                //System.out.println(data.getName() + ":" + data.getFormat()+" "+ data.getInt("state"));
                //data.print();
                label_runno.setText(String.format("%04d.%04d.%03d", data.getInt("expno"),
                        data.getInt("runno"), data.getInt("subno")));
                state.update(RCState.get(data.getInt("state")));
                if (table_stat.getItems().size() < data.getNObjects("io")) {
                    table_stat.getItems().clear();
                    for (int i = 0; i < data.getNObjects("io"); i++) {
                        table_stat.getItems().add(new DataFlow((i==0?"IN":"OUT")));
                    }
                }
                long ctime = 1000l * data.getInt("ctime");
                label_ctime.setText(dateformat.format(new Date(ctime)));
                for (int i = 0; i < data.getNObjects("io"); i++) {
                    NSMData cdata = (NSMData)data.getObject("io", i);
                    DataFlow flow = (DataFlow)table_stat.getItems().get(i);
                    flow.setCount(cdata.getInt("count"));
                    flow.setFreq(cdata.getFloat("freq"));
                    flow.setRate(cdata.getFloat("rate"));
                    flow.setSize(cdata.getFloat("evtsize"));
                    gr_rate[i].addPoint(cdata.getFloat("freq"));
                    gr_size[i].addPoint(cdata.getFloat("evtsize"));
                    setConnection(c[i], cdata.getInt("port"));
                }
                c_in.update();
                c_out.update();
                c_rate.update();
                c_size.update();
            }
        } catch (Exception e) {
            Logger.getLogger(DataFlowMonitorController.class.getName()).log(Level.SEVERE, null, e);
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
        this.label_nodename.setText(nodename);
    }

    String getNodeName() {
        return label_nodename.getText();
    }

    static final DataFlowMonitorController create(String name) {
        try {
            FXMLLoader loader = new FXMLLoader(DataFlowMonitorController.class.getResource("DataFlowMonitor.fxml"));
            loader.load();
            DataFlowMonitorController controller = loader.getController();
            controller.setNodeName(name);
            return controller;
        } catch (IOException e) {
            Logger.getLogger(DataFlowMonitorController.class.getName()).log(Level.SEVERE, null, e);
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
