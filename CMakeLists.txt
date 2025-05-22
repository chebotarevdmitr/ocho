# Указываем минимальную версию CMake 3.10 и максимальную 3.28 для совместимости
cmake_minimum_required(VERSION 3.10...3.28)
project(crc32_crack CXX)

# Устанавливаем стандарт C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Создаём исполняемый файл crc32_crack из указанных исходных файлов
add_executable(crc32_crack
    src/main.cpp
    src/CRC32.cpp
    src/IO.cpp
)

# Указываем пути к заголовочным файлам
target_include_directories(crc32_crack PRIVATE include)

# Настраиваем флаги компиляции в зависимости от компилятора
if (MSVC)
    target_compile_options(crc32_crack PUBLIC /W4)
else ()
    target_compile_options(crc32_crack PUBLIC -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif ()

# Заменил cmake_minimum_required(VERSION 3.7.2) на VERSION 3.10...3.28,
# чтобы устранить предупреждение и обеспечить совместимость с современными версиями CMake.

# Добавил set(CMAKE_CXX_STANDARD_REQUIRED ON) для строгого соблюдения C++17.
# Явно указал директории src/ для исходных файлов и include/ для заголовочных файлов (CRC32.hpp, IO.hpp),
# предполагая стандартную структуру проекта (src/ для .cpp, include/ для .hpp).
