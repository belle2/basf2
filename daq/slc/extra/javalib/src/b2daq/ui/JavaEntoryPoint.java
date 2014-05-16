package b2daq.ui;

import java.awt.Font;
import java.util.Enumeration;

import javax.swing.UIManager;


public abstract class JavaEntoryPoint extends Thread implements EntoryPoint {

	public JavaEntoryPoint() {
		 try {
			UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
			final Font font = new Font("Arial",Font.PLAIN,14);
			Enumeration<Object> enumeration = UIManager.getDefaults().keys();
			while(enumeration.hasMoreElements()){
			    Object key = enumeration.nextElement();
			    Object value = UIManager.get(key);
			    if (value instanceof java.awt.Font){
			    	UIManager.put(key.toString(), font);
			    }
			} 
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
}
