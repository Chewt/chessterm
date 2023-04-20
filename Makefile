SRCDIR = src
OBJDIR = obj
INCLUDEDIR = include
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)%.c=$(OBJDIR)%.o)
INCLUDES = $(SOURCES:$(SRCDIR)%.c=$(INCLUDEDIR)%.h)
UNIDEPS = include/settings.h
CFLAGS = -I$(INCLUDEDIR) 
CC = gcc
TARGET = chessterm

.PHONY: all 
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) -o $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCLUDEDIR)/%.h $(UNIDEPS)
	@mkdir -p $(OBJDIR)
	$(CC) -c $< $(CFLAGS) -o $@

$(INCLUDES):

$(UNIDEPS):
	@touch $@

.PHONY: color_picker
color_picker: 
	$(MAKE) -C add_ons

.PHONY: clean
clean: 
	rm -rf $(OBJDIR) /*.o $(TARGET) color_picker include/settings.h
	$(MAKE) clean -C add_ons

.PHONY: debug
debug: CFLAGS += -g -D DEBUG
debug: clean all

.PHONY: install
install: all
	@sudo cp $(TARGET) /usr/bin/$(TARGET)

.PHONY: uninstall
uninstall: 
	@sudo rm -f /usr/bin/$(TARGET)
