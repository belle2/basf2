/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.pscontrol;

import b2daq.core.LogLevel;
import b2daq.database.ConfigObject;
import b2daq.hvcontrol.core.HVCommand;
import b2daq.hvcontrol.core.HVState;
import b2daq.logger.core.LogMessage;
import b2daq.logger.ui.LogViewPaneController;
import b2daq.nsm.NSMCommand;
import b2daq.nsm.NSMConfig;
import b2daq.nsm.NSMData;
import b2daq.nsm.NSMDataProperty;
import b2daq.nsm.NSMListenerService;
import b2daq.nsm.NSMMessage;
import b2daq.nsm.NSMObserver;
import b2daq.ui.NetworkConfigPaneController;
import java.net.URL;
import java.text.SimpleDateFormat;
import java.util.Comparator;
import java.util.Date;
import java.util.HashMap;
import java.util.ResourceBundle;
import javafx.collections.ObservableList;
import javafx.event.Event;
import javafx.event.EventHandler;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Label;
import javafx.scene.control.RadioButton;
import javafx.scene.control.TabPane;
import javafx.scene.control.TableCell;
import javafx.scene.control.TableColumn;
import javafx.scene.control.TableView;
import javafx.scene.control.cell.PropertyValueFactory;
import javafx.scene.layout.GridPane;
import javafx.scene.paint.Color;
import javafx.scene.shape.Rectangle;
import javafx.scene.text.Text;
import javafx.util.Callback;

/**
 *
 * @author tkonno
 */
public class PowerSupplyMainPaneController implements Initializable, NSMObserver {

    @FXML
    private PowerSettingPaneController powerSettingsController;
    @FXML
    private PowerControlCommandButtonPaneController commandButtonController;
    @FXML
    private HVStatusMonitorController hvmonitorController;
    @FXML
    private TabPane state_tabpane;
    @FXML
    private GridPane summary_grid;
    @FXML
    private Label label_psnode;
    @FXML
    private Label label_update;
    @FXML
    private Label label_modules;
    @FXML
    private Label label_channels;
    @FXML
    private Label label_config;
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
    private Rectangle rect_node0;
    @FXML
    private Text text_node0;
    @FXML
    private Rectangle rect_node1;
    @FXML
    private Text text_node1;
    @FXML
    private Rectangle rect_node2;
    @FXML
    private Text text_node2;
    @FXML
    private Rectangle rect_node3;
    @FXML
    private Text text_node3;
    @FXML
    private Rectangle rect_node4;
    @FXML
    private Text text_node4;
    @FXML
    private Rectangle rect_rc;
    @FXML
    private Text text_rc;
    @FXML
    private LogViewPaneController logviewController;
    @FXML
    private NetworkConfigPaneController networkconfigController;
    @FXML
    private PowerStateLabelController psStateController;
    @FXML
    private HVParamEditorPaneController hvparameditorController;
    @FXML
    private TableView table_node0;
    @FXML
    private TableView table_node1;
    @FXML
    private TableColumn col_state0;
    @FXML
    private TableColumn col_state1;
    @FXML
    private RadioButton radio_programatic;
    @FXML
    private RadioButton radio_manual;
    
    private PowerStateLabelController[] statelabel_v = null;
    private Label[] label_v = null;
    private Rectangle[] rect_v = null;
    private Text[] text_v = null;
    private TableView[] table_v = null;

    private String[] namelist = null;

    private final ConfigObject cobj = new ConfigObject();

    private final HashMap<Integer, PowerStateLabelController> label_m
            = new HashMap<>();
    private boolean sent_listrequest = false;

    public ConfigObject getDB() {
        return cobj;
    }

    @Override
    public void initialize(URL url, ResourceBundle rb) {
        label_v = new Label[]{label_node0, label_node1,
            label_node2, label_node3, label_node4
        };
        rect_v = new Rectangle[]{rect_node0, rect_node1,
            rect_node2, rect_node3, rect_node4
        };
        text_v = new Text[]{text_node0, text_node1,
            text_node2, text_node3, text_node4
        };
        table_v = new TableView[]{table_node0};
        statelabel_v = new PowerStateLabelController[label_v.length];
        for (int i = 0; i < label_v.length; i++) {
            statelabel_v[i] = new PowerStateLabelController(rect_v[i], text_v[i]);
        }
        psStateController = new PowerStateLabelController(rect_rc, text_rc);
        psStateController.setVisible(true);
        logviewController.add(new LogMessage("LOCAL", LogLevel.INFO, "GUI opened"));
        setStateColumn(col_state0);
        //setStateColumn(col_state1);
        radio_manual.setOnAction(new EventHandler() {
            @Override
            public void handle(Event event) {
                if (!radio_manual.isSelected()) {
                    powerSettingsController.getVBox().setDisable(false);
                }
            }
        });
    }

