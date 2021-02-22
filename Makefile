SRCDIR = src
OBJDIR = obj
INCLUDEDIR = include add_ons/include
INCLUDEPARAMS = $(foreach dir, $(INCLUDEDIR), -I$(dir))
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)%.c=$(OBJDIR)%.o)
INCLUDES = $(SOURCES:$(SRCDIR)%.c=$(INCLUDEDIR)%.h)
UNIDEPS = 
CFLAGS = $(INCLUDEPARAMS) -g
CC = cc
TARGET = chessterm

VPATH = add_ons

.PHONY: all $(TARGET)
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) -o $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCLUDEDIR)/%.h $(UNIDEPS)
	mkdir -p $(OBJDIR)
	$(CC) -c $< $(CFLAGS) -o $@

<<<<<<< HEAD
$(INCLUDES):
=======
color_picker: obj/color_picker.o obj/board.o
	$(CC) $^ $(CFLAGS) -o color_picker
>>>>>>> add_ons

.PHONY: clean
clean: 
	rm -rf $(OBJDIR)/*.o $(TARGET) color_picker
