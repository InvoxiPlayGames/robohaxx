GCIBUILDER := gci_builder
ifeq ($(OS),Windows_NT)
	GCIBUILDER := $(addsuffix .exe,$(GCIBUILDER))
endif

PAYLOAD_DIR := gc_payload
GCI_BUILDER_DIR := gci_builder_src

.PHONY: $(GCIBUILDER) payload_bins eur usa clean
all: eur usa

$(GCIBUILDER):
	@$(MAKE) -C $(GCI_BUILDER_DIR)
	@cp $(GCI_BUILDER_DIR)/$(GCIBUILDER) $(GCIBUILDER)

payload_bins:
	@$(MAKE) -C $(PAYLOAD_DIR)

eur: payload_bins $(GCIBUILDER)
	@./$(GCIBUILDER) $(PAYLOAD_DIR)/robohaxx-EUR.bin E

usa: payload_bins $(GCIBUILDER)
	@./$(GCIBUILDER) $(PAYLOAD_DIR)/robohaxx-USA.bin U

clean:
	@$(MAKE) -C $(PAYLOAD_DIR) clean
	@$(MAKE) -C $(GCI_BUILDER_DIR) clean
	@-rm $(GCIBUILDER)
	@-rm *.gci
