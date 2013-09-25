package b2daq.java.io;

import java.io.File;
import java.io.FileOutputStream;
import java.io.FileNotFoundException;

public class FileDataWriter extends DataWriter {

	public FileDataWriter() {
		super();
	}
	
	public FileDataWriter(String path) throws FileNotFoundException {
		super(new FileOutputStream(path));
	}
	
	public FileDataWriter(File file) throws FileNotFoundException {
		super(new FileOutputStream(file));
	}
	
}
