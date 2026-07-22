CC=cc
CFLAGS += -Wall -Wno-missing-braces -g
LIBPATH=./libs/SDL2/
CPATH=-I./libs/SDL2/include/
LDPATH=-L$(LIBPATH)
LIBS= -lSDL2 -lm #-l:libSDL2.a
SRC=src/main.c src/lmath.c src/render.c src/mesh.c src/loop.c src/utils.c \
	src/obj.c
DEPS=src/lmath.h src/render.h src/mesh.h src/loop.h src/utils.h src/obj.h
EXENAME=game
RUNNAME=run
TARGETDIR=target

.PHONY = release sanit

$(EXENAME): $(SRC)
	make runscr
	$(CC) -o $@ $^ $(CPATH) $(LDPATH) $(LIBS) $(CFLAGS)

sanit: CFLAGS += -fsanitize=address
sanit: clean $(EXENAME)

run: $(EXENAME) runscr
	./$@.sh

clean:
	@if [ -f $(EXENAME) ]; then rm $(EXENAME); echo "removing $(EXENAME)"; \
		else echo "$(EXENAME) not removed, no such file"; fi;
	@if [ -f $(RUNNAME).sh ]; then rm $(RUNNAME).sh; echo "removing $(RUNNAME).sh"; \
		else echo "$(RUNNAME).sh not removed, no such file"; fi;

release: CFLAGS += -O3
release: clean $(EXENAME)
	@if [ ! -d "$(TARGETDIR)" ]; then mkdir $(TARGETDIR); fi
	make runscr && mv $(RUNNAME).sh $(TARGETDIR)/ && \
		cp -r libs/ $(TARGETDIR)/
		cp -r assets/ $(TARGETDIR)/
	chmod +x $(TARGETDIR)/$(RUNNAME).sh
	mv $(EXENAME) $(TARGETDIR)/

runscr:
	@if [ ! -f "$(RUNNAME).sh" ]; then \
		echo "LD_LIBRARY_PATH=$(LIBPATH) ./$(EXENAME)" > $(RUNNAME).sh \
		; fi
	@chmod +x $(RUNNAME).sh

clang: CC=clang
clang: $(EXENAME)
