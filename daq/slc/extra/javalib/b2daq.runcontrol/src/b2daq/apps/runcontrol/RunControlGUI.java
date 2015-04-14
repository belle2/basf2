/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import b2daq.logger.core.LogMessage;
import b2daq.nsm.NSMCommunicator;
import b2daq.nsm.NSMConfig;
import b2daq.nsm.ui.NSMRequestHandlerUI;
import b2daq.ui.LoginDialog;
import java.io.IOException;
import javafx.application.Application;
import javafx.scene.Scene;
import javafx.scene.image.Image;
import javafx.stage.Stage;
import javafx.stage.WindowEvent;

/**
 *
 * @author tkonno
 */
public class RunControlGUI extends Application {

    @Override
    public void start(Stage stage) throws IOException {
        NSMConfig conf = LoginDialog.showDialog(".runcontrol.ini", "Connect to Run control",
                "localhost", 9090, "ecl01", "b2slow2.kek.jp", 9122, "rcgui");
        RunControlMainPane root = new RunControlMainPane(conf.getNsmNode());
        NSMCommunicator.get().reconnect(conf.getHostname(), conf.getPort(), conf.getGuiNode(), conf.getNsmHost(), conf.getNsmPort());
        NSMCommunicator.get().add(NSMRequestHandlerUI.get());
        Scene scene = new Scene(root, 650, 450);
        scene.getStylesheets().add(LogMessage.getCSSPath());
        stage.setOnCloseRequest((WindowEvent t) -> {
            System.exit(0);
        });
        stage.getIcons().add(new Image(RunControlGUI.class.getResource("runcontrol.png").toExternalForm()));
        stage.setTitle("Belle II Run Control GUI");
        stage.setScene(scene);
        stage.show();
    }

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        launch(args);
    }

}
