-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION url" to load this file. \quit

CREATE OR REPLACE FUNCTION url_in(cstring)
RETURNS url
AS '$libdir/url'
LANGUAGE C IMMUTABLE STRICT;

--url_out(url)
CREATE OR REPLACE FUNCTION url_out(url)
RETURNS cstring
AS '$libdir/url'
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
	OUTPUT         = url_out,
	LIKE           = text,
	CATEGORY       = 'S'
);
COMMENT ON TYPE url IS 'text written in url: [0-9A-Z]+';

CREATE OR REPLACE FUNCTION url(text)
RETURNS url
AS '$libdir/url', 'text_to_url'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION text(url)
RETURNS cstring
AS '$libdir/url', 'url_to_text'
LANGUAGE C IMMUTABLE STRICT;

-- CREATE CAST (text as url) WITH FUNCTION url(text) AS IMPLICIT;
-- CREATE CAST (url as cstring) WITH FUNCTION text(url);

-- to be continue --start
CREATE OR REPLACE FUNCTION get_authority(url)
RETURNS cstring
AS '$libdir/url', 'get_authority'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION get_file(url)
RETURNS cstring
AS '$libdir/url', 'get_file'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION get_path(url)
RETURNS cstring
AS '$libdir/url', 'get_path'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION same_host(url, url)
RETURNS boolean
AS '$libdir/url', 'same_host'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION get_default_port(url)
RETURNS integer
AS '$libdir/url', 'get_default_port'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION get_host(url)
RETURNS cstring
AS '$libdir/url', 'get_host'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION get_port(url)
RETURNS integer
AS '$libdir/url', 'get_port'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION get_query(url)
RETURNS cstring
AS '$libdir/url', 'get_query'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION get_ref(url)
RETURNS cstring
AS '$libdir/url', 'get_ref'
LANGUAGE C IMMUTABLE STRICT;

-- TODO replace text with URL data type 
CREATE OR REPLACE FUNCTION equals(text, text)
RETURNS boolean
AS '$libdir/url', 'equals'
LANGUAGE C IMMUTABLE STRICT;
-- to be continue --finish