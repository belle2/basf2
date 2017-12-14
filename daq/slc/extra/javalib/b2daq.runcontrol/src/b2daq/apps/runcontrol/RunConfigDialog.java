/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import b2daq.io.ConfigTree;
import java.io.IOException;
import java.util.HashMap;
import javafx.beans.value.ChangeListener;
import javafx.beans.value.ObservableValue;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.control.TreeItem;
import javafx.scene.control.TreeView;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;
import javafx.scene.layout.VBox;
import javafx.stage.Modality;
import javafx.stage.Stage;
import javafx.stage.StageStyle;

/**
 *
 * @author tkonno
 */
public class RunConfigDialog extends VBox {

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
    private HashMap<TreeItem, String> map = new HashMap<>();

    private void handleCloseAction(boolean isOK) {
        this.isOK = isOK;
        label.getScene().getWindow().hide();
    }

    public void set(String message, String prompt, String... namelist) {
        label.setText(message);
        input.setText(prompt);
        ConfigTree.Branch branch = new ConfigTree().createBranch(namelist);
        branch.setName("Run config list");
        tree_runtype.setRoot(createTree(branch));
    }

    private TreeItem<String> createTree(ConfigTree.Branch branch) {
        TreeItem<String> root = new TreeItem<>(branch.getName());
        map.put(root, branch.getName());
        for (ConfigTree.Leaf leaf : branch.getLeafs()) {
            if (leaf.isLeaf()) {
                TreeItem<String> item = new TreeItem<>(leaf.getName());
                map.put(item, leaf.getLabel());
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

    private RunConfigDialog() {
        FXMLLoader fxmlLoader = new FXMLLoader(getClass().getResource("RunConfigDialog.fxml"));
        fxmlLoader.setRoot(this);
        fxmlLoader.setController(this);
        try {
            fxmlLoader.load();
        } catch (IOException exception) {
            throw new RuntimeException(exception);
        }
        setOnKeyPressed(new EventHandler<KeyEvent>() {
            @Override
            public void handle(KeyEvent ke) {
                if (ke.getCode().equals(KeyCode.ENTER)) {
                    handleCloseAction(true);
                }
            }
        });
        okButton.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent e) {
                handleCloseAction(true);
            }
        });
        okButton.setOnKeyPressed(new EventHandler<KeyEvent>() {
            @Override
            public void handle(KeyEvent ke) {
                if (ke.getCode().equals(KeyCode.ENTER)) {
                    handleCloseAction(true);
                }
            }
        });
        cancelButton.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent e) {
                handleCloseAction(false);
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
                    input.setText(map.get(item));
                }
            }

        });

    }

    public static String showDialog(Scene pscene, String title,
            String message, String prompt, String... namelist) {
        RunConfigDialog dialog = new RunConfigDialog();
        dialog.set(message, prompt, namelist);
        Scene scene = new Scene(dialog);
        Stage stage = new Stage(StageStyle.UTILITY);
        stage.setScene(scene);
        if (pscene != null) {
            stage.initOwner(pscene.getWindow());
        }
        stage.initModality(Modality.WINDOW_MODAL);
        stage.setResizable(false);
        stage.setTitle(title);
        stage.showAndWait();
        return dialog.getText();
    }
}
