#include <algorithm>
// Подключает алгоритмы стандартной библиотеки, такие как std::copy и std::min.
// В данном коде std::copy используется для копирования данных в функции replaceLastFourBytes
// и для формирования результирующего вектора в функции hack.

#include <iostream>
// Подключает средства для работы с вводом-выводом, такие как std::cout и std::cerr.
// Используется для вывода сообщений об успехе (например, "Success in thread...")
// и ошибок в консоль в функциях hack и main.

#include <limits>
// Подключает константы для работы с числовыми типами, такие как std::numeric_limits.
// В коде std::numeric_limits<uint32_t>::max() используется для определения максимального
// значения uint32_t при переборе диапазона и как индикатор ненайденного результата.

#include <vector>
// Подключает класс std::vector для работы с динамическими массивами.
// Используется для хранения данных из файла (original, result) и управления
// списками потоков, промисов и фьючерсов в функции hack

#include <thread>
// Подключает средства для работы с потоками (многопоточность).
// В коде std::thread используется для создания и управления потоками,
// а std::thread::hardware_concurrency() — для определения числа доступных потоков.

#include <future>
// Подключает средства для асинхронного взаимодействия между потоками, такие как
// std::promise и std::future. В коде std::promise используется для передачи
// найденного значения из потока, а std::future — для получения результата в функции hack.

#include <cmath>
// Подключает математические функции, такие как std::min.
// В коде std::min используется для вычисления границ диапазона значений
// при делении общего диапазона на поддиапазоны для потоков.

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