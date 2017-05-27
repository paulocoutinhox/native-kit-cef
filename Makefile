.DEFAULT_GOAL := help

# general
help:
	@echo "Type: make [rule]. Available options are:"
	@echo ""
	@echo "> GENERAL" 
	@echo "- help"
	@echo "- clean"
	@echo ""
	@echo "> MACOS"
	@echo "- build-native-kit-cef-macos"
	@echo ""

clean:
	rm -rf build

# macos
build-native-kit-cef-macos:
	rm -rf build
	mkdir -p build
	cd build && cmake -G "Xcode" -DPROJECT_ARCH="x86_64" ../source/