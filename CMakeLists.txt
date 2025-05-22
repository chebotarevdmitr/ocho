# Требуемая минимальная версия CMake
cmake_minimum_required(VERSION 3.7.2)

# Название проекта и язык
project(crc32_crack CXX)

# Устанавливаем стандарт C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Создаём исполняемый файл
add_executable(crc32_crack 
    src/main.cpp 
    src/CRC32.cpp 
    src/IO.cpp
    include/CRC32.hpp 
    include/IO.hpp
)

# Указываем путь к заголовочным файлам
target_include_directories(crc32_crack PRIVATE include)

# Настройки компилятора
if (MSVC)
    target_compile_options(crc32_crack PRIVATE /W4)
else ()
    target_compile_options(crc32_crack PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif ()
