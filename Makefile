# Compiler and flags
CPP      := c++
CPPFLAGS := -Wall -Werror -Wextra -std=c++11
DEPFLAGS := -MMD -MP

# Target name
NAME     := webserv

OBJ_DIR  := obj

DIRS     :=
SRCS     := main.cpp
HDRS     :=

# configuration
DIRS     += src/configuration include/configuration
SRCS     += Lexer.cpp Parser.cpp
HDRS     += Lexer.hpp Parser.hpp

# Server / Routes
DIRS     += src include
SRCS     += Server.cpp Route.cpp
HDRS     += Server.hpp Route.hpp



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
	$(CPP) $(CPPFLAGS) $(OBJS) -o $(NAME)

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