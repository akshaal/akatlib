# Note that gcc must be built with --enable-lto option (gcc 4.5 or newer)!
# At them momennt gold linker supports only x86 platform, so we are not going to use libraries

distclean: clean

benchmark:
	+make -C benchmark

clean:
	+make -C benchmark clean

.PHONY: benchmark clean distclean
