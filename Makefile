all:
	$(MAKE) -C src/tmx-parser
	$(MAKE) -C src/mlpbf
	
debug:
	$(MAKE) -C src/tmx-parser
	$(MAKE) -C src/mlpbf debug
	
clean:
	$(MAKE) -C src/tmx-parser clean
	$(MAKE) -C src/mlpbf clean

