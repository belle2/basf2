/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.dqmview;

import b2daq.dqm.core.HistoPackage;
import java.io.IOException;
import java.net.URL;
import java.util.ArrayList;
import java.util.ResourceBundle;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.fxml.FXMLLoader;
import javafx.fxml.Initializable;
import javafx.scene.Parent;
import javafx.scene.control.Accordion;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class DQMSidePaneController implements Initializable {

    private Accordion accordion;
    private DQMMainPaneController mainpane;

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        // TODO
    }

    public void init(ArrayList<HistoPackage> pack_v) {
        try {
            for (HistoPackage pack : pack_v) {
                FXMLLoader loader = new FXMLLoader(HistoListPaneController.class.getResource("HistoListPane.fxml"));
                loader.load();
                Parent root = loader.getRoot();
                HistoListPaneController controller = loader.getController();
                controller.init(pack, this);
                accordion.getPanes().add(controller.getPane());
            }
        } catch (IOException ex) {
            Logger.getLogger(DQMSidePaneController.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    public void setMainPanel(DQMMainPaneController main_panel) {
        mainpane = main_panel;
    }

    public DQMMainPaneController getMainPanel() {
        return mainpane;
    }

    public void setPane(Accordion pane) {
        accordion = pane;
    }

    public Accordion getPane() {
        return accordion;
    }

    void update() {
    }
    
}
