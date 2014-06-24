/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.dqmview;

import b2daq.dqm.core.Histo;
import b2daq.dqm.core.HistoPackage;
import b2daq.dqm.core.MonObject;
import java.net.URL;
import java.util.ResourceBundle;
import javafx.event.Event;
import javafx.event.EventHandler;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.TitledPane;
import javafx.scene.control.TreeItem;
import javafx.scene.control.TreeView;
import javafx.scene.input.MouseEvent;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class HistoListPaneController implements Initializable {

    @FXML
    private TitledPane title;
    @FXML
    private TreeView list;

    private DQMSidePaneController sidepane;
    private HistoPackage pack;

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        // TODO
    }

    public void init(HistoPackage pack, DQMSidePaneController sidepane) {
        title.setText(pack.getName());
        this.pack = pack;
        this.sidepane = sidepane;
        TreeItem<String> rootItem = new TreeItem<>(pack.getName());
        for (MonObject obj : pack.getMonObjects()) {
            try {
                Histo h = (Histo) obj;
                if (h.getName().endsWith(":diff") || h.getName().endsWith(":tmp")) {
                    continue;
                }
                //TreeItem<String> item = new TreeItem(h.getName());
                TreeItem<String> item = new TreeItem(h.getTitle());
                item.addEventHandler(MouseEvent.MOUSE_CLICKED, new EventHandler() {
                    @Override
                    public void handle(Event event) {
                    }
                });
                rootItem.getChildren().add(item);
            } catch (Exception e) {
                //	e.printStackTrace();
            }
        }
        rootItem.setGraphic(null);
        list.setRoot(rootItem);
    }

    public TitledPane getPane() {
        return title;
    }
}
