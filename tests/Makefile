# Host unit tests for the pure playtime logic and the IPC command contract.
# These do not require the Switch SDK to build or run.

CXX ?= g++
DEVKITPRO ?= /opt/devkitpro
CXXFLAGS ?= -std=c++20 -Wall -Wextra -I../sysmodule/source -I$(DEVKITPRO)/libnx/include

BUILDDIR = build
TESTS = playtime_rules_test command_test

# devkitA64 cross-compiler (used only by the `switch` target).
A64CXX ?= $(DEVKITPRO)/devkitA64/bin/aarch64-none-elf-g++
A64CXXFLAGS ?= -std=c++20 -Wall -O2 -march=armv8-a -mtune=cortex-a57 -fPIE -D__SWITCH__ \
	-I../sysmodule/source -I$(DEVKITPRO)/libnx/include

.PHONY: all test switch clean

all: test

test: $(addprefix $(BUILDDIR)/,$(TESTS))
	@for t in $(TESTS); do \
		echo "=== $$t ==="; \
		./$(BUILDDIR)/$$t || exit 1; \
	done
	@echo "All tests passed"

# Toolchain check: cross-compile the toolchain test for the Switch target.
# Requires the devkitPro toolchain; the host build does not run it.
switch: $(BUILDDIR)/switch/switch_only_test.o
	@echo "Switch cross-compile check OK"

$(BUILDDIR)/switch/switch_only_test.o: switch_only_test.cpp ../sysmodule/source/playtime_rules.cpp | $(BUILDDIR)/switch
	$(A64CXX) $(A64CXXFLAGS) -c $< -o $@

$(BUILDDIR)/switch:
	mkdir -p $(BUILDDIR)/switch

$(BUILDDIR)/%: %.cpp ../sysmodule/source/playtime_rules.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR)
