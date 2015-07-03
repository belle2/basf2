/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.monitor;

import java.net.URL;
import java.util.ResourceBundle;
import javafx.fxml.Initializable;

/**
 *
 * @author tkonno
 */
public class MonitorPaneController implements Initializable {

    @Override
    public void initialize(URL url, ResourceBundle rb) {
    }
/*
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
        } else if (command.equals(NSMCommand.DBLISTSET)) {
            if (msg.getNParams() > 0 && msg.getParam(0) > 0) {
                namelist = msg.getData().split("\n");
                //System.out.println(msg.getData());
            }
        } else if (command.equals(NSMCommand.DBSET)) {
            ConfigObject cobj = NSMListenerService.getDB(msg.getNodeName());
            if (cobj != null) {
                networkconfigController.add(cobj);
            }
        } else if (command.equals(NSMCommand.DATASET)) {
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
        }
    }
*/
}
