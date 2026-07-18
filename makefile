CC=cc
CFLAGS += -Wall -Wno-missing-braces
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

$(EXENAME): $(SRC)
	$(CC) -o $@ $^ $(CPATH) $(LDPATH) $(LIBS) $(CFLAGS)

sanit: CFLAGS += -fsanitize=address
sanit: $(EXENAME)

run: $(EXENAME) runscr
	./$@.sh

clean:
	rm game
	rm $(RUNNAME).sh

# should also copy libs on target dir, for now ill just ".."
release: CFLAGS += -O3
release:
	if [ ! -d "$(TARGETDIR)" ]; then mkdir $(TARGETDIR); fi
	make runscr && mv $(RUNNAME).sh $(TARGETDIR)/ && \
		cp -r libs/ $(TARGETDIR)/
	chmod +x $(TARGETDIR)/$(RUNNAME).sh
	make $(EXENAME) && mv $(EXENAME) $(TARGETDIR)/

runscr:
	if [ ! -f "$(RUNNAME).sh" ]; then \
		echo "LD_LIBRARY_PATH=$(LIBPATH) ./$(EXENAME)" > $(RUNNAME).sh \
		; fi
	chmod +x $(RUNNAME).sh
