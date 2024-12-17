PROJECT_NAME = PR12-CYA-2425
DIR_PATH=$(shell realpath .)
DIR_NAME=$(shell basename ${DIR_PATH})

configure:
	cmake -S . -B build

build: configure
	cmake --build build

clean:
	rm -rf build

tar:
	cd ..; tar cvfz ./${DIR_NAME}/HernándezJiménezPablo-$(PROJECT_NAME).tar.gz --exclude-from=${DIR_NAME}/.gitignore ${DIR_NAME}
