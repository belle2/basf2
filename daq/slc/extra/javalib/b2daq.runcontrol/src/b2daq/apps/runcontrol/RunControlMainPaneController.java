/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import b2daq.core.LogLevel;
import b2daq.database.ConfigObject;
import b2daq.logger.core.LogMessage;
import b2daq.logger.ui.LogViewPaneController;
import b2daq.nsm.NSMCommand;
import b2daq.nsm.NSMConfig;
import b2daq.nsm.NSMData;
import b2daq.nsm.NSMDataProperty;
import b2daq.nsm.NSMListenerService;
import b2daq.nsm.NSMMessage;
import b2daq.nsm.NSMObserver;
import b2daq.runcontrol.core.RCCommand;
import b2daq.runcontrol.core.RCState;
import b2daq.ui.NetworkConfigPaneController;
import java.net.URL;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.ResourceBundle;
import javafx.beans.value.ChangeListener;
import javafx.beans.value.ObservableValue;
import javafx.collections.ObservableList;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Label;
import javafx.scene.control.Tab;
import javafx.scene.control.TabPane;
import javafx.scene.layout.GridPane;
import javafx.scene.paint.Color;
import javafx.scene.shape.Rectangle;
import javafx.scene.text.Text;

/**
 *
 * @author tkonno
 */
public class RunControlMainPaneController implements Initializable, NSMObserver {

    @FXML
    private LogViewPaneController logviewController;
    @FXML
    private RunSettingPaneController runSettingsController;
    @FXML
    private RunCommandButtonPaneController commandButtonController;
    @FXML
    private RunStateLabelController rcStateController;
    @FXML
    private NetworkConfigPaneController networkconfigController;
    @FXML
    private CopperEditorController copperEditorController;
    
    private final HashMap<String, DataFlowMonitorController> flowmonitors = new HashMap<>();
    @FXML
    private TabPane tabpane_mon;
    @FXML
    private Label label_rcnode;
    @FXML
    private Label label_node0;
    @FXML
    private Label label_node1;
    @FXML
    private Label label_node2;
    @FXML
    private Label label_node3;
    @FXML
    private Label label_node4;
    @FXML
    private Label label_node5;
    @FXML
    private Label label_node6;
    @FXML
    private Label label_node7;
    @FXML
    private Label label_node8;
    @FXML
    private Label label_node9;
    @FXML
    private Label label_node10;
    @FXML
    private Label label_node11;
    @FXML
    private Label label_node12;
    @FXML
    private Label label_node13;
    @FXML
    private Label label_node14;

    @FXML
    private Rectangle rect_rc;
    @FXML
    private Rectangle rect_node0;
    @FXML
    private Rectangle rect_node1;
    @FXML
    private Rectangle rect_node2;
    @FXML
    private Rectangle rect_node3;
    @FXML
    private Rectangle rect_node4;
    @FXML
    private Rectangle rect_node5;
    @FXML
    private Rectangle rect_node6;
    @FXML
    private Rectangle rect_node7;
    @FXML
    private Rectangle rect_node8;
    @FXML
    private Rectangle rect_node9;
    @FXML
    private Rectangle rect_node10;
    @FXML
    private Rectangle rect_node11;
    @FXML
    private Rectangle rect_node12;
    @FXML
    private Rectangle rect_node13;
    @FXML
    private Rectangle rect_node14;

    @FXML
    private Text text_rc;
    @FXML
    private Text text_node0;
    @FXML
    private Text text_node1;
    @FXML
    private Text text_node2;
    @FXML
    private Text text_node3;
    @FXML
    private Text text_node4;
    @FXML
    private Text text_node5;
    @FXML
    private Text text_node6;
    @FXML
    private Text text_node7;
    @FXML
    private Text text_node8;
    @FXML
    private Text text_node9;
    @FXML
    private Text text_node10;
    @FXML
    private Text text_node11;
    @FXML
    private Text text_node12;
    @FXML
    private Text text_node13;
    @FXML
    private Text text_node14;

    @FXML
    private GridPane summary_grid;
    @FXML
    private Label label_runnum;
    @FXML
    private Label label_starttime;
    @FXML
    private Label label_endtime;
    @FXML
    private Label label_runtype;

    private RunStateLabelController[] statelabel_v = null;
    private Label[] label_v = null;
    private Rectangle[] rect_v = null;
    private Text[] text_v = null;

    private String[] namelist = null;

    private final HashMap<String, RunStateLabelController> label_m
            = new HashMap<>();

