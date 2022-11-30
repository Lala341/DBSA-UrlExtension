-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION url" to load this file. \quit

CREATE OR REPLACE FUNCTION test_url_in(cstring)
RETURNS url
AS '$libdir/testurl', 'test_url_in'
LANGUAGE C IMMUTABLE STRICT;

--url_out(url)
CREATE OR REPLACE FUNCTION test_url_out(url)
RETURNS cstring
AS '$libdir/testurl', 'test_url_out'
LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE url (
	INPUT          = test_url_in,
	OUTPUT         = test_url_out
	-- LIKE           = text,
	-- CATEGORY       = 'S'
);
COMMENT ON TYPE url IS 'text written in url: [0-9A-Z]+';


