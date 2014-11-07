/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import java.io.IOException;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
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
public class NewConfigDialog extends VBox {

    @FXML
    private Label label;
    @FXML
    private TextField input;
    @FXML
    private Button okButton;
    @FXML
    private Button cancelButton;

    private boolean isOK = false;

    private void handleCloseAction(boolean isOK) {
        this.isOK = isOK;
        label.getScene().getWindow().hide();
    }

    public String getText() {
        return (isOK) ? input.getText() : null;
    }

    private NewConfigDialog() {
        FXMLLoader fxmlLoader = new FXMLLoader(getClass().getResource("NewConfigDialog.fxml"));
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
    }

    private NewConfigDialog(String message, String prompt) {
        this();
        label.setText(label.getText() + " " + message);
        input.setText(prompt);
    }

    public static String showDialog(Scene pscene, String title,
            String message, String prompt) {
        NewConfigDialog dialog = new NewConfigDialog(message, prompt);
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
