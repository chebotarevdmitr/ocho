#include "../include/IO.hpp"
#include <fstream>
#include <stdexcept>
#include <limits>

/**
 * @brief Читает данные из файла в вектор
 * @param path Путь к файлу
 * @return Вектор байт, содержащий данные файла
 * @throws std::runtime_error если файл не удалось открыть
 */
std::vector<char> readFromFile(const char *path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open input file");
    }
    return std::vector<char>(std::istreambuf_iterator<char>(file), {});
}

/**
 * @brief Записывает данные из вектора в файл
 * @param path Путь к файлу
 * @param data Вектор байт для записи
 * @throws std::runtime_error если файл не удалось открыть или размер данных слишком велик
 */
void writeToFile(const char *path, const std::vector<char> &data) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open output file");
    }
    // Проверяем, что размер вектора не превышает максимальное значение std::streamsize
    if (data.size() > static_cast<std::vector<char>::size_type>(std::numeric_limits<std::streamsize>::max())) {
        throw std::runtime_error("Data size exceeds maximum stream size");
    }
    // Явное приведение data.size() к std::streamsize для устранения предупреждения
    file.write(data.data(), static_cast<std::streamsize>(data.size()));
}

/*
Добавлена проверка размера вектора data.size() против std::numeric_limits<std::streamsize>::max() 
для предотвращения переполнения.
Использовано явное приведение static_cast<std::streamsize>(data.size()) в вызове file.write, 
чтобы устранить предупреждение -Wsign-conversion.
Включён заголовок <limits> для использования std::numeric_limits.
Остальная часть файла осталась без изменений.
*/
