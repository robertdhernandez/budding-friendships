all:
	$(MAKE) -C src/tmx-parser
	$(MAKE) -C src/mlpbf
	
clean:
	$(MAKE) -C src/tmx-parser clean
	$(MAKE) -C src/mlpbf clean

