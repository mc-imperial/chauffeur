chauffeur
=========

Clang AST frontend for Linux device driver analysis

##How to compile

  CXXFLAGS="-std=c++11 -stdlib=libstdc++" \
	cmake -D LLVM_CONFIG=/Users/pantazis/workspace/whoop/llvm_and_clang/build/bin -D CMAKE_BUILD_TYPE=Release ../src
