/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.pscontrol;

import b2daq.hvcontrol.core.HVState;
import b2daq.logger.core.LogMessage;
import b2daq.nsm.ui.NSMDataProperty;
import b2daq.nsm.NSMListenerService;
import b2daq.nsm.ui.NSM2SocketInitDialog;
import b2daq.nsm.ui.NSMListenerGUIHandler;
import b2daq.ui.NetworkConfigPaneController;
import java.net.URL;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.fxml.JavaFXBuilderFactory;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.stage.Stage;

/**
 *
 * @author tkonno
 */
public class PowerSupplyControlGUI extends Application {

    private FXMLLoader loader;
    static private String[] arguments;

    @Override
    public void start(Stage stage) throws Exception {
        try {
            NSM2SocketInitDialog socket = NSM2SocketInitDialog.connect((arguments.length > 1 ? arguments[1] : ".pscontrol.init"),
                    null, 9090, null, 8122, "HV_GUI", "ARICH_HV",
                    new String[]{"ARICH_HV_STATUS:hv_status:1"}, "Login to Belle II Power supply control",
                    "NSM set up for power supply");
            if (socket == null) {
                return;
            }

            URL location = getClass().getResource("PowerSupplyMainPane.fxml");
            loader = new FXMLLoader();
            loader.setLocation(location);
            loader.setBuilderFactory(new JavaFXBuilderFactory());
            Parent root = (Parent) loader.load(location.openStream());
            PowerSupplyMainPaneController controller
                    = ((PowerSupplyMainPaneController) loader.getController());
            NetworkConfigPaneController netconf = controller.getNetworkConfig();
            netconf.setConfig(socket.getConfig());
            for (NSMDataProperty data : socket.getNSMConfig().getNSMDataProperties()) {
                netconf.add(data);
            }
            NSMListenerGUIHandler.get().add(controller);
            NSMListenerGUIHandler.get().add(controller.getEditor());
            NSMListenerGUIHandler.get().add(controller.getMonitor());
            NSMListenerGUIHandler.get().add(netconf);
            NSMListenerService.add(NSMListenerGUIHandler.get());
            NSMListenerService.restart();
            Scene scene = new Scene(root);
            scene.getStylesheets().add(LogMessage.getCSSPath());
            scene.getStylesheets().add(HVState.getCSSPath());
            stage.setTitle("Belle II Power Supply Control GUI");
            stage.setScene(scene);
            stage.show();
        } catch (Exception e) {
            Logger.getLogger(getClass().getName()).log(Level.SEVERE, null, e);

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
        arguments = args;
        launch(args);
    }

}
