all:
	@$(MAKE) -C src

.PHONY: clean

clean: 
	rm notevensql
	@$(MAKE) -C src/ clean

