/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.monitor;

import b2daq.io.ConfigFile;
import b2daq.logger.core.LogMessage;
import b2daq.nsm.NSMCommunicator;
import b2daq.nsm.NSMConfig;
import b2daq.nsm.ui.NSMRequestHandlerUI;
import b2daq.ui.LoginDialog;
import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.fxml.JavaFXBuilderFactory;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.image.Image;
import javafx.stage.FileChooser;
import javafx.stage.Stage;
import javafx.stage.WindowEvent;

/**
 *
 * @author tkonno
 */
public class MonitorGUI extends Application {

    @Override
    public void start(Stage stage) throws Exception {
        try {
            FileChooser fc = new FileChooser();
            fc.setTitle("select file");
            fc.getExtensionFilters().add(new FileChooser.ExtensionFilter("init", "*.init"));
            File f = fc.showOpenDialog(stage);
            ConfigFile config = new ConfigFile(f.getPath());
            NSMConfig conf = LoginDialog.showDialog(".monitor.ini", "Connect to Monitor",
                    config.getString("hostname"), config.getInt("port"), "",
                    config.getString("nsmhost"), config.getInt("nsmport"), config.getString("nsmnode"));
            URL location = getClass().getResource("MonitorPane.fxml");
            FXMLLoader loader = new FXMLLoader();
            loader.setLocation(location);
            loader.setBuilderFactory(new JavaFXBuilderFactory());
            Parent root = (Parent) loader.load(location.openStream());
            MonitorPaneController controller = ((MonitorPaneController) loader.getController());
            //controller.setNode(new NSMNode(conf.getNsmNode()));
            NSMCommunicator.get().reconnect(conf.getHostname(), conf.getPort(), 
                    conf.getGuiNode(), conf.getNsmHost(), conf.getNsmPort());
            NSMCommunicator.get().add(NSMRequestHandlerUI.get());
            for (int i = 0; i < 100; i++) {
                String label = String.format("data%02d", i);
                if (config.hasKey(label + ".dataname")) {
                    String name = config.getString(label + ".dataname");
                    String format = config.getString(label + ".format");
                    if (format.matches("rorc_status")) {
                        javafx.application.Platform.runLater(new Runnable() {
                            @Override
                            public void run() {
                                Stage stage = new Stage();
                                RORCDataFlowTableController flowmonitor = RORCDataFlowTableController.create(name);
                                Scene scene = new Scene(flowmonitor.getPane());
                                scene.getStylesheets().add(DataFlowMonitorPane.class.getResource("DataFlowMonitor.css").toExternalForm());
                                scene.getStylesheets().add(StorageDataFlowTableController.class.getResource("StorageDataFlowTable.css").toExternalForm());
                                stage.setScene(scene);
                                flowmonitor.init(name);
                                stage.setTitle(name);
                                stage.getIcons().add(new Image(MonitorGUI.class.getResource("mon.png").toExternalForm()));
                                stage.show();
                            }
                        });
                    } else if (format.matches("storage_status")) {
                        javafx.application.Platform.runLater(new Runnable() {
                            @Override
                            public void run() {
                                Stage stage = new Stage();
                                StorageDataFlowTableController flowmonitor = StorageDataFlowTableController.create(name);
                                Scene scene = new Scene(flowmonitor.getPane());
                                scene.getStylesheets().add(DataFlowMonitorPane.class.getResource("DataFlowMonitor.css").toExternalForm());
                                scene.getStylesheets().add(StorageDataFlowTableController.class.getResource("StorageDataFlowTable.css").toExternalForm());
                                stage.setScene(scene);
                                flowmonitor.init(name);
                                stage.setTitle(name);
                                stage.getIcons().add(new Image(MonitorGUI.class.getResource("mon.png").toExternalForm()));
                                stage.show();
                            }
                        });
                    } else if (format.matches("rfunitinfo")) {
                        javafx.application.Platform.runLater(new Runnable() {
                            @Override
                            public void run() {
                                Stage stage = new Stage();
                                HLTDataFlowTableController flowmonitor = HLTDataFlowTableController.create(name);
                                Scene scene = new Scene(flowmonitor.getPane());
                                scene.getStylesheets().add(DataFlowMonitorPane.class.getResource("DataFlowMonitor.css").toExternalForm());
                                scene.getStylesheets().add(StorageDataFlowTableController.class.getResource("StorageDataFlowTable.css").toExternalForm());
                                stage.setScene(scene);
                                flowmonitor.init(name);
                                stage.setTitle(name);
                                stage.getIcons().add(new Image(MonitorGUI.class.getResource("mon.png").toExternalForm()));
                                stage.show();
                            }
                        });
                    }

                } else {
                    break;
                }
            }
            Scene scene = new Scene(root);
            scene.getStylesheets().add(LogMessage.getCSSPath());
            scene.getStylesheets().add(DataFlowMonitorPane.class.getResource("DataFlowMonitor.css").toExternalForm());
            scene.getStylesheets().add(StorageDataFlowTableController.class.getResource("StorageDataFlowTable.css").toExternalForm());
            stage.setTitle("Belle II Monitor GUI");
            stage.getIcons().add(new Image(MonitorGUI.class.getResource("mon.png").toExternalForm()));
            stage.setScene(scene);
            stage.setOnCloseRequest((WindowEvent t) -> {
                System.exit(0);
            });
            stage.show();
        } catch (IOException ex) {
            ex.printStackTrace();
            Logger.getLogger(MonitorGUI.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        launch(args);
    }

}
