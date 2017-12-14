/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import b2daq.io.ConfigFile;
import b2daq.logger.core.LogMessage;
import b2daq.nsm.NSMCommunicator;
import b2daq.nsm.NSMConfig;
import b2daq.nsm.ui.NSMRequestHandlerUI;
import b2daq.ui.LoginDialog;
import java.io.File;
import java.io.IOException;
import javafx.application.Application;
import javafx.scene.Scene;
import javafx.scene.image.Image;
import javafx.stage.FileChooser;
import javafx.stage.Stage;
import javafx.stage.WindowEvent;

/**
 *
 * @author tkonno
 */
public class RunControlGUI extends Application {

    @Override
    public void start(Stage stage) throws IOException {
        FileChooser fc = new FileChooser();
        fc.setTitle("select file");
        fc.getExtensionFilters().add(new FileChooser.ExtensionFilter("init", "*.init"));
        File f = fc.showOpenDialog(stage);
        ConfigFile config = new ConfigFile(f.getPath());
        NSMConfig conf = LoginDialog.showDialog(f.getPath(), "Connect to Run control",
                config.getString("hostname"), config.getInt("port"), "",
                config.getString("nsmhost"), config.getInt("nsmport"), config.getString("nsmnode"));
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
