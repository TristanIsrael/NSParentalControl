TARGET = parental_control
BUILD = build

CC = aarch64-none-elf-g++
CFLAGS = -std=c++17 -Iinclude -I$(DEVKITPRO)/libnx/include -DVERSION=1.0
CXXFLAGS = -std=c++17 -Iinclude -I$(DEVKITPRO)/libnx/include
LDFLAGS = -lnx -L$(DEVKITPRO)/libnx/lib

SRC = $(wildcard src/*.c src/*.cpp)
OBJ = $(SRC:source/%.cpp=$(BUILD)/%.o)

all: $(BUILD)/$(TARGET).nro

$(BUILD)/%.o: src/%.cpp src/%.c
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/$(TARGET).nro: $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

clean:
	rm -rf $(BUILD)/*