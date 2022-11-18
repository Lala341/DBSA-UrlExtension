# PostgreSQL URL data type

> A valid URL as defined by Java \
> `<scheme>://<authority><path>?<query>#<fragment>`  
> `<scheme> -mandatory`  
> `:// -mandatory`  
> `<authority> -mandatory`, `<authority>` is defined as `<host>:<port>`, `<host> -mandatory`, `:<port> -optional`  
> `<path> -optional`  
> `? -optional`  
> `<query> -optional`  
> `# -optional`  
> `<fragment> -optional` 
##Constructors

###URL(varchar spec)
>Creates a URL object from the String representation, if it's a valid URL. \
> A URL without scheme will return  
> `no protocol: <host>` \
> if `<port>` is not defined \
> `unknown protocol: <host>` \
> if `<port>` is defined

###URL(varchar protocol, varchar host, int port, varchar file)
>Creates a URL object from the specified @NotNullOrEmpty protocol, host, port number, and @NotNull file.  
> in case of host is null, port will be ignored  
> in case of null protocol  
> Cannot invoke "String.equals(Object)" because "protocol" is null error will be thrown  
> in case of null file  
> Cannot invoke "String.equals(Object)" because "file" is null error will be thrown  
> in case of empty protocol  
> unknown protocol error will be thrown  
> output will be `<protocol>://<host>:<port><file>` in case host is not null  
> output will be `<protocol>:<file>` in case host is null

###URL(varchar protocol, varchar host, varchar file)
>Creates a URL from the specified @NotNullOrEmpty protocol, host, and NotNull file.  
> in case of null protocol  
> Cannot invoke "String.equals(Object)" because "protocol" is null error will be thrown  
> in case of null file  
> Cannot invoke "String.equals(Object)" because "file" is null error will be thrown  
> in case of empty protocol  
> unknown protocol error will be thrown  
> output will be `<protocol>://<host><file>` in case host is not null and not empty  
> output will be `<protocol>:<file>` in case host is null or empty

###URL(URL context, varchar spec)
>Creates a URL by parsing the given @NotNull spec within a specified context  
> in case of null context the URL, the spec part has to be a valid URL or a file or `no protocol` will be thrown  
> in case of null spec  
> Cannot invoke "String.equals(Object)" because "spec" is null error will be thrown  
> in case of empty spec  
> full context will be returned as is   
> in case of spec as query  
> context will be returned until `<query>` and what's after it, and they will be replaced by **spec**
> in case of spec as path  
> context will be returned until `<path>` and what's after it, and they will be replaced by **spec** 

