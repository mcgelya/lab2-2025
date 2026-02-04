#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "alphabet_index.hpp"
#include "flat_table.hpp"
#include "hash_table.hpp"

using Clock = std::chrono::steady_clock;

struct CliOptions {
    std::string file_path;
    size_t page_size = 100;
    AlphabetIndexMode mode = AlphabetIndexMode::Words;
    std::string backend = "hash";  // hash | flat | both
    bool bench = false;
    std::vector<size_t> bench_iters = {1000, 10000, 50000};
    std::string export_csv;       // word,page mapping
    std::string export_bench_csv; // benchmark table
};

std::string ReadText(const std::string& file_path) {
    if (!file_path.empty()) {
        std::ifstream ifs(file_path);
        std::ostringstream ss;
        ss << ifs.rdbuf();
        return ss.str();
    }
    std::cout << "Введите текст (Ctrl+D для завершения ввода):\n";
    std::ostringstream ss;
    ss << std::cin.rdbuf();
    return ss.str();
}

CliOptions InteractiveDialog() {
    CliOptions opt;
    std::cout << "=== Алфавитный указатель ===\n";
    std::cout << "1) Читать текст из файла? (y/n): ";
    char c;
    std::cin >> c;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (c == 'y' || c == 'Y') {
        std::cout << "Укажите путь: ";
        std::getline(std::cin, opt.file_path);
    }
    std::cout << "2) Размер страницы (по умолчанию 100): ";
    std::string line;
    std::getline(std::cin, line);
    if (!line.empty()) opt.page_size = std::stoul(line);

    std::cout << "3) Режим (w=words, c=chars) [w]: ";
    std::getline(std::cin, line);
    if (!line.empty() && (line[0] == 'c' || line[0] == 'C')) opt.mode = AlphabetIndexMode::Chars;

    std::cout << "4) Структура (h=hash, f=flat, b=both) [h]: ";
    std::getline(std::cin, line);
    if (!line.empty()) {
        if (line[0] == 'f' || line[0] == 'F') opt.backend = "flat";
        else if (line[0] == 'b' || line[0] == 'B') opt.backend = "both";
    }

    std::cout << "5) Запустить бенчмарк? (y/n) [n]: ";
    std::getline(std::cin, line);
    if (!line.empty() && (line[0] == 'y' || line[0] == 'Y')) {
        opt.bench = true;
        std::cout << "   Числа запросов через запятую (по умолчанию 1000,10000,50000): ";
        std::getline(std::cin, line);
        if (!line.empty()) {
            opt.bench_iters.clear();
            std::stringstream ss(line);
            std::string token;
            while (std::getline(ss, token, ',')) {
                if (!token.empty())
                    opt.bench_iters.push_back(static_cast<size_t>(std::stoull(token)));
            }
            if (opt.bench_iters.empty()) opt.bench_iters = {1000, 10000, 50000};
        }
        std::cout << "   Путь для CSV с бенчмарком (пусто — в stdout): ";
        std::getline(std::cin, opt.export_bench_csv);
    }

    std::cout << "6) Экспорт разбиения в CSV (оставьте пустым, чтобы пропустить): ";
    std::getline(std::cin, opt.export_csv);
    return opt;
}

template <typename DictPtr>
void ExportCsv(const DictPtr& dict, const std::string& path) {
    if (path.empty())
        return;
    std::ofstream ofs(path);
    ofs << "word,page\n";
    auto it = dict->GetIterator();
    while (it->HasNext()) {
        auto kv = it->GetCurrentItem();
        ofs << kv.key << "," << kv.value << "\n";
        it->Next();
    }
}

template <typename DictPtr>
double Benchmark(const DictPtr& dict, const std::vector<std::string>& words, size_t iters) {
    if (words.empty() || iters == 0) return 0.0;
    auto start = Clock::now();
    volatile int acc = 0;
    for (size_t i = 0; i < iters; ++i) {
        const auto& w = words[i % words.size()];
        if (dict->ContainsKey(w)) {
            acc += dict->Get(w);
        }
    }
    auto dur = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
    (void)acc;
    return dur;
}

int main(int argc, char** argv) {
    CliOptions opt = InteractiveDialog();
    std::string text = ReadText(opt.file_path);

    std::vector<std::string> words_vec;
    {
        LexerStream lex_for_bench(text);
        std::string tok;
        while (lex_for_bench.Read(tok)) {
            words_vec.push_back(tok);
        }
    }

    auto print_dict = [](const auto& dict) {
        auto it = dict->GetIterator();
        while (it->HasNext()) {
            auto kv = it->GetCurrentItem();
            std::cout << kv.key << " -> " << kv.value << "\n";
            it->Next();
        }
    };

    struct BenchRow {
        std::string backend;
        size_t queries;
        double build_ms;
        double query_ms;
    };
    std::vector<BenchRow> bench_results;

    auto run_backend = [&](const std::string& name, auto builder) {
        auto build_start = Clock::now();
        auto dict = builder();
        double build_ms = std::chrono::duration<double, std::milli>(Clock::now() - build_start).count();
        ExportCsv(dict, opt.export_csv);
        if (opt.bench) {
            for (auto q : opt.bench_iters) {
                double ms = Benchmark(dict, words_vec, q);
                bench_results.push_back({name, q, build_ms, ms});
            }
        } else {
            print_dict(dict);
        }
    };

    if (opt.backend == "flat" || opt.backend == "both") {
        run_backend("flat", [&] {
            return BuildAlphabetIndex<FlatTable<std::string, int>>(text, opt.page_size, opt.mode);
        });
    }
    if (opt.backend == "hash" || opt.backend == "both") {
        run_backend("hash", [&] {
            return BuildAlphabetIndex<HashTable<std::string, int>>(text, opt.page_size, opt.mode);
        });
    }

    if (opt.bench && !bench_results.empty()) {
        std::ostream* out = &std::cout;
        std::unique_ptr<std::ofstream> file;
        if (!opt.export_bench_csv.empty()) {
            file = std::make_unique<std::ofstream>(opt.export_bench_csv);
            out = file.get();
        }
        (*out) << "backend,queries,build_ms,query_ms\n";
        for (const auto& row : bench_results) {
            (*out) << row.backend << "," << row.queries << "," << row.build_ms << "," << row.query_ms << "\n";
        }
    }
    return 0;
}
