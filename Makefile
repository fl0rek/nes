development:
	@$(MAKE) -C src CFLAGS_D='-ggdb -rdynamic -D_DEBUG'

production: 
	@$(MAKE) -C src CFLAGS_D=-DNDEBUG

.PHONY: clean

clean: 
	rm nes
	@$(MAKE) -C src/ clean

