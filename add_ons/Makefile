SRCDIR = src
OBJDIR = obj
INCLUDEDIR = include
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)%.c=$(OBJDIR)%.o)
CFLAGS = -I$(INCLUDEDIR) -I ../$(INCLUDEDIR) -g
CC = cc

color_picker: $(OBJDIR)/color_picker.o ../obj/board.o
	$(CC) $^ $(CFLAGS) -o ../$@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(OBJDIR)
	$(CC) -c $< $(CFLAGS) -o $@

../obj/board.o:
	$(MAKE) -C .. obj/board.o

.PHONY: clean
clean: 
	rm -rf $(OBJDIR)/*.o $(TARGET)
