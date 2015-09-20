import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.lang.ArrayStoreException;

public class TckDemo extends Applet
{
    public TckDemo()
    {
        System.out.println("[TckDemo] constructor is called!");
    }

    public void cleanup()
    {
    }

    public void processEvent(Event paramEvent)
    {
    }

    public void startup()
    {
        System.out.println("[TckDemo] startup is called!");
        DataInputStream0007();
        
        System.out.println("\n----------------------------------------------------\n");
        
        DataInputStream2014();
        System.out.println("[TckDemo] startup is over!");
    }
    
    private void ArrayStoreTest(){
    	System.out.println("ArrayStoreTest");
    	try{
    		Object[] x = new String[3];
    		x[0] = new Integer(0);
    		System.out.println("not throw ArrayStoreTest");
    	} catch(ArrayStoreException e){
    		System.out.println("throw ArrayStoreTest");
    	}
    }
    
    private void DataInputStream0007() {
        byte[] b = new byte[100];
        double[] values = {
                Double.MAX_VALUE, Double.MIN_VALUE, (double) 0, (double) -5.5, (double) 5.5
        };     
        System.out.println("=================DataInputStream0007===");
        for (int n = 0; n < values.length; n++) {
        	System.out.println("Now to handle " + n + " :" + values[n]);
            long bits = Double.doubleToLongBits(values[n]);
            System.out.println("long bits:" + bits);
            for (int i = 7; i >= 0; i--) {
                b[n * 8 + i] = (byte) (bits >> ((7 - i) * 8));
            }
        }
        System.out.println("===Byte array:");
        for(int i=0; i < b.length; i++){
        	if(i%8 ==0)
        		System.out.println();
        	System.out.print(b[i] + " ");        	
        }
        System.out.println();
        System.out.println();

        ByteArrayInputStream source = new ByteArrayInputStream(b);
        DataInputStream in = new DataInputStream(source);
        try {
            for (int i = 0; i < values.length; i++) {
                double result = in.readDouble();
                System.out.println("Read double(" + i + "):" + result);
                if (result != values[i]) {
                	System.out.println("Fuck readxxxxxxxxxx");
                }
            }            
        } catch (IOException e) {
        	System.out.println("Unexpected exception:" + e.getMessage());
        }
        System.out.println("===DataInputStream0007 over ===");
    }
    
    
    public void DataInputStream2014() {
        byte[] b = null;
        System.out.println("=================DataInputStream2014===");
        ByteArrayOutputStream bout = new ByteArrayOutputStream();
        DataOutputStream out = new DataOutputStream(bout);
        double[] values = {
                Double.MAX_VALUE, Double.MIN_VALUE, (double) 0, (double) -5.5, (double) 5.5
        };
        try {
            for (int i = 0; i < values.length; i++) {
                out.writeDouble(values[i]);
            }
            out.close();
            b = bout.toByteArray();
            System.out.println("===Byte 2014 array:");
            for(int i=0; i < b.length; i++){
            	if(i%8 ==0)
            		System.out.println();
            	System.out.print(b[i] + " ");        	
            }
            System.out.println();
            System.out.println();
            ByteArrayInputStream source = new ByteArrayInputStream(b);
            DataInputStream in = new DataInputStream(source);
            for (int i = 0; i < values.length; i++) {
                double result = in.readDouble();
                System.out.println("Read 2014 double(" + i + "):" + result);
                if (result != values[i]) {
                	System.out.println("Fuck 2014 readxxxxxxxxxx");
                }
            }            
        } catch (IOException e) {
        	System.out.println("Unexpected exception:" + e.getMessage());
        }
        System.out.println("===DataInputStream2014 over ===");
    }
}
