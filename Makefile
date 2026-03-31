SRCDIR = src
OBJDIR = obj
INCLUDEDIR = include
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)%.c=$(OBJDIR)%.o)
INCLUDES = $(SOURCES:$(SRCDIR)%.c=$(INCLUDEDIR)%.h)
UNIDEPS = 
CFLAGS = -I$(INCLUDEDIR) -Wunused-function -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers
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

.PHONY: clean
clean:
	rm -rf $(OBJDIR) /*.o $(TARGET)

.PHONY: debug
debug: CFLAGS += -g -D DEBUG
debug: clean all

.PHONY: release
release: CFLAGS += -O3
release: clean all

.PHONY: install
install: release
	@sudo cp $(TARGET) /usr/bin/$(TARGET)

.PHONY: uninstall
uninstall:
	@sudo rm -f /usr/bin/$(TARGET)
