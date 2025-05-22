#include <algorithm>
#include <iostream>
#include <limits>
#include <vector>
#include <thread>
#include <future>
#include <cmath>

#include "../include/CRC32.hpp"
#include "../include/IO.hpp"

/**
 * @brief Заменяет последние 4 байта вектора значением value
 * @param data Вектор, в котором заменяются байты
 * @param value Значение для замены (4 байта)
 */
void replaceLastFourBytes(std::vector<char> &data, uint32_t value) {
    std::copy_n(reinterpret_cast<const char *>(&value), 4, data.end() - 4);
}

/**
 * @brief Ищет значение для последних 4 байт, чтобы CRC32 результата совпадал с оригинальным
 * @param original Оригинальный вектор данных
 * @param injection Строка для добавления в конец данных
 * @param start Начало диапазона перебора значений
 * @param end Конец диапазона перебора значений
 * @param originalCrc32 CRC32 хеш оригинального вектора
 * @param promise Объект для возврата найденного значения (или max, если не найдено)
 */
void hackRange(const std::vector<char> &original, const std::string &injection,
               uint32_t start, uint32_t end, uint32_t originalCrc32,
               std::promise<uint32_t> &promise) {
    // Создаем копию результирующего вектора для текущего потока
    std::vector<char> result(original.size() + injection.size() + 4);
    auto it = std::copy(original.begin(), original.end(), result.begin());
    std::copy(injection.begin(), injection.end(), it);

    // Перебираем значения в заданном диапазоне
    for (uint32_t i = start; i < end && i < std::numeric_limits<uint32_t>::max(); ++i) {
        replaceLastFourBytes(result, i);
        if (crc32(result.data(), result.size()) == originalCrc32) {
            promise.set_value(i); // Устанавливаем найденное значение
            return;
        }
    }
    promise.set_value(std::numeric_limits<uint32_t>::max()); // Ничего не найдено
}

/**
 * @brief Формирует новый вектор с тем же CRC32, добавляя строку injection и 4 байта
 * @param original Оригинальный вектор данных
 * @param injection Строка для добавления
 * @return Новый вектор с совпадающим CRC32
 * @throws std::logic_error если подходящее значение не найдено
 */
std::vector<char> hack(const std::vector<char> &original, const std::string &injection) {
    const uint32_t originalCrc32 = crc32(original.data(), original.size());

    // Создаем базовый результирующий вектор
    std::vector<char> result(original.size() + injection.size() + 4);
    auto it = std::copy(original.begin(), original.end(), result.begin());
    std::copy(injection.begin(), injection.end(), it);

    // Определяем количество потоков
    const uint32_t threadCount = std::thread::hardware_concurrency();
    const uint64_t maxVal = std::numeric_limits<uint32_t>::max();
    const uint64_t rangeSize = (maxVal + threadCount - 1) / threadCount; // Делим диапазон на части

    std::vector<std::thread> threads;
    std::vector<std::promise<uint32_t>> promises(threadCount);
    std::vector<std::future<uint32_t>> futures;

    // Заполняем futures для получения результатов
    for (auto &p : promises) {
        futures.push_back(p.get_future());
    }

    // Запускаем потоки
    for (uint32_t i = 0; i < threadCount; ++i) {
        uint32_t start = static_cast<uint32_t>(i * rangeSize);
        uint32_t end = static_cast<uint32_t>(std::min(static_cast<uint64_t>((i + 1) * rangeSize), maxVal));
        threads.emplace_back(hackRange, std::cref(original), std::cref(injection),
                            start, end, originalCrc32, std::ref(promises[i]));
    }

    // Ожидаем завершения потоков и проверяем результаты
    for (auto &t : threads) {
        t.join();
    }

    // Проверяем результаты от всех потоков
    for (size_t i = 0; i < futures.size(); ++i) {
        uint32_t value = futures[i].get();
        if (value != std::numeric_limits<uint32_t>::max()) {
            std::cout << "Success in thread " << i << " with value " << value << "\n";
            replaceLastFourBytes(result, value);
            return result;
        }
    }

    throw std::logic_error("Can't hack");
}

/**
 * @brief Основная функция программы
 * @param argc Количество аргументов командной строки
 * @param argv Аргументы командной строки (входной и выходной файлы)
 * @return Код возврата (0 — успех, 1 — неверные аргументы, 2 — ошибка выполнения)
 */
int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Call with two args: " << argv[0] << " <input file> <output file>\n";
        return 1;
    }

    try {
        const std::vector<char> data = readFromFile(argv[1]);
        const std::vector<char> badData = hack(data, "He-he-he");
        writeToFile(argv[2], badData);
    } catch (std::exception &ex) {
        std::cerr << ex.what() << '\n';
        return 2;
    }
    return 0;
}