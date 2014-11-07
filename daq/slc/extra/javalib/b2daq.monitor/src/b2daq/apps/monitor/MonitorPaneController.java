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
import b2daq.nsm.ui.NSMDataProperty;
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
import javafx.scene.Scene;
import javafx.scene.control.Tab;
import javafx.scene.image.Image;
import javafx.stage.Stage;

/**
 *
 * @author tkonno
 */
public class MonitorPaneController implements Initializable, NSMObserver {

    @FXML
    private NetworkConfigPaneController networkconfigController;

    private final HashMap<String, NSMObserver> flowmonitors = new HashMap<>();
    private String[] namelist = null;

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
                //System.out.println(msg.getData());
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
            if ((data.getFormat().matches("rorc_status")
                    || data.getFormat().matches("storage_status")
                    || data.getFormat().matches("rfunitinfo"))
                    && flowmonitors.containsKey(nodename)) {
                flowmonitors.get(nodename).handleOnReceived(msg);
                return;
            }
            if (tab == null) {
                if (data.getFormat().matches("rorc_status")) {
                    javafx.application.Platform.runLater(new Runnable() {
                        @Override
                        public void run() {
                            Stage stage = new Stage();
                            RORCDataFlowTableController flowmonitor = RORCDataFlowTableController.create(nodename);
                            Scene scene = new Scene(flowmonitor.getPane());
                            scene.getStylesheets().add(DataFlowMonitorPane.class.getResource("DataFlowMonitor.css").toExternalForm());
                            scene.getStylesheets().add(StorageDataFlowTableController.class.getResource("StorageDataFlowTable.css").toExternalForm());
                            stage.setScene(scene);
                            flowmonitors.put(nodename, flowmonitor);
                            flowmonitor.handleOnReceived(msg);
                            stage.setTitle(nodename);
                            stage.getIcons().add(new Image(MonitorGUI.class.getResource("mon.png").toExternalForm()));
                            stage.show();
                        }
                    });
                } else if (data.getFormat().matches("storage_status")) {
                    javafx.application.Platform.runLater(new Runnable() {
                        @Override
                        public void run() {
                            Stage stage = new Stage();
                            StorageDataFlowTableController flowmonitor = StorageDataFlowTableController.create(nodename);
                            Scene scene = new Scene(flowmonitor.getPane());
                            scene.getStylesheets().add(DataFlowMonitorPane.class.getResource("DataFlowMonitor.css").toExternalForm());
                            scene.getStylesheets().add(StorageDataFlowTableController.class.getResource("StorageDataFlowTable.css").toExternalForm());
                            stage.setScene(scene);
                            flowmonitors.put(nodename, flowmonitor);
                            flowmonitor.handleOnReceived(msg);
                            stage.setTitle(nodename);
                            stage.getIcons().add(new Image(MonitorGUI.class.getResource("mon.png").toExternalForm()));
                            stage.show();
                        }
                    });
                } else if (data.getFormat().matches("rfunitinfo")) {
                    javafx.application.Platform.runLater(new Runnable() {
                        @Override
                        public void run() {
                            Stage stage = new Stage();
                            HLTDataFlowTableController flowmonitor = HLTDataFlowTableController.create(nodename);
                            Scene scene = new Scene(flowmonitor.getPane());
                            scene.getStylesheets().add(DataFlowMonitorPane.class.getResource("DataFlowMonitor.css").toExternalForm());
                            scene.getStylesheets().add(StorageDataFlowTableController.class.getResource("StorageDataFlowTable.css").toExternalForm());
                            stage.setScene(scene);
                            flowmonitors.put(nodename, flowmonitor);
                            flowmonitor.handleOnReceived(msg);
                            stage.setTitle(nodename);
                            stage.getIcons().add(new Image(MonitorGUI.class.getResource("mon.png").toExternalForm()));
                            stage.show();
                        }
                    });
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
