/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.monitor;

import b2daq.core.LogLevel;
import b2daq.database.ConfigObject;
import b2daq.logger.core.LogMessage;
import b2daq.nsm.NSMCommand;
import b2daq.nsm.NSMData;
import b2daq.nsm.NSMDataProperty;
import b2daq.nsm.NSMListenerService;
import b2daq.nsm.NSMMessage;
import b2daq.nsm.NSMObserver;
import b2daq.runcontrol.core.RCCommand;
import b2daq.ui.NetworkConfigPaneController;
import java.net.URL;
import java.util.Date;
import java.util.HashMap;
import java.util.ResourceBundle;
import javafx.collections.ObservableList;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Tab;
import javafx.scene.control.TabPane;

/**
 *
 * @author tkonno
 */
public class MonitorPaneController implements Initializable, NSMObserver {

    @FXML
    private NetworkConfigPaneController networkconfigController;

    private final HashMap<String, NSMObserver> flowmonitors = new HashMap<>();
    private String[] namelist = null;
    @FXML
    private TabPane tabpane_mon;

    @Override
    public void initialize(URL url, ResourceBundle rb) {
    }

    @Override
    public void handleOnConnected() {
        for (NSMDataProperty pro : getNSMDataProperties()) {
            NSMListenerService.requestNSMGet(pro.getDataname(),
                    pro.getFormat(), pro.getRevision());
        }
        networkconfigController.setState(true, true);
    }

    @Override
    public void handleOnReceived(NSMMessage msg) {
        if (msg == null) {
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
            }
        } else if (command.equals(NSMCommand.DBSET)) {
            ConfigObject cobj = NSMListenerService.getDB(msg.getNodeName());
            if (cobj != null) {
                networkconfigController.add(cobj);
            }
        } else if (command.equals(NSMCommand.NSMSET)) {
            String dataname = msg.getNodeName();
            NSMData data = NSMListenerService.getData(dataname);
            Tab tab = null;
            String nodename = msg.getNodeName().replace("_STATUS", "");
            for (Tab t : tabpane_mon.getTabs()) {
                if (data.getFormat().matches("rorc_status")
                        && t.getText().matches(nodename)) {
                    tab = t;
                    flowmonitors.get(nodename).handleOnReceived(msg);
                    break;
                }
            }
            if (tab == null) {
                if (data.getFormat().matches("rorc_status")) {
                    RORCDataFlowTableController flowmonitor = RORCDataFlowTableController.create(nodename);
                    tab = new Tab();
                    tab.setText(flowmonitor.getNodeName());
                    tab.setContent(flowmonitor.getPane());
                    tab.setClosable(false);
                    tabpane_mon.getTabs().add(tab);
                    flowmonitors.put(nodename, flowmonitor);
                } else if (data.getFormat().matches("storage_status")) {
                    RORCDataFlowTableController flowmonitor = RORCDataFlowTableController.create(nodename);
                    tab = new Tab();
                    tab.setText(flowmonitor.getNodeName());
                    tab.setContent(flowmonitor.getPane());
                    tab.setClosable(false);
                    tabpane_mon.getTabs().add(tab);
                    flowmonitors.put(nodename, flowmonitor);
                }
            }
        } else if (command.equals(RCCommand.OK)) {
        } else if (command.equals(RCCommand.STATE)) {
        }
    }

    @Override
    public void handleOnDisConnected() {
    }

    @Override
    public void log(LogMessage log) {
        System.out.println(log.getMessage());
    }

    public NetworkConfigPaneController getNetworkConfig() {
        return networkconfigController;
    }

    public String[] getNameList() {
        return namelist;
    }

    public ObservableList<NSMDataProperty> getNSMDataProperties() {
        return networkconfigController.getNSMDataProperties();
    }
}
