TARGET := gci_builder
ifeq ($(OS),Windows_NT)
	TARGET := $(addsuffix .exe,$(TARGET))
endif

SRC := gci_builder.c

default: all

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	@-rm -f $(TARGET)