    @Override
    public void handleOnConnected() {
        NSMDataProperty pro = getNSMDataProperties().get(0);
        NSMListenerService.requestNSMGet(pro.getDataname(),
                pro.getFormat(), pro.getRevision());
        NSMConfig config = NSMListenerService.getNSMConfig();
        label_psnode.setText(config.getNsmTarget());
        psStateController.setFont(Color.BLACK);
        commandButtonController.set(this);
    }

    @Override
    public void handleOnDisConnected() {

    }

    @Override
    public void log(LogMessage log) {
        logviewController.add(log);
    }

    @Override
    public void handleOnReceived(NSMMessage msg) {
        if (msg == null) {
            psStateController.update(0);
            return;
        }
        //System.out.println(msg.getReqName());
        HVCommand command = new HVCommand();
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
                //System.out.println(namelist[0]);
                commandButtonController.clearStack();
            }
        } else if (command.equals(NSMCommand.DBSET)) {
            msg.getData(cobj);
            if (cobj.getId() > 0 && sent_listrequest) {
                sent_listrequest = false;
                label_config.setText(cobj.getName());
                networkconfigController.add(cobj);
                powerSettingsController.update(cobj);
                if (cobj.hasObject("channel")) {
                    int count = 0;
                    for (ConfigObject obj : cobj.getObjects("channel")) {
                        int crate = obj.getInt("crate");
                        if (label_v[count] != null) {
                            //label_v[count].setText(name);
                        }
                        table_v[crate].getItems().add(new HVStatusProperty(obj.getIndex(),
                                obj.getInt("slot"), obj.getInt("channel")));
                        if (statelabel_v[count] != null
                                && !label_m.containsKey(crate)) {
                            statelabel_v[count].setFont(Color.BLACK);
                            statelabel_v[count].set(cobj.getNode(), crate, logviewController);
                            label_m.put(crate, statelabel_v[count]);
                            count++;
                        }
                    }
                }
            }
        } else if (command.equals(NSMCommand.NSMSET)) {
            String dataname = getNSMDataProperties().get(0).getDataname();
            if (dataname.matches(msg.getNodeName())) {
                NSMData data = NSMListenerService.getData(dataname);
                networkconfigController.add(data);
                NSMConfig config = NSMListenerService.getNSMConfig();
                final SimpleDateFormat dateformat = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss");
                label_update.setText(dateformat.format(new Date()));
                if (cobj.getId() == 0) {
                    NSMListenerService.requestDBGet(config.getNsmTarget(),
                            data.getInt("configid", 0));
                    sent_listrequest = true;
                } else {
                    HVState state = HVState.get(data.getInt("state"));
                    psStateController.update(state);
                    commandButtonController.update(state);
                    powerSettingsController.getVBox().setDisable(!(radio_manual.isSelected() && state.isStable()));
                    if (cobj.hasObject("channel")) {
                        int n = 0, crate_tmp = 0;
                        boolean isoff = true;
                        boolean istransition = false;
                        boolean iserror = false;
                        int id = HVState.PEAK_S.getId();
                        int ncrate = 0, ncrate_active = 0;
                        int nchannel = 0, nchannel_active = 0;
                        for (ConfigObject obj : cobj.getObjects("channel")) {
                            int crate = obj.getInt("crate");
                            if (crate != crate_tmp) {
                                n = 0;
                                if (isoff) {
                                    id = HVState.OFF_S.getId();
                                } else if (istransition) {
                                    id = HVState.TRANSITION_TS.getId();
                                    ncrate_active++;
                                } else {
                                    ncrate_active++;
                                }
                                if (iserror) {
                                    id = HVState.ERROR_ES.getId();
                                }
                                state.copy(id);
                                label_m.get(crate_tmp).update(state);
                                isoff = true;
                                istransition = false;
                                iserror = false;
                                crate_tmp = crate;
                                ncrate++;
                            }
                            NSMData ch_data = (NSMData) data.getObject("channel", obj.getIndex());
                            HVStatusProperty status = (HVStatusProperty) table_v[crate].getItems().get(n);
                            state.copy(ch_data.getInt("state"));
                            status.setState(state);
                            if (!state.equals(HVState.OFF_S)) {
                                isoff = false;
                                if (state.isStable()){
                                    nchannel_active++;
                                    if (state.getId() < id) {
                                        id = state.getId();
                                    }
                                }
                            }
                            if (state.isError()) {
                                iserror = true;
                            }
                            if (state.isTransition()) {
                                istransition = true;
                                nchannel_active++;
                            }
                            status.setVoltage(ch_data.getFloat("voltage_mon"));
                            status.setCurrent(ch_data.getFloat("current_mon"));
                            nchannel++;
                            n++;
                        }
                        if (isoff) {
                            id = HVState.OFF_S.getId();
                        } else if (istransition) {
                            id = HVState.TRANSITION_TS.getId();
                            ncrate_active++;
                        } else {
                              ncrate_active++;
                        }
                        if (iserror) {
                            id = HVState.ERROR_ES.getId();
                        }
                        ncrate++;
                        label_modules.setText( ncrate_active + " / " + ncrate);
                        label_channels.setText( nchannel_active + " / " + nchannel);
                        state.copy(id);
                        label_m.get(crate_tmp).update(state);
                    }
                }
            }
        } else if (command.equals(HVCommand.OK)) {
            HVState state = new HVState();
            state.copy(msg.getData());
            commandButtonController.update(state);
            powerSettingsController.getVBox().setDisable(!(radio_manual.isSelected() && state.isStable()));
            if (!psStateController.update(state.getId()) && !state.isTransition()) {
                String dataname = getNSMDataProperties().get(0).getDataname();
                logviewController.add(new LogMessage(msg.getNodeName(),
                        LogLevel.INFO, "State shift "
                        + msg.getNodeName() + ">> "
                        + state.getLabel()));
                NSMListenerService.requestNSMGet(dataname, "", 0);
            }
        } else if (command.equals(HVCommand.STATE)) {
            HVState state = new HVState();
            String[] str_v = msg.getData().split(" ");
            if (str_v.length > 1) {
                if (label_m.containsKey(str_v[0])) {
                    state.copy(str_v[1]);
                    label_m.get(str_v[0]).update(state.getId());
                }
            } else {
                System.out.println("'" + msg.getData() + "' " + msg.getData().contains(" "));
            }
        }
    }

    public LogViewPaneController getLogView() {
        return logviewController;
    }

    public PowerSettingPaneController getPowerSetting() {
        return powerSettingsController;
    }

    public NetworkConfigPaneController getNetworkConfig() {
        return networkconfigController;
    }

    public HVStatusMonitorController getMonitor() {
        return hvmonitorController;
    }

    public HVParamEditorPaneController getEditor() {
        return hvparameditorController;
    }

    public String[] getNameList() {
        return namelist;
    }

    public Label getLabelConfig() {
        return label_config;
    }

    public Text getTextHVState() {
        return text_rc;
    }

    public ObservableList<NSMDataProperty> getNSMDataProperties() {
        return networkconfigController.getNSMDataProperties();
    }

    private void setStateColumn(TableColumn col) {
        col.setComparator(new Comparator<HVState>() {
            @Override
            public int compare(HVState t, HVState t1) {
                return (t.getId() > t1.getId()) ? 1 : 0;
            }
        });
        col.setCellFactory(new Callback<TableColumn<HVStatusProperty, HVState>, TableCell<HVStatusProperty, HVState>>() {
            @Override
            public TableCell<HVStatusProperty, HVState> call(TableColumn<HVStatusProperty, HVState> param) {
                return new TableCell<HVStatusProperty, HVState>() {
                    @Override
                    protected void updateItem(HVState item, boolean empty) {
                        super.updateItem(item, empty);
                        getStyleClass().removeAll("hvstate-off", "hvstate-peak",
                                "hvstate-stable", "hvstate-transition",
                                "hvstate-error");
                        if (!empty && item != null) {
                            HVState state = (HVState) item;
                            if (state.equals(HVState.OFF_S)) {
                                getStyleClass().add("hvstate-off");
                            } else if (state.equals(HVState.PEAK_S)) {
                                getStyleClass().add("hvstate-peak");
                            } else if (state.isStable()) {
                                getStyleClass().add("hvstate-stable");
                            } else if (state.isTransition()) {
                                getStyleClass().add("hvstate-transition");
                            } else if (state.isError()) {
                                getStyleClass().add("hvstate-error");
                            }
                            setText(item.getLabel());
                        } else {
                            setText(null);
                        }
                    }
                };
            }
        });
        col.setCellValueFactory(new PropertyValueFactory<>("state"));

    }

}
