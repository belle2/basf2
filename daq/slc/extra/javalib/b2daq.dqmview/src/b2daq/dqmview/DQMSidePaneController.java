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
import java.util.HashMap;
import java.util.ResourceBundle;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.fxml.FXMLLoader;
import javafx.fxml.Initializable;
import javafx.scene.control.Accordion;
import javafx.scene.control.TitledPane;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class DQMSidePaneController implements Initializable {

    private Accordion accordion;
    private DQMMainPaneController mainpane;
    private final DQMViewGUIController m_ctl;
    private ArrayList<HistoListPaneController> m_ctls = new ArrayList<>();
    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        // TODO
    }

    public DQMSidePaneController(DQMViewGUIController ctl) {
        m_ctl = ctl;
    }
    
    public DQMViewGUIController getViewMain() {
        return m_ctl;
    }
    
    public void init(ArrayList<HistoPackage> pack_v) {
        try {
            for (HistoPackage pack : pack_v) {
                HashMap<String, TitledPane> pane_v = new HashMap<>();
                for (int i = 0; i < pack.getNHistos(); i++) {
                    String dir = pack.getDirectory(pack.getHisto(i));
                    if (!pane_v.containsKey(dir) && !dir.isEmpty()) {
                        System.out.println("dir='"+dir+"'");
                        FXMLLoader loader = new FXMLLoader(HistoListPaneController.class.getResource("HistoListPane.fxml"));
                        loader.load();
                        //Parent root = loader.getRoot();
                        HistoListPaneController controller = loader.getController();
                        controller.init(pack, dir, this);
                        pane_v.put(dir, controller.getPane());
                        accordion.getPanes().add(controller.getPane());
                        m_ctls.add(controller);
                    }
                }
            }
        } catch (IOException ex) {
            Logger.getLogger(DQMSidePaneController.class.getName()).log(Level.SEVERE, null, ex);
        }
        TitledPane titlepane = ((TitledPane)accordion.getPanes().get(0));
        titlepane.setExpanded(true);
        m_ctls.get(0).update();
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
