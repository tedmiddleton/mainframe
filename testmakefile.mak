PYTHON ?= $(shell command -v python3 2> /dev/null || echo python)

all: show-versions

show-versions:
	which $(PYTHON)
	$(PYTHON) --version
	which make
	make --version
	which gcc
	gcc --version
	which ld
	ld --version
	python -m pip list
	python -m pip list --user
	python -m pip install ninja
	python -m pip install meson
	python -m pip list
	printenv
	echo "=============="
	set



