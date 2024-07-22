CXX := g++
CXXFLAGS := -std=c++20 -O2

SRC_DIR := .
OBJ_DIR := build/obj
BIN_DIR := build/

SRCS := common.cpp main.cpp ollama_client.cpp comfy_client.cpp commands.cpp server.cpp db.cpp ai_client.cpp rag.cpp vecdb.cpp config.cpp imagesrv.cpp 
OBJS := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRCS))

# Final executable name
TARGET := $(BIN_DIR)/llmback


INCLUDES := -I spdlog/include/ -I cpp-httplib/ -I json/include/
LIBS := -lcurl -lssl -lcrypto -lpqxx -lpq 

# Phony targets
.PHONY: all clean

# Default target
all: $(TARGET)

# Rule to create directories
$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

# Rule to build object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Rule to build the executable
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

# Clean up
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Include dependencies
-include $(OBJS:.o=.d)