# See https://stackoverflow.com/a/1139313/12597781

all:
	+$(MAKE) -C src/
#   ^ Use the + sign to tell the top-level `make` that it can safely run the `make` in the
# src/ directory in parallel with other tasks that might be defined in the top-level Makefile.

clean:
	+$(MAKE) -C src/ clean