# Настройки компилятора
CXX = g++
WINDRES = windres

# Флаги компиляции (C++)
CXXFLAGS = -std=c++17 -Wall

# Флаги линковки (Сборка exe)
# "-mwindows": убирает консоль на заднем плане окна
# "-static-libgcc" и "-static-libstdc++": для запуска exe на других ПК без установленного MinGW
LDFLAGS = -lgdiplus -lgdi32 -mwindows -static-libgcc -static-libstdc++

# Директории
SRC_DIR = src
BUILD_DIR = build

# Имя итогового файла
EXECUTABLE = $(BUILD_DIR)/app.exe

# Списки файлов
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SOURCES))
RC_FILE = $(SRC_DIR)/resource.rc
RES_FILE = $(BUILD_DIR)/resource.res


# Основная задача (make)
all: $(EXECUTABLE)

# Линковка (создание exe из объектных файлов и ресурсов)
$(EXECUTABLE): $(OBJECTS) $(RES_FILE)
	$(CXX) $(OBJECTS) $(RES_FILE) -o $@ $(LDFLAGS)

# Компиляция .cpp в .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Компиляция ресурсов (.rc в .res)
$(RES_FILE): $(RC_FILE) | $(BUILD_DIR)
	$(WINDRES) $< -O coff -o $@

# Создание директории build (если её нет)
$(BUILD_DIR):
	if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)


# Команда для сборки и запуска (make run)
run: all
	./$(EXECUTABLE)


# Очистка проекта от собранных файлов (make clean)
clean:
	if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