    @Override
    public void initialize(URL url, ResourceBundle rb) {
        label_v = new Label[]{label_node0, label_node1, label_node2,
            label_node3, label_node4, label_node5,
            label_node6, label_node7, label_node8, 
            label_node9, label_node10, label_node11,
            label_node12, label_node13, label_node14
        };
        rect_v = new Rectangle[]{rect_node0, rect_node1, rect_node2,
            rect_node3, rect_node4, rect_node5,
            rect_node6, rect_node7, rect_node8,
            rect_node9, rect_node10, rect_node11,
            rect_node12, rect_node13, rect_node14
        };
        text_v = new Text[]{text_node0, text_node1, text_node2,
            text_node3, text_node4, text_node5, 
            text_node6, text_node7, text_node8, 
            text_node9, text_node10, text_node11,
            text_node12, text_node13, text_node14
        };
        statelabel_v = new RunStateLabelController[label_v.length];
        for (int i = 0; i < label_v.length; i++) {
            statelabel_v[i] = new RunStateLabelController(rect_v[i], text_v[i]);
        }
        rcStateController = new RunStateLabelController(rect_rc, text_rc);
        rcStateController.setVisible(true);
        logviewController.add(new LogMessage("LOCAL", LogLevel.INFO, "GUI opened"));
//        Tab tab = new Tab();
//        tab.setText("Summary");
//        tab.setContent(DataFlowSummaryController.create("").getPane());
//        tabpane_mon.getTabs().add(tab);
    }

    @Override
    public void handleOnConnected() {
        NSMDataProperty pro = getNSMDataProperties().get(0);
        NSMListenerService.requestNSMGet(pro.getDataname(),
                pro.getFormat(), pro.getRevision());
        NSMConfig config = NSMListenerService.getNSMConfig();
        label_rcnode.setText(config.getNsmTarget());
        rcStateController.setFont(Color.BLACK);
        commandButtonController.set(this);
        runSettingsController.bind(this);
        ChangeListener listener = (ChangeListener) (ObservableValue observable, Object oldValue, Object newValue) -> {
            final RCState state = new RCState();
            state.copy(text_rc.getText());
            commandButtonController.update(state,
                    runSettingsController.readyProperty().get(),
                    runSettingsController.getFieldOperator1().getText().length() > 0);
        };
        text_rc.textProperty().addListener(listener);
        runSettingsController.getFieldOperator1().textProperty().addListener(listener);
        runSettingsController.readyProperty().addListener(listener);
        networkconfigController.setState(true, true);
    }

