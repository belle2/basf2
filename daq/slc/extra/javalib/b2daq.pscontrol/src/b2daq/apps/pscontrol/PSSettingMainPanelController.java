/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.pscontrol;

import b2daq.logger.core.LogMessage;
import b2daq.nsm.NSMCommand;
import b2daq.nsm.NSMListenerService;
import b2daq.nsm.NSMMessage;
import b2daq.nsm.NSMNode;
import b2daq.nsm.NSMObserver;
import b2daq.nsm.NSMVar;
import java.io.IOException;
import java.net.URL;
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.fxml.JavaFXBuilderFactory;
import javafx.scene.Parent;
import javafx.scene.control.Tab;
import javafx.scene.control.TabPane;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class PSSettingMainPanelController implements NSMObserver {

    @FXML
    TabPane tabpane;

    private NSMNode m_node;

    private final ArrayList<PSCrateSettingPanelController> crate = new ArrayList();

    @FXML
    protected void handleMonitorMenuItem() {
        MonitorOpenDialog.showDialog(tabpane.getScene(), this);
    }

    @FXML
    protected void handleAllOn() {
        for (PSCrateSettingPanelController c : crate) {
            c.allOn(true);
        }
    }

    @FXML
    protected void handleAllOff() {
        for (PSCrateSettingPanelController c : crate) {
            c.allOn(false);
        }
    }

    public void requestUpdate() {
        for (PSCrateSettingPanelController c : crate) {
            c.requestUpdate();
        }
    }

    public void setNode(NSMNode node) {
        m_node = node;
    }

    public ArrayList<PSCrateSettingPanelController> getCrates() {
        return crate;
    }

    public PSCrateSettingPanelController getCrate(String name) {
        for (PSCrateSettingPanelController c : crate) {
            if (c.getName().equals(name)) {
                return c;
            }
        }
        return null;
    }

    @Override
    public void handleOnConnected() {
        NSMListenerService.requestVGet(m_node.getName(), "ncrates");
        /*
         Timeline timer = new Timeline(new KeyFrame(Duration.millis(3000), new EventHandler<ActionEvent>() {

         @Override
         public void handle(ActionEvent event) {
         requestUpdate();
         }

         }));
         timer.setCycleCount(Timeline.INDEFINITE);
         timer.play();
         */
        requestUpdate();
    }

    @Override
    public void handleOnReceived(NSMMessage msg) {
        NSMCommand cmd = new NSMCommand(msg.getReqName());
        if (cmd.equals(NSMCommand.VSET)) {
            NSMVar var = (NSMVar) msg.getObject();
            switch (var.getType()) {
                case NSMVar.INT:
                    if (var.getName().contains("ncrates")) {
                        try {
                            int ncrates = var.getInt();
                            for (int i = 0; i < ncrates; i++) {
                                URL location = getClass().getResource("PSCrateSettingPanel.fxml");
                                FXMLLoader loader = new FXMLLoader();
                                loader.setLocation(location);
                                loader.setBuilderFactory(new JavaFXBuilderFactory());
                                Parent root = (Parent) loader.load(location.openStream());
                                PSCrateSettingPanelController controller
                                        = ((PSCrateSettingPanelController) loader.getController());
                                Tab tab = new Tab();
                                tab.setText("crate:" + i);
                                tab.setContent(root);
                                tab.setClosable(false);
                                tabpane.getTabs().add(tab);
                                controller.setNode(m_node);
                                controller.setCrateId(i + 1);
                                controller.handleOnConnected();
                                crate.add(controller);

                            }
                        } catch (IOException ex) {
                            Logger.getLogger(PSCrateSettingPanelController.class
                                    .getName()).log(Level.SEVERE, null, ex);
                        }

                    }
                    for (PSCrateSettingPanelController c : crate) {
                        c.handleOnReceived(msg);
                    }
                    break;
                case NSMVar.FLOAT:
                    for (PSCrateSettingPanelController c : crate) {
                        c.handleOnReceived(msg);
                    }
                    break;
                case NSMVar.TEXT: {
                    int i = 1;
                    for (Tab tab : tabpane.getTabs()) {
                        if (var.getName().equals("crate[" + i + "].name")) {
                            tab.setText(var.getText());
                            crate.get(i - 1).setName(var.getText());
                            return;
                        }
                        i++;
                    }
                    for (PSCrateSettingPanelController c : crate) {
                        c.handleOnReceived(msg);
                    }
                    break;
                }
            }
        }
    }

    @Override
    public void handleOnDisConnected() {
    }

    @Override
    public void log(LogMessage log) {
    }

}
