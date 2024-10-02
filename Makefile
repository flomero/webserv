# Compiler and flags
CPP      := c++
CPPFLAGS := -Wall -Werror -Wextra -std=c++11
DEPFLAGS := -MMD -MP

# Target library name
NAME     := libftpp.a

OBJ_DIR  := obj



# data_structures
DIRS     := src/configuration
SRCS     :=
HDRS     :=



OBJS     := $(addprefix $(OBJ_DIR)/, $(SRCS:.cpp=.o))
DEPS     := $(SRCS:.cpp=.d)

HDR_CHECK := $(addprefix $(OBJ_DIR)/, $(HDRS:.hpp=.hpp.gch))

# VPATH for source and header files
vpath %.cpp $(DIRS)
vpath %.hpp $(DIRS)

# Main target
all: $(NAME)

# Rule to create the static library
$(NAME): $(OBJS) $(HDR_CHECK)
	ar rcs $@ $(OBJS)

# Rule to compile source files into object files
$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	$(CPP) $(CPPFLAGS) $(DEPFLAGS) -c $< -o $@

$(OBJ_DIR)/%.hpp.gch: %.hpp | $(OBJ_DIR)
	$(CPP) $(CPPFLAGS) -x c++-header -c $< -o $@

$(OBJ_DIR) :
	mkdir -p $@

-include $(DEPS)

# Clean rule
clean:
	rm -rf $(OBJ_DIR) $(DEPS)

# Full clean rule
fclean: clean
	rm -f $(NAME)

# Rebuild everything
re: fclean all

# Phony targets
.PHONY: all clean fclean re