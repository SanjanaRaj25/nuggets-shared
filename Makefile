# Makefile for all Nuggets modules
#
# Plankton - May 2023

.PHONY: all clean

############## default: make all libs and programs ##########
all: 
	make -C libs
	make -C support
	make -C common
	make -C server

############### TAGS for emacs users ##########
TAGS:  Makefile */Makefile */*.c */*.h */*.md */*.sh
	etags $^

############## clean  ##########
clean:
	rm -f *~
	rm -f TAGS
	make -C common clean
	make -C libs clean
	make -C support clean
	make -C server clean