EXTENSION   = testurl
MODULES     = testurl
DATA        = testurl--1.0.sql testurl.control

LDFLAGS=-lrt

PG_CONFIG ?= pg_config
PGXS = $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
