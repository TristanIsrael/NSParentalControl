# Host unit tests for the pure playtime logic (no Switch SDK required)

CXX ?= g++
CXXFLAGS ?= -std=c++20 -Wall -Wextra -I../sysmodule/source

BUILDDIR = build
TESTS = playtime_rules_test

.PHONY: all test clean

all: test

test: $(addprefix $(BUILDDIR)/,$(TESTS))
	@for t in $(TESTS); do \
		echo "=== $$t ==="; \
		./$(BUILDDIR)/$$t || exit 1; \
	done
	@echo "All tests passed"

$(BUILDDIR)/%: %.cpp ../sysmodule/source/playtime_rules.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR)
