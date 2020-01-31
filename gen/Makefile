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
override generator_source := $(_gen_dir)/$(_gen_source_file).cpp
override generator_binary := $(common_object_dir)/$(_gen_source_file)$(host_extension_exe)
override tmp_file := $(common_object_dir)/$(basename $(notdir $(target))).tmp$(suffix $(target))

# --- TARGETS
.DEFAULT_GOAL := generate
.PHONY: generate
generate: $(tmp_file)
	@$(call echo,[Generating] $(target) [Validating] $<)
	@$(CXX_COMPILER) $(CXXFLAGS) -fsyntax-only -x c++-header $<
	@$(call move,$<,$(target))
	@$(call echo,[Generating] $(target) [Done])

$(tmp_file): $(generator_binary)
	@$(call echo,[Generating] $(target) [Running] $<)
	@$(call native_path,./$<) $@
	@$(call rmfile,$<)

$(generator_binary): $(generator_source)
	@$(call echo,[Generating] $(target) [C++] $<)
	@$(CXX_LINKER) $(GEN_CXXFLAGS) $< -o $@
