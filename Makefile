.PHONY: install

install: SHELL:=/bin/bash
install:
	targetDir='/usr/local/include/tcp' ;\
	rm -rf $$targetDir ;\
	cp -r ./src $$targetDir ;\
