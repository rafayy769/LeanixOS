# Color codes for print statements
GREEN = \033[1;32m
CYAN = \033[1;36m
RED = \033[1;31m
YELLOW = \033[1;33m
BOLD = \033[1;37m
RESET = \033[0m

V ?= 2
D ?= 1
MAKEFLAGS += --no-print-directory

# Verbosity control. Inspired from the Contiki-NG build system. A few hacks here and there, will probably improve later.
ifeq ($(V),0)
  TRACE_AR =
  TRACE_AS =
  TRACE_CC =
  TRACE_LD =
  TRACE_MKDIR =
  TRACE_CP =
  TRACE_OBJCOPY =
  TRACE_DD =
  Q =
  
  BUILD_SUCCESS=
  BUILD_FAILURE=
  LINK_FAILURE=
  INIT_SUCCESS=
  INIT_MAIN=
  RUN=
  VALGRIND_RUN=
  CLEAN=

else ifeq ($(V),1)
  TRACE_AR	     = @echo "$(YELLOW)[ AR      ]$(RESET)" $@
  TRACE_AS	     = @echo "$(YELLOW)[ AS      ]$(RESET)" $< 
  TRACE_CC       = @echo "$(YELLOW)[ CC      ]$(RESET)" $< 
  TRACE_LD       = @echo "$(YELLOW)[ LD      ]$(RESET)" $@
  TRACE_MKDIR    = @echo "$(YELLOW)[ MKDIR   ]$(RESET)" $@
  TRACE_CP       = @echo "$(YELLOW)[ CP      ]$(RESET)" $<
  TRACE_OBJCOPY  = @echo "$(YELLOW)[ OBJCOPY ]$(RESET)" $@
  TRACE_DD       = @echo "$(YELLOW)[ DD      ]$(RESET)" $@
  Q = @
  
  BUILD_SUCCESS  =  @echo "-- $(GREEN)Build successful in $(BUILD_DEFAULT) mode.$(RESET): $(TARGET)"
  BUILD_FAILURE  =  echo  "-- $(RED)Build failed.$(RESET)"; exit 1
  LINK_FAILURE   =  echo  "-- $(RED)Linking failed.$(RESET)"; exit 1
  INIT_MAIN      =  @echo "-- $(CYAN)Creating main.c$(RESET)"
  INIT_SUCCESS   =  @echo "-- $(GREEN)Initialized the project structure$(RESET)"
  RUN            =  @echo "-- $(CYAN)Executing$(RESET): $(TARGET_NAME)"
  VALGRIND_RUN   =  @echo "-- $(CYAN)Running Valgrind on$(RESET): $(TARGET_NAME)"
  CLEAN          =  @echo "-- $(GREEN)Cleaned$(RESET): $(BUILD_DIR)/*"

else
  TRACE_AR	     = @echo "$(GREEN)[ AR      ]$(RESET)" $@ "<--" "[ "$^ "]"
  TRACE_AS	     = @echo "$(GREEN)[ AS      ]$(RESET)" $< "-->" $@
  TRACE_CC       = @echo "$(GREEN)[ CC      ]$(RESET)" $< "-->" $@
  TRACE_LD       = @echo "$(GREEN)[ LD      ]$(RESET)" $@ "<--" "[ "$^ "]"
  TRACE_MKDIR    = @echo "$(GREEN)[ MKDIR   ]$(RESET)" $@
  TRACE_CP       = @echo "$(GREEN)[ CP      ]$(RESET)" $< "-->" $@
  TRACE_OBJCOPY  = @echo "$(GREEN)[ OBJCOPY ]$(RESET)" $@
  TRACE_DD       = @echo "$(GREEN)[ DD      ]$(RESET)" $@
  Q ?= @
  
  BUILD_SUCCESS  = @echo "-- $(GREEN)Build successful in $(BUILD_DEFAULT) mode.$(RESET): $(TARGET)"
  BUILD_FAILURE  = echo  "-- $(RED)Build failed.$(RESET)"; exit 1
  LINK_FAILURE   = echo  "-- $(RED)Linking failed.$(RESET)"; exit 1
  INIT_MAIN      = @echo "-- $(CYAN)Creating main.c$(RESET)"
  INIT_SUCCESS   = @echo "-- $(GREEN)Initialized the project structure$(RESET)"
  RUN            = @echo "-- $(CYAN)Executing$(RESET): $(TARGET_NAME)"
  VALGRIND_RUN   = @echo "-- $(CYAN)Running Valgrind on$(RESET): $(TARGET_NAME)"
  CLEAN          = @echo "-- $(GREEN)Cleaned$(RESET): $(BUILD_DIR)/*"
endif