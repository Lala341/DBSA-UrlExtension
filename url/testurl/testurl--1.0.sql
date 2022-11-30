-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION testurl" to load this file. \quit

CREATE OR REPLACE FUNCTION test_url_in(cstring)
RETURNS testurl
AS '$libdir/testurl', 'test_url_in'
LANGUAGE C IMMUTABLE STRICT;

--url_out(url)
CREATE OR REPLACE FUNCTION test_url_out(testurl)
RETURNS cstring
AS '$libdir/testurl', 'test_url_out'
LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE testurl (
	INPUT          = test_url_in,
	OUTPUT         = test_url_out
	-- LIKE           = text,
	-- CATEGORY       = 'S'
);
COMMENT ON TYPE testurl IS 'text written in url: [0-9A-Z]+';


