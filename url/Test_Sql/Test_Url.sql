CREATE EXTENSION url
--###URL(varchar spec)
Select url(https://www.ulb.be/servlet/search?page=&site=%23&l=1&RH=1538989254560&beanKey=150bfcee-1f87-11e7-a0e0-b753bedcad22&q=test#1) --Correct format: <scheme>://<authority><path>?<query>#<fragment>
Select url(www.ulb.be/en) --Scheme missing  #Protocol mandatory
Select url(http://www.ulb.be:8080/en) --test port
Select url(https://www.ulb.be/en) -- test get default port
Select url(0) --testing null vlaue


--###URL(varchar protocol, varchar host, int port, varchar file)
Select url('https', 'www.ulb.be','443', 'enrolment' ) --Creates a URL object from the specified @NotNullOrEmpty protocol, host, port number, and @NotNull file.
Select url('https',null,'443', 'enrolment' ) --Host is null -> port will be ignored
Select url(null, 'www.ulb.be','443', 'enrolment' ) --Protocol is null -> annot invoke "String.equals(Object)" because "protocol" is null error will be thrown 
Select url('https', 'www.ulb.be','443',null ) -- null file -> annot invoke "String.equals(Object)" because "file" is null error will be thrown  
Select url('', 'www.ulb.be','443', 'enrolment' ) --in case of empty protocol -> unknown protocol error will be thro
--output will be `<protocol>://<host>:<port><file>` in case host is not null  
--output will be `<protocol>:<file>` in case host is null



--###URL(varchar protocol, varchar host, varchar file)
Select url('https', 'www.ulb.be', 'enrolment' ) --Creates a URL from the specified @NotNullOrEmpty protocol, host, and NotNull file
Select url(null, 'www.ulb.be', 'enrolment' ) --Protocol is null -> Cannot invoke "String.equals(Object)" because "protocol" is null error will be thrown  
Select url('https', 'www.ulb.be',null ) --File is nulll -> Cannot invoke "String.equals(Object)" because "file" is null error will be thrown
Select url('', 'www.ulb.be', 'enrolment' ) --Protocol is empty -> unknown protocol error will be thrown 
--output will be `<protocol>://<host><file>` in case host is not null and not empty  
--output will be `<protocol>:<file>` in case host is null or empty

--###URL(URL context, varchar spec)
Select url('http://www.ulb.be/en', 'en/get-help-with-french') --Creates a URL by parsing the given @NotNull spec within a specified context 
Select url('www.ulb.be/en', 'en/get-help-with-french') --No protocol specified  ->n case of null context the URL, the spec part has to be a valid URL or a file or `no protocol` will be thrown 
Select url('www.ulb.be/en', 'http://www.f9languages.eu/')-- No protocol in the context, since the pec part has a valid URL NO ERROR should be thrown
Select url('www.ulb.be/en',null ) --Spec is null -> Cannot invoke "String.equals(Object)" because "spec" is null error will be thrown
Select url('http://www.ulb.be/en', '') --Spec is empty -> full context will be returned as is
Select url('http://www.ulb.be/en', 'https://www.ulb.be/servlet/search?page=&site=%23&l=1&RH=1538989013462&beanKey=150bfcee-1f87-11e7-a0e0-b753bedcad22&q=French')
--in case of spec as query  
--context will be returned until `<query>` and what's after it, and they will be replaced by **spec**
Select url('http://www.ulb.be/en', '/?page_id=2620')
--in case of spec as path  
--context will be returned until `<path>` and what's after it, and they will be replaced by **spec** 
