all:
	mmake
.PHONY: all

cleanTemp:
	rm Makefile.tmp.* # temp Makefiles may be generated if mmake is killed

clean: cleanTemp
	rm -rf bin/
.PHONY: clean