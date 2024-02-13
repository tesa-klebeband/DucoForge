CPP = g++
LINKER = -lcurl -lcrypto -lpthread
BUILD_DIR = build
SRC_DIR = src
EXEC_BIN = ducoforge

all: prepare $(EXEC_BIN)

prepare:
	mkdir -p $(BUILD_DIR)

$(EXEC_BIN): $(SRC_DIR)/*.cpp
	$(CPP) -Ofast -o $(BUILD_DIR)/$@ $^ $(LINKER)

clean:
	rm -rf $(BUILD_DIR)