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
import java.util.Date;
import java.util.ResourceBundle;
import javafx.collections.ObservableList;
import javafx.event.Event;
import javafx.event.EventHandler;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Label;
import javafx.scene.control.RadioButton;

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
    private PowerSupplyStatusPaneController psStatusController;
    @FXML
    private LogViewPaneController logviewController;
    @FXML
    private NetworkConfigPaneController networkconfigController;
    @FXML
    private HVParamEditorPaneController hvparameditorController;

    @FXML
    private RadioButton radio_programatic;
    @FXML
    private RadioButton radio_manual;
    
    private final ConfigObject cobj = new ConfigObject();

    private boolean sent_listrequest = false;
    private String[] namelist = null;

    @Override
    public void initialize(URL url, ResourceBundle rb) {
        logviewController.add(new LogMessage("LOCAL", LogLevel.INFO, "GUI opened"));
        radio_programatic.setOnAction(new EventHandler() {
            @Override
            public void handle(Event event) {
                powerSettingsController.getVBox().setDisable(true);
            }
        });
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
//        NSMDataProperty pro = getNSMDataProperties().get(0);
//        NSMListenerService.requestNSMGet(pro.getDataname(),
//                pro.getFormat(), pro.getRevision());
        NSMConfig config = NSMListenerService.getNSMConfig();
        commandButtonController.set(this);
        psStatusController.setConnected(config);
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
            psStatusController.update(0);
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
                commandButtonController.clearStack();
            }
        } else if (command.equals(NSMCommand.DBSET)) {
            msg.getData(cobj);
            if (cobj.getId() > 0 && sent_listrequest) {
                sent_listrequest = false;
                networkconfigController.add(cobj);
                powerSettingsController.update(cobj);
                psStatusController.setDB(cobj, logviewController);
            }
        } else if (command.equals(NSMCommand.NSMSET)) {
            String dataname = getNSMDataProperties().get(0).getDataname();
            if (dataname.matches(msg.getNodeName())) {
                NSMData data = NSMListenerService.getData(dataname);
                networkconfigController.add(data);
                NSMConfig config = NSMListenerService.getNSMConfig();
                if (cobj.getId() == 0) {
                    NSMListenerService.requestDBGet(config.getNsmTarget(),
                            data.getInt("configid", 0));
                    sent_listrequest = true;
                } else {
                    HVState state = HVState.get(data.getInt("state"));
                    commandButtonController.update(state);
                    powerSettingsController.getVBox().setDisable(!(radio_manual.isSelected() && state.isStable()));
                    psStatusController.update(state, cobj, data);
                }
            }
        } else if (command.equals(HVCommand.OK)) {
            HVState state = new HVState();
            state.copy(msg.getData());
            commandButtonController.update(state);
            powerSettingsController.getVBox().setDisable(!(radio_manual.isSelected() && state.isStable()));
            if (!psStatusController.update(state.getId()) && !state.isTransition()) {
                String dataname = getNSMDataProperties().get(0).getDataname();
                logviewController.add(new LogMessage(msg.getNodeName(),
                        LogLevel.INFO, "State shift "
                        + msg.getNodeName() + ">> "
                        + state.getLabel()));
                NSMListenerService.requestNSMGet(dataname, "", 0);
            }
        } else if (command.equals(HVCommand.STATE)) {
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

    public ObservableList<NSMDataProperty> getNSMDataProperties() {
        return networkconfigController.getNSMDataProperties();
    }

    public String[] getNameList() {
        return namelist;
    }

    public ConfigObject getDB() {
        return cobj;
    }

    public Label getLabelConfig() {
        return psStatusController.getLabelConfig();
    }

}
