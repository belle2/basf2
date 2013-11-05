package b2daq.java.io;

import java.io.IOException;

public class DCSpace extends ConfigFile {

	static private DCSpace __db = null;
	static public DCSpace getDB() { 
		if ( __db == null ) __db = new DCSpace();
		return __db;
	}
	
	private DCSpace() {
		String online_path = "";
		try {
			online_path = System.getenv("DCONLINE_PATH");
		} catch (Exception e) {
			System.out.println("[DEBUG] : failed to get DCONLINE_PATH.");
		}
		if ( online_path != null ) {
			String ip_path = online_path + "/config/DCSpaceIP.config";
			String port_path = online_path + "/config/DCSpacePort.config";
			try {
				read(ip_path);
			} catch (IOException e) {
				System.out.println("[DEBUG] : failed to read SpaceIP from "+ip_path+".");
			}
			try {
				read(port_path);
			} catch (IOException e) {
				System.out.println("[DEBUG] : failed to read SpacePort from "+port_path+".");
			}
		} else {
			try {
				read(getClass().getResourceAsStream("config/DCSpaceIP.config"));
			} catch (IOException e) {
				System.out.println("[DEBUG] : failed to read SpaceIP.");
			}
			try {
				read(getClass().getResourceAsStream("config/DCSpacePort.config"));
			} catch (IOException e) {
				System.out.println("[DEBUG] : failed to read SpacePort.");
			}
		}
		
	}
	
	static public void main(String [] argv) {
		System.out.println(DCSpace.getDB().toString());
	}
	
}
