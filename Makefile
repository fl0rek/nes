development:
	@$(MAKE) -C src CFLAGS_D='-ggdb -D_DEBUG'

production: 
	@$(MAKE) -C src CFLAGS_D=-DNDEBUG

.PHONY: clean

clean: 
	rm notevensql
	@$(MAKE) -C src/ clean

