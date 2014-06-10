/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.pscontrol;

import b2daq.io.ConfigTree;
import java.net.URL;
import java.util.ResourceBundle;
import javafx.beans.value.ChangeListener;
import javafx.beans.value.ObservableValue;
import javafx.event.EventHandler;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.control.TreeItem;
import javafx.scene.control.TreeView;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class PowerConfigDialogController implements Initializable {

    @FXML
    private Label label;
    @FXML
    private TextField input;
    @FXML
    private Button okButton;
    @FXML
    private Button cancelButton;
    @FXML
    private TreeView tree_runtype;

    private boolean isOK = false;

    @FXML
    private void handleOKButton() {
        handleCloseAction(true);
    }

    @FXML
    private void handleCancelButton() {
        handleCloseAction(false);
    }

    private void handleCloseAction(boolean isOK) {
        this.isOK = isOK;
        label.getScene().getWindow().hide();
    }

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        okButton.setOnKeyPressed(new EventHandler<KeyEvent>() {
            @Override
            public void handle(KeyEvent ke) {
                if (ke.getCode().equals(KeyCode.ENTER)) {
                    handleCloseAction(true);
                }
            }
        });
        cancelButton.setOnKeyPressed(new EventHandler<KeyEvent>() {
            @Override
            public void handle(KeyEvent ke) {
                if (ke.getCode().equals(KeyCode.ENTER)) {
                    handleCloseAction(false);
                }
            }
        });
        tree_runtype.getSelectionModel().selectedItemProperty().addListener(new ChangeListener() {
            @Override
            public void changed(ObservableValue observable,
                    Object oldValue, Object newValue) {
                TreeItem<String> item = (TreeItem<String>) newValue;
                if (item != null && item.isLeaf()) {
                    input.setText(item.getValue());
                }
            }

        });
    }

    public void set(String message, String prompt, String... namelist) {
        label.setText(message);
        input.setText(prompt);
        ConfigTree.Branch branch = new ConfigTree().createBranch(namelist);
        branch.setName("Configuration list");
        tree_runtype.setRoot(createTree(branch));
    }

    private TreeItem<String> createTree(ConfigTree.Branch branch) {
        TreeItem<String> root = new TreeItem<String>(branch.getName());
        for (ConfigTree.Leaf leaf : branch.getLeafs()) {
            if (!leaf.isLeaf()) {
                TreeItem<String> item = new TreeItem<String>(leaf.getName());
                root.getChildren().add(item);
            } else {
                root.getChildren().add(createTree((ConfigTree.Branch) leaf));
            }
        }
        return root;
    }

    public String getText() {
        return (isOK) ? input.getText() : null;
    }

}
