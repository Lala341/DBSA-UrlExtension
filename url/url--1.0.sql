-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION url" to load this file. \quit

CREATE OR REPLACE FUNCTION url_in(cstring)
RETURNS url
AS '$libdir/url', 'url_in'
LANGUAGE C IMMUTABLE STRICT;

--url_out(url)
CREATE OR REPLACE FUNCTION url_out(url)
RETURNS cstring
AS '$libdir/url', 'url_out'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION url(cstring, cstring, integer, cstring)
RETURNS url
AS '$libdir/url', 'url_constructor_port'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION get_protocol(url)
RETURNS cstring
AS '$libdir/url', 'get_protocol'
LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE url (
	INPUT          = url_in,
	OUTPUT         = url_out
	-- LIKE           = text,
	-- CATEGORY       = 'S'
);
COMMENT ON TYPE url IS 'text written in url: [0-9A-Z]+';

CREATE OR REPLACE FUNCTION url(text)
RETURNS url
AS '$libdir/url', 'text_to_url'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION text(url)
RETURNS text
AS '$libdir/url', 'url_to_text'
LANGUAGE C IMMUTABLE STRICT;

CREATE CAST (text as url) WITH FUNCTION url(text) AS IMPLICIT;
CREATE CAST (url as text) WITH FUNCTION text(url);


