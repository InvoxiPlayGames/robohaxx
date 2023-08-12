TARGET := xsv_builder
ifeq ($(OS),Windows_NT)
	TARGET := $(addsuffix .exe,$(TARGET))
endif

SRC := sha1.c hmac_sha1.c xsv_builder.c

default: all

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	@-rm -f $(TARGET)
