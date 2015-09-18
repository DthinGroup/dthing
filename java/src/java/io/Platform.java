package java.io;

public class Platform {	
	public static final int PLAT_WIN = 0x01;
	public static final int PLAT_UNIX = 0x02;
	
	private static final int platform = PLAT_UNIX;
	
	public static int getPlatform(){
		return platform;
	}
}
