# This file is included by all module.mk files, and provides common functionality
# You normally should never have to edit it.

# Generate .rules files for each application target
LRULES := $(patsubst %,$(LPATH)/$(DEPDIR)/%.rules,$(LPRODUCTS))
$(LRULES): genrules.pl
	-$(MKDIR) $(dir $@)
	./genrules.pl $(notdir $(basename $@)) > $@
-include $(LRULES) $(EMPTY_FILE)


# include generated dependencies (we append EMPTY_FILE to avoid warnings if
# the list happens to be empty)
LOBJ := $(patsubst %.cc,%.o,$(filter %.cc,$(LSRC)))
-include $(wildcard $(LPATH)/$(DEPDIR)/*.d) $(EMPTY_FILE)

# Transform LPRODUCTS to have full path information
LPRODUCTS := $(patsubst %,$(LPATH)/%$(EXEEXT),$(LPRODUCTS))

# Local all target
all-$(LPATH): $(LPRODUCTS)

# We want that "make dirname" works just like "make all-dirname"
$(LPATH): all-$(LPATH)

# Local clean target
LOBJ-$(LPATH) := $(patsubst %.cc,%.o,$(filter %.cc,$(LSRC)))
LPRODUCTS-$(LPATH) := $(LPRODUCTS)
clean-$(LPATH): clean-% :
	-$(RM) $(LOBJ-$*) $(LPRODUCTS-$*)

# The global all/clean targets should invoke all sub-targets, we do that here
all: all-$(LPATH)
clean: clean-$(LPATH)

.PHONY: $(LPATH) all-$(LPATH) clean-$(LPATH)
