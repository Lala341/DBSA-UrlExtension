Drop EXTENSION url cascade;
CREATE EXTENSION url;
Create table testurl(Emri url);
CREATE INDEX testidx ON testurl(Emri);

--###URL(varchar spec)
Insert into testurl values(url('https://www.ulb.be/servlet/search?page=&site=%23&l=1&RH=1538989254560&beanKey=150bfcee-1f87-11e7-a0e0-b753bedcad22&q=test#1')); --Correct format: <scheme>://<authority><path>?<query>#<fragment>
Insert into testurl values(url('http://www.ulb.be:8080/en')); --test port
Insert into testurl values(url('https://www.ulb.be/en')); -- test get default port
Insert into testurl values(url('https://www.ulb.be/servlet/search?page=&site=%23&l=1&RH=1538989254560&beanKey=150bfcee-1f87-11e7-a0e0-b753bedcad22&q=test#1'));


--###URL(varchar protocol, varchar host, int port, varchar file)
Insert into testurl values(url('https','www.ulb.be',443,'enrolment')); --Creates a URL object from the specified @NotNullOrEmpty protocol, host, port number, and @NotNull file.
Insert into testurl values(url('https','www.ulb.be',8080,'enrolment/test1/324wer#1'));

--###URL(varchar protocol, varchar host, varchar file)
Insert into testurl values(url('https','www.ulb.be','enrolment' ));--Creates a URL from the specified @NotNullOrEmpty protocol, host, and NotNull file

--###URL(URL context, varchar spec)
insert into testurl values( url(url('http://www.ulb.be/en')::url, 'http://www.test.com/co/get-help-with-french'));
insert into testurl values( url(url('http://www.ulb.be/en')::url, '/co/get-help-with-french'));
insert into testurl values( url(url('http://www.ulb.be/en')::url, '/?page_id=2620'));



--boolean equals(URL url1, URL url2) Compares two URLs for equality. This operation must be index-supported.
Select * from testurl
where equals (url('http://www.ulb.be/en'), url('http://www.ulb.be/en'));
--Other way to test
Select equals (url('http://www.ulb.be/en'), url('http://www.ulb.be/en'));
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
Select get_Ref(url('https://blog.apastyle.org/apastyle/2011/07/punctuating-the-reference-list-entry.html#123')); 
--String getUserInfo() Gets the userInfo part of this URL. 
Select get_User_Info(url('https://username@apastyle.org/apastyle/2011/07/punctuating-the-reference-list-entry.html'));
--boolean sameFile(URL url1, URL url2) Compares two URLs, excluding the fragment component. 
Select same_file(url('https://www.ulb.be/servlet/search?page=&site=%23&l=1&RH=1538989254560&beanKey=150bfcee-1f87-11e7-a0e0-b753bedcad22&q=test#1'), url('https://www.ulb.be/servlet/search?page=&site=%23&l=1&RH=1538989254560&beanKey=150bfcee-1f87-11e7-a0e0-b753bedcad22&q=test#1'));
--This operation must be index supported. boolean 
--sameHost(URL url1, URL url2) Compares the hosts of two URLs. This operation must be index-supported. 
Select same_host(url('https://www.ulb.be/servlet/search?page=&site=%23&l=1&RH=1538989254560&beanKey=150bfcee-1f87-11e7-a0e0-b753bedcad22&q=test#1'), url('https://www.ulb.be/servlet/search?page=&site=%23&l=1&RH=1538989254560&beanKey=150bfcee-1f87-11e7-a0e0-b753bedcad22&q=test#1'));
--varchar toString() Constructs a string representation of this URL
Select url_to_string(url('https://www.ulb.be/servlet/search?page=&site=%23&l=1&RH=1538989254560&beanKey=150bfcee-1f87-11e7-a0e0-b753bedcad22&q=test#1'));

--Drop table testurl;

SET enable_seqscan = OFF;

Explain Select * from testurl where Emri='http://www.ulb.be/en';
Explain Select * from testurl where equals(url('http://www.ulb.be/en'),url('http://www.ulb.be/en'));
Explain Select * from testurl where same_host(url('http://www.ulb.be/en'),url('http://www.ulb.be/en'));
Explain Select * from testurl where same_file(url('http://www.ulb.be/en'),url('http://www.ulb.be/en'));

Drop index testidx;
SET enable_seqscan = ON;