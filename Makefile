# Compiler and flags
CXX      := c++
CXXFLAGS := -Wall -Werror -Wextra -std=c++17
DEPFLAGS := -MMD -MP

# Target name
NAME     := webserv

# Directory for object files
OBJ_DIR  := obj

# Source directories and files
SRCS_DIR := src \
            src/http/messages \
			src/http/status

# Header directories
HDRS_DIR := include/ \
			include/http/messages \
			include/http/status

INCLUDES := $(addprefix -I, $(HDRS_DIR))

SRCS     := main.cpp \
            HttpMessage.cpp \
            HttpRequest.cpp \
			HttpResponse.cpp \
			HttpStatus.cpp

HDRS     := HttpMessage.hpp \
            HttpRequest.hpp \
			HttpResponse.hpp \
			HttpStatus.hpp


OBJS     := $(addprefix $(OBJ_DIR)/, $(SRCS:.cpp=.o))
DEPS     := $(OBJS:.o=.d)

# Precompiled headers
HDR_CHECK := $(addprefix $(OBJ_DIR)/, $(HDRS:.hpp=.hpp.gch))

# VPATH for source and header files
vpath %.cpp $(SRCS_DIR)
vpath %.hpp $(HDRS_DIR)

# Main target
all: ascii $(NAME)

# Rule to build the executable
$(NAME): $(OBJS) $(HDR_CHECK)
	@echo "$(CLEAR_LINE)$(YELLOW)Linking $(ITALIC_LIGHT_YELLOW)$(NAME)$(NC)"
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $@ $(INCLUDES)
	@if [ -f $(NAME) ]; then \
		echo "$(GREEN)$(NAME) compiled successfully!$(NC)"; \
		echo "$(CYAN)Run with ./$(NAME)$(NC)"; \
		echo "------------------------------------------------"; \
	else \
		echo "$(RED)$(NAME) failed to compile$(NC)"; \
	fi

# Rule to compile source files into object files
$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	@$(eval CURRENT := $(shell echo $$(($(CURRENT) + 1))))
	@$(eval PERCENT := $(shell echo $$(($(CURRENT) * 100 / $(TOTAL_SRCS)))))
	@printf "$(CLEAR_LINE)$(YELLOW)Compiling $(PERCENT)%% [$(CURRENT)/$(TOTAL_SRCS)] $(ITALIC_LIGHT_YELLOW)$<$(NC) "
	@$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@ $(INCLUDES)

# Rule for precompiled headers
$(OBJ_DIR)/%.hpp.gch: %.hpp | $(OBJ_DIR)
	@$(CXX) $(CXXFLAGS) -x c++-header -c $< -o $@

# Create object directory
$(OBJ_DIR):
	@echo "$(YELLOW)Creating object directory...$(NC)"
	@mkdir -p $@

# Include generated dependencies
-include $(DEPS)

# Clean rule
clean:
	@echo "$(RED)Cleaning $(NAME)...$(NC)"
	@rm -rf $(OBJ_DIR)

# Full clean rule
fclean: clean
	@echo "$(RED)Removing binary files...$(NC)"
	@rm -f $(NAME)

# Rebuild everything
re: fclean all

# add debug flags
debug: $(CXXFLAGS) += -g -fsanitize=address -pedantic
debug: re

# Phony targets
.PHONY: all clean fclean re debug

# Colors:
GREEN = \033[0;32m
LIGHT_GREEN = \033[0;92m
RED = \033[0;31m
YELLOW = \033[0;33m
LIGHT_YELLOW = \033[0;93m
ITALIC_LIGHT_YELLOW = \033[3;93m
CYAN = \033[0;36m
NC = \033[0m
CLEAR_LINE = \033[2K\r
ASCII_ART = $(CYAN)

ascii:
	@echo "------------------------------------------------"
	@echo "$(CYAN)"
	@echo "     __ __  __ __ __      "
	@echo "|  ||_ |__)(_ |_ |__)\  / "
	@echo "|/\||__|__)__)|__| \  \/  "
	@echo "                          "
	@echo "$(NC)"
	@echo "------------------------------------------------"

BAR_WIDTH = 50
TOTAL_SRCS = $(words $(SRCS))
