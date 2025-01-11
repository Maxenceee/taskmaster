OBJ_DIR			=	.objs
SOURCES_DIR		=	sources

COMMON_SRCS		=	$(shell find $(SOURCES_DIR)/common -name "*.cpp")
COMMON_OBJS		=	$(patsubst $(SOURCES_DIR)%.cpp, $(OBJ_DIR)%.o, $(COMMON_SRCS))

D_SRCS			=	$(shell find $(SOURCES_DIR)/daemon -name "*.cpp")
D_OBJS			=	$(patsubst $(SOURCES_DIR)%.cpp, $(OBJ_DIR)%.o, $(D_SRCS))

CTL_SRCS		=	$(shell find $(SOURCES_DIR)/ctl -name "*.cpp")
CTL_OBJS		=	$(patsubst $(SOURCES_DIR)%.cpp, $(OBJ_DIR)%.o, $(CTL_SRCS))

HEADERS_DIR		=	includes
HEADERS			=	$(shell find $(HEADERS_DIR) -name "*.hpp")

RM				=	rm -f
CC				=	g++
CFLAGS			=	-g3 -std=c++20 -I $(HEADERS_DIR) #-Wall -Wextra -Werror
LIBS			=	-lpthread -lcrypto

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

$(OBJ_DIR)/%.o: $(SOURCES_DIR)/%.cpp $(HEADERS) Makefile
	@mkdir -p $(@D)
	@echo "$(YELLOW)Compiling [$<]$(DEFAULT)"
	@$(CC) $(CFLAGS) -c $< -o $@
	@printf ${UP}${CUT}

$(NAME_D): $(D_OBJS) $(COMMON_OBJS)
	@$(CC) $(D_OBJS) $(COMMON_OBJS) -o $(NAME_D) $(LIBS)
	@echo "$(GREEN)$(NAME_D) compiled!$(DEFAULT)"

$(NAME_CTL): $(CTL_OBJS) $(COMMON_OBJS)
	@$(CC) $(CTL_OBJS) $(COMMON_OBJS) -o $(NAME_CTL) $(LIBS)
	@echo "$(GREEN)$(NAME_CTL) compiled!$(DEFAULT)"

clean:
	@echo "$(RED)Cleaning build folder$(DEFAULT)"
	@$(RM) -r $(OBJ_DIR)

fclean: clean
	@echo "$(RED)Cleaning $(NAME_D)$(DEFAULT)"
	@$(RM) $(NAME_D) $(NAME_CTL)

re:				fclean all

.PHONY:			all clean fclean re bonus
