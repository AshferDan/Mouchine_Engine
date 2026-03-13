# Put this makefile outside the src and include folders
# Just run command:
#
#			 go wsl do this stupid
#			 sudo apt-get install cloc
#            make countLines
#
# Thing will do for you even with subfolders inside src and include
# DING DONG DING DONG DING DONG

.PHONY: countLines

# Source files
ENGINE_FILES = ./Engine
MATH_FILE = ./Extern/MathLib.cpp ./Extern/MathLib.h
GAME_FILES = ./Game
CSHARP_FILES = ./Assets/Scripts


# Compiler and flags (For C++)
COUNTER_CHECK = cloc

# Combined total files
ALLCOUNT_FILES = $(SOURCES_C) $(HEADERS_H)

#------------------------------------------------------------------------------#
# Compiles the file:
countLines:
	$(COUNTER_CHECK) --by-file --force-lang="C++",h --force-lang="C++",hpp --ignored=ignoredFiles-In-Engine.txt $(ENGINE_FILES)
	$(COUNTER_CHECK) --by-file --force-lang="C++",h --force-lang="C++",hpp --ignored=ignoredFiles-In-Math.txt $(MATH_FILE)
	$(COUNTER_CHECK) --by-file --force-lang="C++",h --force-lang="C++",hpp --ignored=ignoredFiles-In-Game.txt $(GAME_FILES)
	$(COUNTER_CHECK) --by-file --force-lang="C++",h --force-lang="C++",hpp --ignored=ignoredFiles-In-Game.txt $(CSHARP_FILES)
#------------------------------------------------------------------------------#