all:
	$(MAKE) -C src/mlpbf
	
install:
	$(MAKE) -C src/tmx-parser install
	
uninstall:
	$(MAKE) -C src/tmx-parser uninstall
	
debug:
	$(MAKE) -C src/mlpbf debug
	
clean:
	$(MAKE) -C src/mlpbf clean
