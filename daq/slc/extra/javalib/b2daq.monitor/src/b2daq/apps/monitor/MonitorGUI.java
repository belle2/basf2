/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.monitor;

import b2daq.logger.core.LogMessage;
import b2daq.nsm.NSMDataProperty;
import b2daq.nsm.NSMListenerService;
import b2daq.ui.NSM2Socket;
import b2daq.ui.NetworkConfigPaneController;
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
import javafx.stage.FileChooser.ExtensionFilter;
import javafx.stage.Stage;

/**
 *
 * @author tkonno
 */
public class MonitorGUI extends Application {

    private FXMLLoader loader;
    static private String[] arguments;

    @Override
    public void start(Stage stage) {
        try {
            FileChooser fc = new FileChooser();
            fc.setTitle("select file");
            fc.setInitialDirectory(new File(System.getProperty("user.home")));
            fc.getExtensionFilters().add(new ExtensionFilter("init", "*.init"));

            File f = fc.showOpenDialog(stage);
            System.out.println(f.getPath());
            NSM2Socket socket = NSM2Socket.connect(f.getPath(),//(arguments.length > 1 ? arguments[1] : ".runcontrol.init"),
                    null, 9090, null, 8122, "MON_GUI", "CPR5003",
                    new String[]{"CPR5003_STATUS:ronode_status:3"},
                    "Login to Belle II Monitor",
                    "NSM set up for monitoring");
            if (socket == null) {
                return;
            }
            URL location = getClass().getResource("MonitorPane.fxml");
            loader = new FXMLLoader();
            loader.setLocation(location);
            loader.setBuilderFactory(new JavaFXBuilderFactory());
            Parent root = (Parent) loader.load(location.openStream());
            MonitorPaneController controller
                    = ((MonitorPaneController) loader.getController());
            NetworkConfigPaneController netconf = controller.getNetworkConfig();
            netconf.setConfig(socket.getConfig());
            for (NSMDataProperty data : socket.getNSMConfig().getNSMDataProperties()) {
                netconf.add(data);
            }
            NSMListenerService.add(controller);
            NSMListenerService.add(netconf);
            NSMListenerService.restart();
            Scene scene = new Scene(root);
            scene.getStylesheets().add(LogMessage.getCSSPath());
            scene.getStylesheets().add(DataFlowMonitorController.class.getResource("DataFlowMonitor.css").toExternalForm());
            stage.setTitle("Belle II Monitor GUI");
            stage.getIcons().add(new Image(MonitorGUI.class.getResource("runcontrol.png").toExternalForm()));
            stage.setScene(scene);
            stage.show();
        } catch (IOException ex) {
            ex.printStackTrace();;
            Logger.getLogger(MonitorGUI.class.getName()).log(Level.SEVERE, null, ex);
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
