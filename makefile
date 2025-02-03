SRCDIR=src

INCDIR=lib/include

BUILDDIR=build

# Collect source files
CXXSRC=$(wildcard $(SRCDIR)/**/*.cpp) $(wildcard $(SRCDIR)/*.cpp)
CSRC=$(wildcard $(SRCDIR)/**/*.c) $(wildcard $(SRCDIR)/*.c)
INC=$(wildcard $(INCDIR)/**/*.h)

EXEC=audio-vis

CFLAGS= -Wall -I $(INCDIR) -L lib/libraries -lportaudio -lasound -lglfw3 -lGL -lfftw3f -lm -lpthread

# Object and dependency files (based on header files)
OBJS=$(CXXSRC:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o) $(CSRC:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
DEPS=$(OBJS:%.o=%.d)

$(EXEC): $(OBJS)
	@echo "Building executable $@ from build files ($^)"
	@g++ -o $@ $^ $(CFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@echo "Compiling $@ from $<"
	@mkdir -p $(dir $@)
	@g++ -c $< -o $@ $(CFLAGS) -MMD

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@echo "Compiling $@ from $<"
	@mkdir -p $(dir $@)
	@g++ -c $< -o $@ $(CFLAGS) -MMD

# Build task
build: $(EXEC)

# Run Task
run | debug: $(EXEC)
	./audio-vis

# Clean Task
clean:
	@rm -rdfv ./$(BUILDDIR);
	@rm -v ./$(EXEC);
.PHONY: clean debug build run

-include $(DEPS)
