# --- PREVENT CIRCULAR INCLUSION ---
$(if $(detect_circular_inclusion),$(error Current working directory must be the project root))

# --- CHECK CONFIG ---
$(if $(_gen_dir),,$(error Bad config))
$(if $(_gen_target_file),,$(error Bad config))

# --- INCLUDE CONFIG ---
override detect_circular_inclusion := 1
include Makefile

# --- LOCATE FILES ---
override target := $(_gen_target_file)
override generator_source := $(_gen_dir)/make_$(subst .,_,$(notdir $(target))).cpp
override generator_binary := $(common_object_dir)/make_$(subst .,_,$(notdir $(target)))$(host_extension_exe)
override tmp_file := $(common_object_dir)/$(basename $(notdir $(target))).tmp$(suffix $(target))

# --- TARGETS
.DEFAULT_GOAL := generate
.PHONY: generate
generate: $(tmp_file)
	$(info [Generating] $(target) [Validating] $<)
	@$(CXX_COMPILER) $(CXXFLAGS) -fsyntax-only -x c++-header $<
	@$(call move,$<,$(target))
	$(info [Generating] $(target) [Done])

$(tmp_file): $(generator_binary)
	$(info [Generating] $(target) [Running] $<)
	@$(call native_path,./$<) $@
	@$(call rmfile,$<)

$(generator_binary): $(generator_source)
	$(info [Generating] $(target) [C++] $<)
	@$(CXX_LINKER) $(GEN_CXXFLAGS) $< -o $@
