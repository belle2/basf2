/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import b2daq.logger.core.LogMessage;
import b2daq.nsm.ui.NSMDataProperty;
import b2daq.nsm.NSMListenerService;
import b2daq.nsm.ui.NSM2SocketInitDialog;
import b2daq.nsm.ui.NSMListenerGUIHandler;
import b2daq.ui.NetworkConfigPaneController;
import java.io.File;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.application.Application;
import javafx.scene.Scene;
import javafx.scene.image.Image;
import javafx.stage.FileChooser;
import javafx.stage.FileChooser.ExtensionFilter;
import javafx.stage.Stage;
import javafx.stage.WindowEvent;

/**
 *
 * @author tkonno
 */
public class RunControlGUI extends Application {

    private RunControlMainPane mainPane;
    static private String[] arguments;

    @Override
    public void start(Stage stage) {
        try {
            FileChooser fc = new FileChooser();
            fc.setTitle("select file");
            //fc.setInitialDirectory(new File(System.getProperty("user.home")));
            fc.getExtensionFilters().add(new ExtensionFilter("init file", "*.init"));

            File f = fc.showOpenDialog(stage);
            NSM2SocketInitDialog socket = NSM2SocketInitDialog.connect(f.getPath(),
                    null, 9090, null, 8122, "RC_GUI", "ARICH_RC",
                    new String[]{"ARICH_RC_STATUS:rc_status:1"},
                    "Login to Belle II Run control",
                    "NSM set up for run control");
            if (socket == null) {
                return;
            }
            mainPane = new RunControlMainPane();
            NetworkConfigPaneController netconf = mainPane.getNetworkConfig();
            netconf.setConfig(socket.getConfig());
            for (NSMDataProperty data : socket.getNSMConfig().getNSMDataProperties()) {
                netconf.add(data);
            }
            Scene scene = new Scene(mainPane);
            scene.getStylesheets().add(LogMessage.getCSSPath());
            scene.getStylesheets().add(DataFlowMonitorController.class.getResource("DataFlowMonitor.css").toExternalForm());
            stage.setTitle("Belle II Run Control GUI");
            stage.getIcons().add(new Image(RunControlGUI.class.getResource("runcontrol.png").toExternalForm()));
            stage.setScene(scene);
            stage.setOnCloseRequest((WindowEvent t) -> {
                t.consume();
                stop();
            });
            NSMListenerGUIHandler.get().add(mainPane);
            NSMListenerGUIHandler.get().add(netconf);
            NSMListenerService.add(NSMListenerGUIHandler.get());
            NSMListenerService.restart();
            stage.show();
        } catch (Exception ex) {
            Logger.getLogger(RunControlGUI.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    @Override
    public void stop() {
        try {
            NSMListenerService.close();
            System.exit(1);
        } catch (Exception e) {

        }
    }

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        arguments = new String[args.length];
        int i = 0;
        for (String arg : args) {
            arguments[i] = arg;
            i++;
        }
        launch(args);
    }

}
