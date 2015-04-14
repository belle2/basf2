/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.pscontrol;

import b2daq.logger.core.LogMessage;
import b2daq.nsm.NSMCommunicator;
import b2daq.nsm.NSMConfig;
import b2daq.nsm.NSMNode;
import b2daq.nsm.ui.NSMRequestHandlerUI;
import b2daq.ui.LoginDialog;
import java.net.URL;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.fxml.JavaFXBuilderFactory;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.image.Image;
import javafx.stage.Stage;
import javafx.stage.WindowEvent;

/**
 *
 * @author tkonno
 */
public class PowerSupplyControlGUI extends Application {

    @Override
    public void start(Stage stage) throws Exception {
        try {
            NSMConfig conf = LoginDialog.showDialog(".pscontrol.ini", "Connect to Run control",
                    "localhost", 9090, "cdc_hv", "b2slow2.kek.jp", 9122, "hvgui");
            URL location = getClass().getResource("PSSettingMainPanel.fxml");
            FXMLLoader loader = new FXMLLoader();
            loader.setLocation(location);
            loader.setBuilderFactory(new JavaFXBuilderFactory());
            Parent root = (Parent) loader.load(location.openStream());
            PSSettingMainPanelController con = (PSSettingMainPanelController) loader.getController();
            con.setNode(new NSMNode(conf.getNsmNode()));
            NSMCommunicator.get().reconnect(conf.getHostname(), conf.getPort(), conf.getGuiNode(), conf.getNsmHost(), conf.getNsmPort());
            NSMCommunicator.get().add(NSMRequestHandlerUI.get());
            Scene scene = new Scene(root, 1100, 450);
            scene.getStylesheets().add(LogMessage.getCSSPath());
            stage.setOnCloseRequest((WindowEvent t) -> {
                System.exit(0);
            });
            stage.getIcons().add(new Image(PowerSupplyControlGUI.class.getResource("runcontrol.png").toExternalForm()));
            stage.setTitle("Belle II Power Supply Control GUI");
            stage.setScene(scene);
            stage.show();
        } catch (Exception e) {
            Logger.getLogger(getClass().getName()).log(Level.SEVERE, null, e);

        }
    }

    public static void main(String[] args) {
        launch(args);
    }

}
