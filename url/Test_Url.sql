Drop EXTENSION url cascade;
CREATE EXTENSION url;
Create table testurl(Name url);
Insert into testurl values(url('https://www.ulb.be/servlet/search?page=&site=%23&l=1&RH=1538989254560&beanKey=150bfcee-1f87-11e7-a0e0-b753bedcad22&q=test#1'));
Insert into testurl values(url('www.ulb.be/en'));
Insert into testurl values(url('http://www.ulb.be:8080/en'));
Insert into testurl values(url('https://www.ulb.be/en'));
Insert into testurl values(url(''));
Insert into testurl values(url('https://www.ulb.be/en'));
Insert into testurl values(url('https://www.postgresqltutorial.com/postgresql-administration/postgresql-show-tables/#:~:text=Use%20the%20%5Cdt%20or%20%5Cdt%2B,pg_tables%20catalog.'));



--###URL(varchar spec)
Insert into testurl values('https://www.ulb.be/servlet/search?page=&site=%23&l=1&RH=1538989254560&beanKey=150bfcee-1f87-11e7-a0e0-b753bedcad22&q=test#1'); --Correct format: <scheme>://<authority><path>?<query>#<fragment>
Insert into testurl values('www.ulb.be/en'); --Scheme missing  #Protocol mandatory
Insert into testurl values('http://www.ulb.be:8080/en'); --test port
Insert into testurl values('https://www.ulb.be/en'); -- test get default port
Insert into testurl values(0); --testing null vlaue


--###URL(varchar protocol, varchar host, int port, varchar file)
Insert into testurl values('https','www.ulb.be','443','enrolment' ); --Creates a URL object from the specified @NotNullOrEmpty protocol, host, port number, and @NotNull file.
Insert into testurl values('https',null,'443', 'enrolment' ); --Host is null -> port will be ignored
Insert into testurl values(null,'www.ulb.be','443','enrolment' ); --Protocol is null -> annot invoke "String.equals(Object)" because "protocol" is null error will be thrown 
Insert into testurl values('https','www.ulb.be','443',null ); -- null file -> annot invoke "String.equals(Object)" because "file" is null error will be thrown  
Insert into testurl values('','www.ulb.be','443','enrolment' ); --in case of empty protocol -> unknown protocol error will be thro
--output will be `<protocol>://<host>:<port><file>` in case host is not null  
--output will be `<protocol>:<file>` in case host is null



--###URL(varchar protocol, varchar host, varchar file)
Insert into testurl values('https','www.ulb.be','enrolment' );--Creates a URL from the specified @NotNullOrEmpty protocol, host, and NotNull file
Insert into testurl values(null,'www.ulb.be','enrolment' ); --Protocol is null -> Cannot invoke "String.equals(Object)" because "protocol" is null error will be thrown  
Insert into testurl values('https','www.ulb.be',null ) ;--File is nulll -> Cannot invoke "String.equals(Object)" because "file" is null error will be thrown
Insert into testurl values('','www.ulb.be','enrolment' ); --Protocol is empty -> unknown protocol error will be thrown 
--output will be `<protocol>://<host><file>` in case host is not null and not empty  
--output will be `<protocol>:<file>` in case host is null or empty

--###URL(URL context, varchar spec)
insert into testurl values( url(url('http://www.ulb.be/en')::url, 'http://www.test.com/co/get-help-with-french'));
insert into testurl values( url(url('http://www.ulb.be/en')::url, '/co/get-help-with-french'));
Insert into testurl values('www.ulb.be/en','en/get-help-with-french'); --No protocol specified  ->n case of null context the URL, the spec part has to be a valid URL or a file or `no protocol` will be thrown 
;-- No protocol in the context, since the pec part has a valid URL NO ERROR should be thrown
 --Spec is null -> Cannot invoke "String.equals(Object)" because "spec" is null error will be thrown
 --Spec is empty -> full context will be returned as is
insert into testurl values( url(url('http://www.ulb.be/en')::url, '/?page_id=2620'));
--in case of spec as query  
--context will be returned until `<query>` and what's after it, and they will be replaced by **spec**
insert into testurl values( url(url('http://www.ulb.be/en/?page_id=10000')::url, '/?page_id=2620'));
insert into testurl values( url(url('http://www.ulb.be/en/?page_id=10000')::url, '/?page_id=20000'));
insert into testurl values( url(url('http://www.ulb.be/en/?page_id=10000')::url, ''));


