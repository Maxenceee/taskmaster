OBJ_DIR			=	.objs
SOURCES_DIR		=	sources
COMMON_SOURCES_DIR	=	$(SOURCES_DIR)/common
D_SOURCES_DIR		=	$(SOURCES_DIR)/daemon
CTL_SOURCES_DIR		=	$(SOURCES_DIR)/ctl

COMMON_SRCS		=	$(shell find $(COMMON_SOURCES_DIR) -name "*.cpp")
COMMON_OBJS		=	$(patsubst $(SOURCES_DIR)%.cpp, $(OBJ_DIR)%.o, $(COMMON_SRCS))
COMMON_DEPS		=	$(COMMON_OBJS:.o=.d)

D_SRCS			=	$(shell find $(D_SOURCES_DIR) -name "*.cpp")
D_OBJS			=	$(patsubst $(SOURCES_DIR)%.cpp, $(OBJ_DIR)%.o, $(D_SRCS))
D_DEPS			=	$(D_OBJS:.o=.d)

CTL_SRCS		=	$(shell find $(CTL_SOURCES_DIR) -name "*.cpp")
CTL_OBJS		=	$(patsubst $(SOURCES_DIR)%.cpp, $(OBJ_DIR)%.o, $(CTL_SRCS))
CTL_DEPS		=	$(CTL_OBJS:.o=.d)

HEADERS_DIR		=	includes
HEADERS			=	$(shell find $(HEADERS_DIR) -name "*.hpp") $(shell find $(SOURCES_DIR) -name "*.hpp")

RM				=	rm -f
CC				=	g++
CXXFLAGS		=	-Wall -Wextra -Werror
DEPSFLAG		=	-MMD -MP
CFLAGS			=	-g3 -std=c++20 -I $(HEADERS_DIR) -I $(COMMON_SOURCES_DIR) -I $(D_SOURCES_DIR) -I $(CTL_SOURCES_DIR) $(CXXFLAGS)

RLIBS			=	-lreadline -lhistory
ifeq ($(shell uname), Darwin)
RLIBS_DIR		=	$(shell brew --prefix readline)
RLIBS			+=	-L $(RLIBS_DIR)/lib
endif
CFLAGS			+=	-I$(RLIBS_DIR)/include
CFLAGS			+=	-D TOML_EXCEPTIONS=0

NAME_D			=	taskmasterd
NAME_CTL		=	taskmasterctl

GREEN			=	\033[1;32m
BLUE			=	\033[1;34m
RED				=	\033[1;31m
YELLOW			=	\033[1;33m
DEFAULT			=	\033[0m
UP				=	"\033[A"
CUT				=	"\033[K"

all: $(NAME_D) $(NAME_CTL)

use_fork: CFLAGS += -DTM_SPAWN_CHILD_USE_FORK
use_fork: $(NAME_D) $(NAME_CTL)

$(OBJ_DIR)/%.o: $(SOURCES_DIR)/%.cpp $(HEADERS) Makefile
	@mkdir -p $(@D)
	@echo "$(YELLOW)Compiling [$<]$(DEFAULT)"
	@$(CC) $(CFLAGS) $(DEPSFLAG) -c $< -o $@
	@printf ${UP}${CUT}

$(NAME_D): $(D_OBJS) $(COMMON_OBJS)
	@$(CC) $(D_OBJS) $(COMMON_OBJS) -o $(NAME_D)
	@echo "$(GREEN)$(NAME_D) compiled!$(DEFAULT)"

$(NAME_CTL): $(CTL_OBJS) $(COMMON_OBJS)
	@$(CC) $(CTL_OBJS) $(COMMON_OBJS) -o $(NAME_CTL) $(RLIBS)
	@echo "$(GREEN)$(NAME_CTL) compiled!$(DEFAULT)"

-include $(COMMON_DEPS)
-include $(D_DEPS)
-include $(CTL_DEPS)

clean:
	@echo "$(RED)Cleaning build folder$(DEFAULT)"
	@$(RM) -r $(OBJ_DIR)

fclean: clean
	@echo "$(RED)Cleaning $(NAME_D)$(DEFAULT)"
	@$(RM) $(NAME_D) $(NAME_CTL)

re:				fclean all

.PHONY:			all clean fclean re bonus
