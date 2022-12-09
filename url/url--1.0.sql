-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION url" to load this file. \quit

CREATE OR REPLACE FUNCTION url_in(cstring)
RETURNS url
AS '$libdir/url', 'url_in'
LANGUAGE C IMMUTABLE CALLED ON NULL INPUT;

--url_out(url)
CREATE OR REPLACE FUNCTION url_out(url)
RETURNS cstring
AS '$libdir/url', 'url_out'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION url(cstring, cstring, integer, cstring)
RETURNS url
AS '$libdir/url', 'construct_url_with_port'
LANGUAGE C IMMUTABLE CALLED ON NULL INPUT;

CREATE OR REPLACE FUNCTION url(cstring, cstring, cstring)
RETURNS url
AS '$libdir/url', 'construct_url_without_port'
LANGUAGE C IMMUTABLE CALLED ON NULL INPUT;

CREATE OR REPLACE FUNCTION url(url, cstring)
RETURNS url
AS '$libdir/url', 'construct_url_with_context'
LANGUAGE C IMMUTABLE CALLED ON NULL INPUT;

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
LANGUAGE C IMMUTABLE CALLED ON NULL INPUT;

CREATE OR REPLACE FUNCTION text(url)
RETURNS text
AS '$libdir/url', 'url_to_text'
LANGUAGE C IMMUTABLE STRICT;

CREATE CAST (text as url) WITH FUNCTION url(text) AS IMPLICIT;
CREATE CAST (url as text) WITH FUNCTION text(url);

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

CREATE OR REPLACE FUNCTION equals(url, url)
RETURNS boolean
AS '$libdir/url', 'equals'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION same_file(url, url)
RETURNS boolean
AS '$libdir/url', 'same_file'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION get_user_info(url)
RETURNS cstring
AS '$libdir/url', 'get_user_info'
LANGUAGE C IMMUTABLE STRICT;

-- ---------- ---------- ---------- ---------- ---------- ---------- ----------
-- B-Tree Functions
-- For sorting and grouping
-- ---------- ---------- ---------- ---------- ---------- ---------- ----------

CREATE OR REPLACE FUNCTION url_eq(url, url)
RETURNS boolean
AS '$libdir/url', 'url_equals'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION url_nq(url, url)
RETURNS boolean
AS '$libdir/url', 'url_not_equals'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION url_lt(url, url)
RETURNS boolean
AS '$libdir/url', 'url_less_than'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION url_gt(url, url)
RETURNS boolean
AS '$libdir/url', 'url_greater_than'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION url_lt_eq(url, url)
RETURNS boolean
AS '$libdir/url', 'url_less_than_equal'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION url_gt_eq(url, url)
RETURNS boolean
AS '$libdir/url', 'url_greater_than_equal'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION url_compare(url, url)
RETURNS integer
AS '$libdir/url', 'url_compare'
LANGUAGE C IMMUTABLE STRICT;

--
-- Sorting operators for Btree
--

CREATE OPERATOR = (
	LEFTARG = url, RIGHTARG = url, PROCEDURE = url_eq,
	COMMUTATOR = '=', NEGATOR = '<>',
	RESTRICT = eqsel, JOIN = eqjoinsel
);
COMMENT ON OPERATOR =(url, url) IS 'are the both url equal?';

CREATE OPERATOR <> (
	LEFTARG = url, RIGHTARG = url, PROCEDURE = url_nq,
	COMMUTATOR = '<>', NEGATOR = '=',
	RESTRICT = neqsel, JOIN = neqjoinsel
);
COMMENT ON OPERATOR <>(url, url) IS 'are the both url not equal?';

CREATE OPERATOR < (
	LEFTARG = url, RIGHTARG = url, PROCEDURE = url_lt,
	COMMUTATOR = '>', NEGATOR = '>=',
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
COMMENT ON OPERATOR <(url, url) IS 'is first url less than second?';

CREATE OPERATOR > (
	LEFTARG = url, RIGHTARG = url, PROCEDURE = url_gt,
	COMMUTATOR = '<', NEGATOR = '<=',
	RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);
COMMENT ON OPERATOR >(url, url) IS 'is first url greater than second?';

CREATE OPERATOR <= (
	LEFTARG = url, RIGHTARG = url, PROCEDURE = url_lt_eq,
	COMMUTATOR = '>=', NEGATOR = '>',
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
COMMENT ON OPERATOR <=(url, url) IS 'is first url less than or equal to second?';

CREATE OPERATOR >= (
	LEFTARG = url, RIGHTARG = url, PROCEDURE = url_gt_eq,
	COMMUTATOR = '<=', NEGATOR = '<',
	RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);
COMMENT ON OPERATOR >=(url, url) IS 'is first url greater than or equal to second?';

--
-- Finally the Btree
--

CREATE OPERATOR CLASS url_ops
    DEFAULT FOR TYPE url USING btree AS
        OPERATOR        1       < ,
        OPERATOR        2       <= ,
        OPERATOR        3       = ,
        OPERATOR        4       >= ,
        OPERATOR        5       > ,
        FUNCTION        1       url_compare(url, url);

