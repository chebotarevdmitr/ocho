#include "../include/IO.hpp"
#include <fstream>
#include <stdexcept>

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
 * @throws std::runtime_error если файл не удалось открыть
 */
void writeToFile(const char *path, const std::vector<char> &data) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open output file");
    }
    file.write(data.data(), data.size());
}

