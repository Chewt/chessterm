SRCDIR = src
OBJDIR = obj
INCLUDEDIR = include
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)%.c=$(OBJDIR)%.o)
INCLUDES = $(SOURCES:$(SRCDIR)%.c=$(INCLUDEDIR)%.h)
UNIDEPS = include/settings.h
CFLAGS = -I$(INCLUDEDIR) -g
CC = cc
TARGET = chessterm

.PHONY: all $(TARGET)
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) -o $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCLUDEDIR)/%.h $(UNIDEPS)
	mkdir -p $(OBJDIR)
	$(CC) -c $< $(CFLAGS) -o $@

$(INCLUDES):

$(UNIDEPS):
	touch $@

.PHONY: color_picker
color_picker: 
	$(MAKE) -C add_ons

.PHONY: clean
clean: 
	rm -rf $(OBJDIR) /*.o $(TARGET) color_picker 
