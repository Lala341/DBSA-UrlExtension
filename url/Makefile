EXTENSION   = url
MODULES     = url
DATA        = url.sql url.control

LDFLAGS=-lrt

PG_CONFIG ?= pg_config
PGXS = $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
