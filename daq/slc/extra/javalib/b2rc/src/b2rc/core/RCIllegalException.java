package b2rc.core;

public class RCIllegalException extends Exception {

	private static final long serialVersionUID = 1L;
	
	private String fMessage;

	public RCIllegalException() {
		fMessage = "";
	}
	
	public RCIllegalException(String msg) {
		fMessage = msg;
	}
	
	public String toString() { return fMessage; }
	
	public String getMessage() { return fMessage; }
	
}
