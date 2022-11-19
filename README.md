# PostgreSQL URL data type

> A valid URL as defined by Java \
> `<scheme>://<authority><path>?<query>#<fragment>`  
> `<scheme>: -mandatory`  
> `// -mandatory` in case it's not a file  
> `<authority> -mandatory`, `<authority>` is defined as `<host>:<port>`, `<host> -mandatory`, `:<port> -optional`  
> `<path> -optional`  
> `? -optional`  
> `<query> -optional`  
> `# -optional`  
> `<fragment> -optional` 

> Protocols supported by Java URL  
> http , https , ftp , file , and jar

## Constructors

### URL(varchar spec)
> Creates a URL object from the String representation, if it's a valid URL. \
> A URL without scheme will return  
> `no protocol: <host>` \
> if `<port>` is not defined \
> `unknown protocol: <host>` \
> if `<port>` is defined

### URL(varchar protocol, varchar host, int port, varchar file)
> Creates a URL object from the specified @NotNullOrEmpty protocol, host, port number, and @NotNull file.  
> in case of host is null, port will be ignored  
> in case of null protocol  
> Cannot invoke "String.equals(Object)" because "protocol" is null error will be thrown  
> in case of null file  
> Cannot invoke "String.equals(Object)" because "file" is null error will be thrown  
> in case of empty protocol  
> unknown protocol error will be thrown  
> output will be `<protocol>://<host>:<port><file>` in case host is not null  
> output will be `<protocol>:<file>` in case host is null

### URL(varchar protocol, varchar host, varchar file)
> Creates a URL from the specified @NotNullOrEmpty protocol, host, and NotNull file.  
> in case of null protocol  
> Cannot invoke "String.equals(Object)" because "protocol" is null error will be thrown  
> in case of null file  
> Cannot invoke "String.equals(Object)" because "file" is null error will be thrown  
> in case of empty protocol  
> unknown protocol error will be thrown  
> output will be `<protocol>://<host><file>` in case host is not null and not empty  
> output will be `<protocol>:<file>` in case host is null or empty

### URL(URL context, varchar spec)
> Creates a URL by parsing the given @NotNull spec within a specified context  
> in case of null context the URL, the spec part has to be a valid URL or a file or `no protocol` will be thrown  
> in case of null spec  
> Cannot invoke "String.equals(Object)" because "spec" is null error will be thrown  
> in case of empty spec  
> full context will be returned as is   
> in case of spec as query  
> context will be returned until `<query>` and what's after it, and they will be replaced by **spec**
> in case of spec as path  
> context will be returned until `<path>` and what's after it, and they will be replaced by **spec** 

## Methods

| Return type | Method | Index supported |
| ----------- | ----------- | ----------- |
| boolean | equals(URL url1, URL url2) | Y |
| varchar | getAuthority() | N |
| int | getDefaultPort() | N |
| varchar | getFile() | N |
| varchar | getHost() | N |
| varchar | getPath() | N |
| int | getPort() | N |
| varchar | getProtocol() | N |
| varchar | getQuery() | N |
| String | getRef() | N |
| String | getUserInfo() | N |
| boolean | sameFile(URL url1, URL url2) | Y |
| boolean | sameHost(URL url1, URL url2) | Y |
| varchar | toString() | N |

### boolean equals(URL url1, URL url2) 
> Compares two URLs for equality.  
> URLs are equals if  
> their `<scheme>` is the same  
> and their `<host>:<port>` is the same, or if its `:<port>` is not the shown in one and the other have the default port depend on the `<scheme>`      
> and their `<path>?<query>#<fragment>` are the same

### varchar getAuthority()
> Gets the authority part of this URL.
> returns `<host>:<port>` if `:<port>` is defined  
> else `<host>`

### int getDefaultPort()
> Gets the default port number of the protocol associated with this URL.
> returns `80` if protocol is HTTP  
> returns `443` if protocol is HTTPS  
> returns `21` if protocol is FTP  
> returns `-1` if it's file or jar 

### varchar getFile()
> Gets the file name of this URL.  
> returns `<path>?<query>` if they are defined or empty if not

### varchar getHost()
> Gets the host name of this URL, if applicable.  
> returns `<host>` unless `<scheme>` is file, then it returns empty 

### varchar getPath()
> Gets the path part of this URL.  
> returns `<path>` or empty if not defined  
> in case of file, it returns everything after `<scheme>:`

### int getPort()
> Gets the port number of this URL.  
> returns `<port>` if it defined or `-1` if not 

### varchar getProtocol()
> Gets the protocol name of this URL.  
> returns `<scheme>`

### varchar getQuery()
> Gets the query part of this URL.  
> returns `<query>` if defined or `null` if not

### String getRef()
> Gets the anchor (also known as the "reference") of this URL.  
> returns `<fragment>` if defined or `null` if not 

### String getUserInfo()
> Gets the userInfo part of this URL.  
> returns `<userInfo>` if defined as `<scheme>://<userInfo>@<website>`, else it returns `null`

### boolean sameFile(URL url1, URL url2)
> Compares two URLs, excluding the fragment component.  
> returns `true` if this URL and the other argument are equal without taking the fragment component into consideration

### boolean sameHost(URL url1, URL url2)
> Compares the hosts of two URLs.  
> returns `<true>` if `<host>` argument is equal in both URLs

### varchar toString()
> Constructs a string representation of this URL  
> returns the whole URL as string