    @Override
    public void handleOnReceived(NSMMessage msg) {
        if (msg == null) {
            rcStateController.update(0);
            return;
        }
        RCCommand command = new RCCommand();
        command.copy(msg.getReqName());
        if (command.equals(NSMCommand.LOG)) {
            String[] str = msg.getData().split("\n");
            String from = str[0];
            StringBuilder ss = new StringBuilder();
            for (int i = 1; i < str.length; i++) {
                ss.append(str[i]);
            }
            long date = 1000l + msg.getParam(1);
            log(new LogMessage(from, LogLevel.Get(msg.getParam(0)),
                    new Date(date), ss.toString()));
        } else if (command.equals(NSMCommand.LISTSET)) {
            if (msg.getNParams() > 0 && msg.getParam(0) > 0) {
                namelist = msg.getData().split("\n");
                System.out.println(msg.getData());
                commandButtonController.clearStack();
            }
        } else if (command.equals(NSMCommand.DBSET)) {
            ConfigObject cobj = NSMListenerService.getDB(msg.getNodeName());
            if (cobj != null) {
                //System.out.println(cobj.getName() + " " + cobj.getTable() + " " + NSMListenerService.getNSMConfig().getNsmTarget());
                //cobj.print();
                copperEditorController.handleOnReceived(msg);
                networkconfigController.add(cobj);
                if (cobj.getNode().matches(NSMListenerService.getNSMConfig().getNsmTarget()) && 
                        cobj.getTable().matches("runcontrol")) {
                    label_runtype.setText(cobj.getName());
                    if (cobj.hasObject("node")) {
                        int count = 0;
                        for (ConfigObject obj : cobj.getObjects("node")) {
                            String name = obj.getObject("runtype").getNode();
                            if (label_v[count] != null) {
                                label_v[count].setText(name);
                            }
                            if (statelabel_v[count] != null) {
                                statelabel_v[count].setFont(Color.BLACK);
                                statelabel_v[count].set(name, logviewController);
                                label_m.put(name, statelabel_v[count]);
                            }
                            count++;
                            //System.out.println(name+":"+obj.getObject("runtype").getId());
                            NSMListenerService.requestDBGet(name, obj.getObject("runtype").getId());
                        }
                    }
                }
            }
        } else if (command.equals(NSMCommand.NSMSET)) {
            String dataname = msg.getNodeName();
            NSMData data = NSMListenerService.getData(dataname);
            Tab tab = null;
            String nodename = msg.getNodeName().replace("_STATUS", "");
            for (Tab t : tabpane_mon.getTabs()) {
                if (data.getFormat().matches("ronode_status")
                        && t.getText().matches(nodename)) {
                    tab = t;
                    flowmonitors.get(nodename).handleOnReceived(msg);
                    break;
                }
            }
            if (data.getFormat().matches("ronode_status") && tab == null) {
                DataFlowMonitorController flowmonitor = DataFlowMonitorController.create(nodename);
                tab = new Tab();
                tab.setText(flowmonitor.getNodeName());
                tab.setContent(flowmonitor.getPane());
                tab.setClosable(false);
                tabpane_mon.getTabs().add(tab);
                flowmonitors.put(nodename, flowmonitor);
            }
            dataname = getNSMDataProperties().get(0).getDataname();
            if (dataname.matches(msg.getNodeName())) {
                data = NSMListenerService.getData(dataname);
                networkconfigController.add(data);
                NSMConfig config = NSMListenerService.getNSMConfig();
                ConfigObject cobj = NSMListenerService.getDB(config.getNsmTarget());
                label_runnum.setText(String.format("%04d.%04d.%03d",
                        data.getInt("expno"), data.getInt("runno"),
                        data.getInt("subno")));
                long stime = data.getInt("stime");
                if (stime > 0) {
                    final SimpleDateFormat dateformat = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss");
                    Date date = new Date(stime * 1000);
                    label_starttime.setText(dateformat.format(date));
                } else {
                    label_starttime.setText("---- / -- / --  -- : -- : --");
                }

                if (cobj == null) {
                    NSMListenerService.requestDBGet(config.getNsmTarget(),
                            data.getInt("configid", 0));
                } else {
                    runSettingsController.update(cobj, data);
                    rcStateController.update(data.getInt("state"));
                    //System.out.println("rc.state="+data.getInt("state"));
                    if (cobj.hasObject("node")) {
                        int n = 0;
                        for (ConfigObject obj : cobj.getObjects("node")) {
                            //System.out.println("node["+n+"].state="+data.getObject("node", n).getInt("state"));
                            statelabel_v[n].update(data.getObject("node", n).getInt("state"));
                            n++;
                        }
                    }
                }
            }
        } else if (command.equals(RCCommand.OK)) {
            RCState state = new RCState();
            state.copy(msg.getData());
            rcStateController.update(state.getId());
            if (!state.isTransition()) {
                String dataname = getNSMDataProperties().get(0).getDataname();
                logviewController.add(new LogMessage(msg.getNodeName(),
                        LogLevel.INFO, "State shift "
                        + msg.getNodeName() + ">> "
                        + state.getLabel()));
                NSMListenerService.requestNSMGet(dataname, "", 0);
            }
            if (state.equals(RCState.RUNNING_S)) {
                runSettingsController.clear();
            }
        } else if (command.equals(RCCommand.STATE)) {
            RCState state = new RCState();
            String[] str_v = msg.getData().split(" ");
            if (str_v.length > 1) {
                if (label_m.containsKey(str_v[0])) {
                    state.copy(str_v[1]);
                    label_m.get(str_v[0]).update(state.getId());
                    //System.out.println("" + str_v[0] + " = " + state.getLabel());
                }
            } else {
                System.out.println("'" + msg.getData() + "' " + msg.getData().contains(" "));
            }
        }
    }

    @Override
    public void handleOnDisConnected() {
    }

    @Override
    public void log(LogMessage log
    ) {
        logviewController.add(log);
    }

    public LogViewPaneController getLogView() {
        return logviewController;
    }

    public RunSettingPaneController getRunSetting() {
        return runSettingsController;
    }

    public NetworkConfigPaneController getNetworkConfig() {
        return networkconfigController;
    }

    public Label getLabelRunNumbers() {
        return label_runnum;
    }

    public Label getLabelStartTime() {
        return label_starttime;
    }

    public Label getLabelEndTime() {
        return label_endtime;
    }

    public String[] getNameList() {
        return namelist;
    }

    public Label getLabelRunType() {
        return label_runtype;
    }

    public Text getTextRCState() {
        return text_rc;
    }

    public ObservableList<NSMDataProperty> getNSMDataProperties() {
        return networkconfigController.getNSMDataProperties();
    }
}
