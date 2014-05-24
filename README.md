chauffeur
=========

Clang AST frontend for Linux device driver analysis

##How to compile

	CXXFLAGS="-std=c++11 -stdlib=libstdc++" \
	cmake -D LLVM_CONFIG=${PATH_TO_LLVM}/build/bin -D CMAKE_BUILD_TYPE=Release ../src
