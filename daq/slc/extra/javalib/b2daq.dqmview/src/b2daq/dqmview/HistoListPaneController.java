/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.dqmview;

import b2daq.dqm.core.Histo;
import b2daq.dqm.core.HistoPackage;
import b2daq.dqm.core.MonObject;
import b2daq.dqm.ui.CanvasPanel;
import java.net.URL;
import java.util.ResourceBundle;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.event.Event;
import javafx.event.EventHandler;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.Node;
import javafx.scene.control.ListCell;
import javafx.scene.control.ListView;
import javafx.scene.control.ScrollPane;
import javafx.scene.control.Tab;
import javafx.scene.control.TabPane;
import javafx.scene.control.TitledPane;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.HBox;
import javafx.scene.layout.StackPane;
import javafx.scene.layout.VBox;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class HistoListPaneController implements Initializable {

    @FXML
    private TitledPane title;
    @FXML
    private ListView<Histo> list;

    private final ObservableList<Histo> listdata = FXCollections.observableArrayList();
    private DQMSidePaneController sidepane;
    private HistoPackage pack;
    private String m_dir;

    private static boolean find(String name, Node parent) {
        if (parent instanceof TabPane) {
            TabPane tpane = (TabPane) parent;
            for (Tab tab : tpane.getTabs()) {
                if (find(name, tab.getContent())) {
                    tpane.getSelectionModel().select(tab);
                    return true;
                }
            }
        } else if (parent instanceof StackPane) {
            StackPane spane = (StackPane) parent;
            for (Node node : spane.getChildren()) {
                node.setVisible(false);
            }
            for (Node node : spane.getChildren()) {
                if (find(name, node)) {
                    node.setVisible(true);
                    return true;
                }
            }
        } else if (parent instanceof GridPane) {
            GridPane gpane = (GridPane) parent;
            for (Node node : gpane.getChildren()) {
                if (find(name, node)) {
                    return true;
                }
            }
        } else if (parent instanceof VBox) {
            VBox vpane = (VBox) parent;
            for (Node node : vpane.getChildren()) {
                if (find(name, node)) {
                    return true;
                }
            }
        } else if (parent instanceof HBox) {
            HBox hpane = (HBox) parent;
            for (Node node : hpane.getChildren()) {
                if (find(name, node)) {
                    return true;
                }
            }
        } else if (parent instanceof ScrollPane) {
            ScrollPane spane = (ScrollPane) parent;
            if (find(name, spane.getContent())) {
                return true;
            }
        } else if (parent instanceof CanvasPanel) {
            CanvasPanel canvas = (CanvasPanel) parent;
            for (Histo hist : canvas.getCanvas().getHistograms()) {
                if (hist.getName().equals(name)) {
                    return true;
                }
            }
        } else {
            System.out.println(parent.getClass().getName());
        }
        return false;
    }

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        list.setCellFactory((l) -> {
            return new ListCell<Histo>() {
                @Override
                protected void updateItem(Histo item, boolean empty) {
                    super.updateItem(item, empty);
                    if (item == null || empty) {
                        setText("");
                    } else {
                        setText(item.getTitle());
                    }
                }
            };
        });

        list.setOnMouseClicked(new EventHandler() {
            @Override
            public void handle(Event event) {
                update();
            }
        });
        
    }

    public void update() {
        Histo histo = list.getSelectionModel().getSelectedItem();
        if (histo == null) {
            list.getSelectionModel().select(0);
            histo = list.getSelectionModel().getSelectedItem();
        }
        for (Node node : sidepane.getMainPanel().getPane().getChildren()) {
            node.setVisible(false);
        }
        for (Node node : sidepane.getMainPanel().getPane().getChildren()) {
            for (MonObject obj : pack.getMonObjects()) {
                Histo h = (Histo) obj;
                if (h.getName().endsWith(":diff") || h.getName().endsWith(":tmp")) {
                    continue;
                }
                String dir = pack.getDirectory(h);
                if (m_dir.equals(dir) && 
                        h.getName().equals(histo.getName())) {
                    if (find(histo.getName(), node)) {
                        sidepane.getViewMain().SetDirectory(dir);
                        node.setVisible(true);
                        return;
                    }
                }
            }
        }
    }

    public void init(HistoPackage pack, String dir, DQMSidePaneController sidepane) {
        m_dir = dir;
        title.setContent(list);
        title.setText(dir);
        this.pack = pack;
        this.sidepane = sidepane;
        for (MonObject obj : pack.getMonObjects()) {
            try {
                Histo h = (Histo) obj;
                if (!pack.getDirectory(h).matches(dir)
                        || h.getName().endsWith(":diff") || h.getName().endsWith(":tmp")) {
                    continue;
                }
                listdata.add(h);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        list.setItems(listdata);
    }

    public TitledPane getPane() {
        return title;
    }
}
