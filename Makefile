.PHONY: install

install: SHELL:=/bin/bash
install:
	# targetDir='/usr/local/include/tcp' ;\
	targetDir='tcp-test' ;\
	rm -rf $$targetDir ;\
	mkdir $$targetDir ;\
	find src/ -name '*.h' -exec cp --parents \{\} $$targetDir \; # TODO: src folder is copied, not unpacked
