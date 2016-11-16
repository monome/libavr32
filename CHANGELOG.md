# change log

### october 31, 2016

	introduced some chnages to allow aleph compatiblity. 
	
	#### changes in libavr32 sources:
	
	- add aleph conf headers
	- guard against redefinition of basic data types in types.h
    - fix screen drawing functions to work consistently across hardware
    - add arc accessor functions to monome driver
	- renamed SPI and NPCS defines in conf_board headers and module init functions
	(these needed to be 
	- added aleph hardware events to the events enum in events.h
	
	#### changes to ASF sources (tracked in diet_asf scripts)
	
	- removed unused board files from ASF
	- edit common/boards/board.h to include conf_board.h directly
	
	
