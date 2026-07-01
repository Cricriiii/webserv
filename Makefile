# ===========================[ General rules ]==============================

OS			:= $(shell uname -s)

NAME		:=	webserv
CXX			:=	c++
CXXFLAGS	:=	-MMD -MP -std=c++98 -g3
SRC_DIR		:=	source
OBJ_DIR		:=	.build
INCLUDE		:=	include include/handlers include/http include/network include/tools include/vserver
CXXFLAGS	+=	$(addprefix -I, $(INCLUDE))

SRC_FIL_EXT	:=	.cpp
SRC_INC_EXT	:=	.hpp
OBJ_FIL_EXT	:=	.o
DEP_FIL_EXT	:=	.d


# ============================[ File paths ]================================

ROOT_SRC	:=	main									\
				GarbageCollector

# Application core
NET_SRC		:=	Acceptor								\
				NetConnection							\
				NetworkManager							\
				Router									\
				SessionManager

# HANDLERS
HAND_SRC	:=	AutoIndexHandler						\
				CGIHandler								\
				ErrorHandler							\
				StaticFileHandler						\
				HandlerValidator

# HTTP
HTTP_SRC	:=	HTTP_Request							\
				HTTP_Response							\
				HTTP_Parser								\
				HTTP_Serializer

#TOOLS
TOOL_SRC	:=	char_tools								\
				hexdump									\
				KeyVal_Parser							\
				StringHandler

#CONF FILE
CONF_SRC	:=	conf_parsing							\
				conf_lexer								\
				conf_utility							\
				conf

VSER_SRC	:=	vserver_creator							\
				vserver

ifeq ($(OS), Linux)
WRP_SRC		:=	epoll
NET_SRC		+=	NetworkManager_epoll

else ifeq ($(OS), Darwin)
NET_SRC		+= NetworkManager_kqueue

else
    $(error Unsupported OS: $(OS))

endif


SRC_PATHS	:=	$(ROOT_SRC)								\
				$(addprefix conf/,$(CONF_SRC))			\
				$(addprefix handlers/,$(HAND_SRC))		\
				$(addprefix http/,$(HTTP_SRC))			\
				$(addprefix network/,$(NET_SRC))		\
				$(addprefix tools/,$(TOOL_SRC))			\
				$(addprefix vserver/,$(VSER_SRC))		\
				$(addprefix wrappers/,$(WRP_SRC))

SRC			:=	$(addsuffix $(SRC_FIL_EXT),$(addprefix $(SRC_DIR)/,$(SRC_PATHS)))
OBJ			:=	$(patsubst $(SRC_DIR)/%$(SRC_FIL_EXT),$(OBJ_DIR)/%$(OBJ_FIL_EXT),$(SRC))
DEPS		:=	$(OBJ:$(OBJ_FIL_EXT)=$(DEP_FIL_EXT))

SRC			:=	$(addsuffix $(SRC_FIL_EXT),$(addprefix $(SRC_DIR)/,$(SRC_PATHS)))
OBJ			:=	$(patsubst $(SRC_DIR)/%$(SRC_FIL_EXT),$(OBJ_DIR)/%$(OBJ_FIL_EXT),$(SRC))
DEPS		:=	$(OBJ:$(OBJ_FIL_EXT)=$(DEP_FIL_EXT))


# ============================[ Make rules ]=================================

all:	$(NAME)

$(NAME):	$(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@

$(OBJ_DIR)/%$(OBJ_FIL_EXT):	$(SRC_DIR)/%$(SRC_FIL_EXT)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)


# ============================[ Cleaning ]===================================

clean c:
	@rm -rf $(OBJ_DIR)

fclean fc:	clean
	@rm -f $(NAME)

re:		fclean all

format:
	find . -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

test: all
	@valgrind					\
	--leak-check=full			\
	--show-leak-kinds=all		\
	--trace-children=yes		\
	$(NAME)

.PHONY: all clean c fclean fc re format test
