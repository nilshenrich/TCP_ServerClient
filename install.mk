.PHONY: install

tcp: SHELL:=/bin/bash
tcp:
	targetDir='/usr/local/include/tcp' ;\
	rm -rf $$targetDir ;\
	cp -r ./src/basic $$targetDir ;\

install: tcp
