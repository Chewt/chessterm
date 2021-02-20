SRCDIR = src
OBJDIR = obj
INCLUDEDIR = include
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)%.c=$(OBJDIR)%.o)
CFLAGS = -I$(INCLUDEDIR) -g
CC = cc
TARGETS = $(SOURCES:$(SRCDIR)%.c=./%)

.PHONY: all $(TARGETS)
all: $(TARGETS)

$(TARGETS): $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) -o $(TARGETS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(OBJDIR)
	$(CC) -c $< $(CFLAGS) -o $@

.PHONY: clean
clean: 
	rm -rf $(OBJDIR)/*.o $(TARGETS)
