/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.dqm.ui.event;

import javafx.scene.control.Menu;

/**
 *
 * @author tkonno
 */
public class CleanableMenu extends Menu {

    public CleanableMenu() {
    
    }
    
    public CleanableMenu(String name) {
        super(name);
    }
    
    static void cleanup(Menu menu) {
        menu.hide();
        if (menu.getParentMenu() != null) {
            cleanup(menu.getParentMenu());
        } else if (menu.getParentPopup() != null) {
            menu.getParentPopup().hide();
        }
    }

    public void cleanup() {
        cleanup(this);
    }

}
