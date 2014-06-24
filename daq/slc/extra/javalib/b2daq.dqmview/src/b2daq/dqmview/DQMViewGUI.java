/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.dqmview;

import b2daq.dqm.io.DQMListenerService;
import b2daq.hvcontrol.core.HVState;
import b2daq.logger.core.LogMessage;
import java.net.URL;
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
public class DQMViewGUI extends Application {

    private FXMLLoader loader;
    static private String[] arguments;
    private DQMListenerService service;

    @Override
    public void start(Stage stage) throws Exception {
        URL location = getClass().getResource("DQMViewGUI.fxml");
        loader = new FXMLLoader();
        loader.setLocation(location);
        loader.setBuilderFactory(new JavaFXBuilderFactory());
        Parent root = (Parent) loader.load(location.openStream());
        DQMViewGUIController controller
                = ((DQMViewGUIController) loader.getController());
        service = new DQMListenerService();
        service.init("localhost", 50100, "DQMGUI", controller);
        service.start();
        Scene scene = new Scene(root);
        scene.getStylesheets().add(LogMessage.getCSSPath());
        scene.getStylesheets().add(HVState.getCSSPath());
        scene.getStylesheets().add(DQMMainPaneController.getCSSPath());
        stage.setTitle("Belle II DQM Browser GUI");
        stage.setScene(scene);
        stage.show();
    }

    @Override
    public void stop() {
        try {
            service.close();
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
