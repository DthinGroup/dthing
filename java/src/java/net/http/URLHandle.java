package java.net.http;

public class URLHandle {

	static public void parseURL(URL u, String spec, int start, int limit) {
		// These fields may receive context content if this was relative URL
		String protocol = u.getProtocol();
		String authority = u.getAuthority(); 
		String userInfo = u.getUserInfo();
		String host = u.getHost();
		int port = u.getPort();
		String path = u.getPath();
		String query = u.getQuery();
		
		// This field has already been parsed
		String ref = u.getRef();
		
		boolean isRelPath = false;
		boolean queryOnly = false;
		
		// FIX: should not assume query if opaque
		    // Strip off the query part
		if (start < limit) {
	        int queryStart = spec.indexOf('?');
	        queryOnly = queryStart == start;
	        if ((queryStart != -1) && (queryStart < limit)) {
	            query = spec.substring(queryStart+1, limit);
	            if (limit > queryStart)
	                limit = queryStart;
	            spec = spec.substring(0, queryStart);
	        }
		}
		
		int i = 0;
		// Parse the authority part if any
		if ((start <= limit - 2) && (spec.charAt(start) == '/') && (spec.charAt(start + 1) == '/')) 
		{
		    start += 2;
		    i = spec.indexOf('/', start);
	        if (i < 0) {
		        i = spec.indexOf('?', start);
				if (i < 0)
	   				i = limit;
		    }
		
			host = authority = spec.substring(start, i);
		
			int ind = authority.indexOf('@');
			if (ind != -1) {
	            userInfo = authority.substring(0, ind);
	            host = authority.substring(ind+1);
			} else {
				userInfo = null;
			}
		    if (host != null) {
				// If the host is surrounded by [ and ] then its an IPv6 
				// literal address as specified in RFC2732
				if (host.length()>0 && (host.charAt(0) == '[')) 
				{
					throw new IllegalArgumentException(
							"Invalid host: "+ host);
					/*
				    	delete ,not support IPv6
				    */
				} else {
				    ind = host.indexOf(':');
				    port = -1;
				    if (ind >= 0) 
				    {
						// port can be null according to RFC2396
						if (host.length() > (ind + 1)) {
					    	port = Integer.parseInt(host.substring(ind + 1));
						}
						host = host.substring(0, ind);
				    }
				}
			} else {
				host = "";
			}
			
			if (port < -1)
				throw new IllegalArgumentException("Invalid port number :" + port);
			start = i;
			// If the authority is defined then the path is defined by the
			// spec only; See RFC 2396 Section 5.2.4.
			if (authority != null && authority.length() > 0)
				path = "";
			} 
		
		 	if (host == null) {
		 	    host = "";
		 	}
		
		        // Parse the file path if any
			if (start < limit) 
			{
			    if (spec.charAt(start) == '/') {
					path = spec.substring(start, limit);
			    } 
			    else if (path != null && path.length() > 0) 
			    {
					isRelPath = true;
					int ind = path.lastIndexOf('/');
					String seperator = "";
					if (ind == -1 && authority != null)
				    	seperator = "/";
					path = path.substring(0, ind + 1) + seperator + spec.substring(start, limit);						
			    } 
			    else {
					String seperator = (authority != null) ? "/" : "";
					path = seperator + spec.substring(start, limit);
			    }
			} 
			else if (queryOnly && path != null) 
			{
				int ind = path.lastIndexOf('/');
				if (ind < 0)
				    ind = 0;
				path = path.substring(0, ind) + "/";
			}
			if (path == null)
			    path = "";
		
			if (isRelPath) 
			{
				// Remove embedded /./
				while ((i = path.indexOf("/./")) >= 0) {
				path = path.substring(0, i) + path.substring(i + 2);
			}
			// Remove embedded /../ if possible
			i = 0;
			while ((i = path.indexOf("/../", i)) > 0) 
			{
		        if ((limit = path.lastIndexOf('/', i - 1)) >= 0) {
			    	path = path.substring(0, limit) + path.substring(i + 3);
			    	i = 0;
		        } 
		        else 
		        {
			    	i = i + 3;
				}
			}
			
			// Remove trailing .. if possible
			while (path.endsWith("/..")) 
			{
				i = path.indexOf("/..");
				if ((limit = path.lastIndexOf('/', i - 1)) >= 0) {
				    path = path.substring(0, limit+1);
				} else {
				    break;
				}
			}
			// Remove starting .
			if (path.startsWith("./") && path.length() > 2)
		       	path = path.substring(2);
		
			// Remove trailing .
			if (path.endsWith("/."))
			path = path.substring(0, path.length() -1);
		}
		//setURL(u, protocol, host, port, authority, userInfo, path, query, ref);
		u.set(protocol, host, port, authority, userInfo, path, query, ref);
	}
	
	public static boolean sameFile(URL u1, URL u2) 
	{
        // Compare the protocols.
        if (!((u1.getProtocol() == u2.getProtocol()) ||
              (u1.getProtocol() != null &&
               u1.getProtocol().equalsIgnoreCase(u2.getProtocol()))))
            return false;

		// Compare the hosts.
		if (!hostsEqual(u1, u2))
	            return false;
	
		// Compare the files.
		if (!(u1.getFile() == u2.getFile() || (u1.getFile() != null && u1.getFile().equals(u2.getFile()))))
		    return false;
	
		// Compare the ports.
	    int port1, port2;
	    port1 = (u1.getPort() != -1) ? u1.getPort() : u1.getDefaultPort();
	    port2 = (u2.getPort() != -1) ? u2.getPort() : u2.getDefaultPort();
		if (port1 != port2)
		    return false;

        return true;
    }
	
	
	public static boolean equals(URL u1, URL u2) {
        String ref1 = u1.getRef();
        String ref2 = u2.getRef();
        return sameFile(u1, u2) && 
            (ref1 == ref2 ||
            (ref1 != null && ref1.equals(ref2)));
    }
	
	public static boolean hostsEqual(URL u1, URL u2)
	{
		//not support for now!!!
		return false;
	}
	
	public static String toExternalForm(URL u) 
	{
		// pre-compute length of StringBuffer
		int len = u.getProtocol().length() + 1;
		if (u.getAuthority() != null && u.getAuthority().length() > 0)
		    len += 2 + u.getAuthority().length();
		if (u.getPath() != null) 
		{
		    len += u.getPath().length();
		}
		if (u.getQuery() != null) {
		    len += 1 + u.getQuery().length();
		}
		if (u.getRef() != null) 
		    len += 1 + u.getRef().length();
		
		StringBuffer result = new StringBuffer(len);
		result.append(u.getProtocol());
		result.append(":");
	    if (u.getAuthority() != null && u.getAuthority().length() > 0) {
	        result.append("//");
	        result.append(u.getAuthority());
	    }
	    if (u.getPath() != null) {
	        result.append(u.getPath());
	    }
	    if (u.getQuery() != null) {
	        result.append('?');
	        result.append(u.getQuery());
	    }
		    
		if (u.getRef() != null) 
		{
		    result.append("#");
		    result.append(u.getRef());
		}
		return result.toString();
	}
}
