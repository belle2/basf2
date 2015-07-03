/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.pscontrol;

import b2daq.nsm.NSMNode;
import b2daq.nsm.NSMVSetHandler;
import b2daq.nsm.NSMVar;
import b2daq.nsm.ui.NSMRequestHandlerUI;
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
public class PSSettingMainPanelController {

    @FXML
    TabPane tabpane;

    private NSMNode m_hvnode;

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

    public NSMNode getNode()  {
        return m_hvnode;
    }
    
    public void setNode(NSMNode node) throws IOException {
        m_hvnode = node;
        NSMRequestHandlerUI.get().add(new NSMVSetHandler(true, m_hvnode.getName(), "ncrates", NSMVar.INT) {
            @Override
            public boolean handleVSet(NSMVar var) {
                int ncrates = var.getInt();
                for (int i = 0; i < ncrates; i++) {
                    try {
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
                        controller.setCrateId(i + 1);
                        controller.setNode(m_hvnode);
                        crate.add(controller);
                    } catch (IOException ex) {
                        Logger.getLogger(PSSettingMainPanelController.class.getName()).log(Level.SEVERE, null, ex);
                    }
                }
                int i = 1;
                for (Tab tab : tabpane.getTabs()) {
                    NSMRequestHandlerUI.get().add(new CrateNameHandler(m_node, "crate[" + i + "].name", tab, crate.get(i-1)));
                    i++;
                }
                return true;
            }
        });
    }

    private class CrateNameHandler extends NSMVSetHandler {

        private final Tab m_tab;
        private final PSCrateSettingPanelController m_crate;

        public CrateNameHandler(String node, String name, Tab tab, PSCrateSettingPanelController crate) {
            super(true, node, name, NSMVar.TEXT);
            m_tab = tab;
            m_crate = crate;
        }

        @Override
        public boolean handleVSet(NSMVar var) {
            m_tab.setText(var.getText());
            m_crate.setName(var.getText());
            return true;
        }

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

}
