# PostgreSQL URL data type


This datatype is internally a *text pointer*. The functions and assumptions were defined following the documentation of the Java URL library.


## Assumptions

1. None of the constructor attributes can be NULL. In this case an error is thrown. This is defined as "INPUT must not be NULL <name_parameter>
2. The url follows the following format:
> `<protocol>://<userinfo>@<host>:<port>/<path>?<query>#<fragment>` 



> A valid URL as defined by Java \
> `<protocol>://<userinfo>@<host>:<port>/<path>?<query>#<fragment>` 
> `<protocol>://<authority>/<path>?<query>#<fragment>` 

> `<protocol>: -mandatory`  
> `<userinfo>: -optional`  
> `<host>: -optional`  
> `<port>: -optional`  
> `<path>: -optional`  
> `<query>: -optional`  
> `<fragment>: -optional`  


In the case of a file: `protocol=file`
> `<protocol>://<host>/<path>`  
> `<protocol>:///<path>`  


> Protocols supported by Java URL  
> http , https , ftp , file , and jar



## Constructors

The extension supports 4 constructors, of the form:



### URL(varchar spec)
> Creates a URL object from the String representation, if it's a valid URL. \
> A URL without scheme will return  

> A valid URL as defined by Java \
> `<protocol>://<userinfo>@<host>:<port>/<path>?<query>#<fragment>` 
> `<protocol>: -mandatory`  
> `<userinfo>: -optional`  
> `<host>: -optional`  
> `<port>: -optional`  
> `<path>: -optional`  
> `<query>: -optional`  
> `<fragment>: -optional`  
> If the URL is invalid, you get an error of the form Invalid URL pattern provided.

> Example: 
`select url('https://www.ulb.be/servlet/search?page=&site=%23&l=1&RH=1538989254560&beanKey=150bfcee-1f87-11e7-a0e0-b753bedcad22&q=test#1');`
> result: 
`                                                          url                                                             
-----------------------------------------------------------------------------------------------------------------------------
 https://www.ulb.be/servlet/search?page=&site=%23&l=1&RH=1538989254560&beanKey=150bfcee-1f87-11e7-a0e0-b753bedcad22&q=test#1
(1 row) `


### URL(varchar protocol, varchar host, int port, varchar file)

> Creates a URL object from the specified @NotNullOrEmpty protocol, host, port number, and file.  
> output will be `<protocol>://<host>:<port><file>` in case host is not null  
> output will be `<protocol>:<file>` in case host is null 

> `<protocol>: -mandatory`  
> `<host>: -optional`  
> `<port>: -optional`  
> `<file>: -optional`  

> If the URL is invalid, you get an error of the form Invalid URL pattern provided. 

> Example: 
``
> result: 
``

### URL(varchar protocol, varchar host, varchar file)
 
> Creates a URL object from the specified @NotNullOrEmpty protocol, host, port number, and file.  
> output will be `<protocol>://<host><file>` in case host is not null  
> output will be `<protocol>:<file>` in case host is null 

> `<protocol>: -mandatory`  
> `<host>: -optional`  
> `<file>: -optional`  

> If the URL is invalid, you get an error of the form Invalid URL pattern provided. 

> Example: 
``
> result: 
``

### URL(URL context, varchar spec)

> Creates a URL by parsing the given @NotNull spec within a specified context  
> in case of null context the URL, the spec part has to be a valid URL or a file or `no protocol` will be thrown  
> in case of null spec  an error is thrown. This is defined as "INPUT must not be NULL spec
> in case of empty spec  
> full context will be returned as is   
> in case of spec as host  
> context will be returned until `<host>` and what's after it, and they will be replaced by **spec**
> in case of spec as query  
> context will be returned until `<query>` and what's after it, and they will be replaced by **spec**
> in case of spec as path  
> context will be returned until `<path>` and what's after it, and they will be replaced by **spec** 

> If the spec is invalid, you get an error of the form Invalid URL pattern provided. 

> Example: 
`select url(('http://www.ulb.be/en')::url, 'http://www.test.com/co/get-help-with-french');`
> result: 
`                     url                     
---------------------------------------------
 http://www.test.com/co/get-help-with-french
(1 row)`




## Methods

List of methods and its support of index.


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


## Support Methods

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