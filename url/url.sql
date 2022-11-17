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

--url_recv

--url_send

CREATE TYPE url (
	INPUT          = url_in,
	OUTPUT         = url_out,
	RECEIVE        = url_recv,
	SEND           = url_send,
	LIKE           = text,
	CATEGORY       = 'S'
);
COMMENT ON TYPE url IS 'text written in url: [0-9A-Z]+';


