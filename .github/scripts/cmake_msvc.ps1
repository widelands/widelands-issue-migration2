param ($Config)

cmake.exe -G "NMake Makefiles" .. `
	-DVCPKG_TARGET_TRIPLET=$env:VCPKG_TARGET_TRIPLET `
	-DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
	-DCMAKE_BUILD_TYPE="$Config" `
	-DOPTION_BUILD_WEBSITE_TOOLS=OFF `
	-DOPTION_BUILD_TRANSLATIONS=ON `
	-DOPTION_BUILD_TESTS=ON `
	-DOPTION_ASAN=OFF `
	-DOPTION_BUILD_CODECHECK=OFF `
	-DOPTION_BUILD_WINSTATIC=ON `
	-DOPTION_USE_GLBINDING=ON `
	-DOPTION_FORCE_EMBEDDED_MINIZIP=ON