--boolean equals(URL url1, URL url2) Compares two URLs for equality. This operation must be index-supported.
Select * from testurl
where equals (url('http://www.ulb.be/en'), url('http://www.ulb.be/en'));
--varchar getAuthority() Gets the authority part of this URL. 
Select get_Authority(url('https://www.ulb.be/servlet/search?page=&site=%23&l=1&RH=1538989254560&beanKey=150bfcee-1f87-11e7-a0e0-b753bedcad22&q=test#1'));
--int getDefaultPort() Gets the default port number of the protocol associated with this URL.
Select get_Default_Port(url('https://www.ulb.be/servlet/search?page=&site=%23&l=1&RH=1538989254560&beanKey=150bfcee-1f87-11e7-a0e0-b753bedcad22&q=test#1')); 

--varchar getFile() Gets the file name of this URL. 
Select get_File(url('https://www.ulb.be/servlet/search?page=&site=%23&l=1&RH=1538989254560&beanKey=150bfcee-1f87-11e7-a0e0-b753bedcad22&q=test#1'));
--varchar getHost() Gets the host name of this URL, if applicable. 
Select get_Host(url('https://www.ulb.be/servlet/search?page=&site=%23&l=1&RH=1538989254560&beanKey=150bfcee-1f87-11e7-a0e0-b753bedcad22&q=test#1'));
--varchar getPath() Gets the path part of this URL.
 Select get_Path(url('https://www.ulb.be/servlet//-search?page=&site=%23&l=1&RH=1538989254560&beanKey=150bfcee-1f87-11e7-a0e0-b753bedcad22&q=test#1'));
--int getPort() Gets the port number of this URL. 
 Select get_Port(url('https://www.ulb.be:8080/servlet/search?page=&site=%23&l=1&RH=1538989254560&beanKey=150bfcee-1f87-11e7-a0e0-b753bedcad22&q=test#1'));
--varchar getProtocol() Gets the protocol name of this URL.
 Select get_Protocol(url('http://www.ulb.be/servlet/search?page=&site=%23&l=1&RH=1538989254560&beanKey=150bfcee-1f87-11e7-a0e0-b753bedcad22&q=test#1'));
--varchar getQuery() Gets the query part of this URL. 
 Select get_Query(url('https://www.ulb.be/servlet/search?page=&site=%23&l=1&RH=1538989254560&beanKey=150bfcee-1f87-11e7-a0e0-b753bedcad22&q=test#1'));
--String getRef() Gets the anchor (also known as the "reference") of this URL. 
Select get_Ref(url('https://blog.apastyle.org/apastyle/2011/07/punctuating-the-reference-list-entry.html')); 
--String getUserInfo() Gets the userInfo part of this URL. 
Select get_User_Info(url('https://www.ulb.be/servlet/search?page=&site=%23&l=1&RH=1538989254560&beanKey=150bfcee-1f87-11e7-a0e0-b753bedcad22&q=test#1'));
--boolean sameFile(URL url1, URL url2) Compares two URLs, excluding the fragment component. 
Select * from testurl
where equals(url('https://www.ulb.be/servlet/search?page=&site=%23&l=1&RH=1538989254560&beanKey=150bfcee-1f87-11e7-a0e0-b753bedcad22&q=test#1'), url('https://www.ulb.be/servlet/search?page=&site=%23&l=1&RH=1538989254560&beanKey=150bfcee-1f87-11e7-a0e0-b753bedcad22&q=test#1'));
--This operation must be index supported. boolean 
--sameHost(URL url1, URL url2) Compares the hosts of two URLs. This operation must be index-supported. 
where equals (url('https://www.ulb.be/servlet/search?page=&site=%23&l=1&RH=1538989254560&beanKey=150bfcee-1f87-11e7-a0e0-b753bedcad22&q=test#1'), url('https://www.ulb.be/servlet/search?page=&site=%23&l=1&RH=1538989254560&beanKey=150bfcee-1f87-11e7-a0e0-b753bedcad22&q=test#1'));
--varchar toString() Constructs a string representation of this URL
Select toString(url('https://www.ulb.be/servlet/search?page=&site=%23&l=1&RH=1538989254560&beanKey=150bfcee-1f87-11e7-a0e0-b753bedcad22&q=test#1'));
from testurl;

--Drop table testurl;