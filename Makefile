SRCDIR = src
OBJDIR = obj
INCLUDEDIR = include
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)%.c=$(OBJDIR)%.o)
CFLAGS = -I$(INCLUDEDIR) -g
CC = cc
TARGET = chessterm

.PHONY: all $(TARGET)
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) -o $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(OBJDIR)
	$(CC) -c $< $(CFLAGS) -o $@

.PHONY: clean
clean: 
	rm -rf $(OBJDIR)/*.o $(TARGET)
