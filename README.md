chauffeur
=========

Clang AST frontend for Linux device driver analysis

##Prerequisites
- llvm 3.5
- cmake
- libncurses5-dev (required apt package for Ubuntu 14.04 LTS)

##How to compile

	cmake -D LLVM_CONFIG=${PATH_TO_LLVM}/build/bin -D CMAKE_BUILD_TYPE=Release .
