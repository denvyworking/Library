#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <stdexcept>
#include <memory>
#include <algorithm>

using namespace std;

// ==================== ENUM УРОВНЯ ЛОГИРОВАНИЯ ====================
enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

// ==================== УТИЛИТА ВРЕМЕНИ ====================
class TimeUtil {
public:
    static string get_current_time() {
        time_t now = time(0);
        tm* local_time = localtime(&now);
        char buffer[100];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", local_time);
        // создаёт объект типа std::string
        return string(buffer);
    }
};

// ==================== ИНТЕРФЕЙС ЛОГГЕРА ====================
class LoggerInterface {
public:
    virtual ~LoggerInterface() = default;
    virtual void log(LogLevel level, const string& message) = 0;
};

// ==================== ЛОГГЕР В ФАЙЛ ====================
class FileLogger : public LoggerInterface {
private:
    mutable ofstream log_file;

public:
    FileLogger(const string& filename) {
        log_file.open(filename, ofstream::app);
        if (!log_file.is_open()) {
            throw runtime_error("Failed to open log file: " + filename);
        }
        log(LogLevel::INFO, "FileLogger initialized");
    }

    ~FileLogger() override {
        if (log_file.is_open()) {
            log(LogLevel::INFO, "FileLogger shutdown");
            log_file.close();
        }
    }

    void log(LogLevel level, const string& message) override {
        string level_str;
        switch(level) {
            case LogLevel::INFO:    level_str = "INFO";    break;
            case LogLevel::WARNING:    level_str = "WARNING"; break;
            case LogLevel::ERROR:   level_str = "ERROR";   break;
        }

        string log_entry = "[" + TimeUtil::get_current_time() + "] [" + level_str + "] " + message;
        log_file << log_entry << endl;
    }
};

// ==================== ЛОГГЕР В КОНСОЛЬ ====================
class ConsoleLogger : public LoggerInterface {
public:
    void log(LogLevel level, const string& message) override {
        string level_str;
        switch(level) {
            case LogLevel::INFO:    level_str = "INFO";    break;
            case LogLevel::WARNING:    level_str = "WARNING"; break;
            case LogLevel::ERROR:   level_str = "ERROR";   break;
        }

        string log_entry = "[" + TimeUtil::get_current_time() + "] [" + level_str + "] " + message;
        cout << log_entry << endl;
    }
};

// ==================== МНОГОКАНАЛЬНЫЙ ЛОГГЕР (файл + консоль) ====================
class MultiLogger : public LoggerInterface {
private:
    vector<shared_ptr<LoggerInterface>> loggers;

public:
    void add_logger(shared_ptr<LoggerInterface> logger) {
        loggers.push_back(logger);
    }

    void log(LogLevel level, const string& message) override {
        for (const auto& logger : loggers) {
            logger->log(level, message);
        }
    }
};

// ==================== КНИГА ====================
class Book {
private:
    string title;
    string author;
    int year;
    bool is_borrowed;

public:
    Book(string title, string author, int year)
        : title(move(title)), author(move(author)), year(year), is_borrowed(false) {}

    const string& get_title() const { return title; }
    const string& get_author() const { return author; }
    int get_year() const { return year; }
    bool is_borrowed_status() const { return is_borrowed; }

    void borrow() { is_borrowed = true; }
    void return_book() { is_borrowed = false; }

    string get_status() const {
        return is_borrowed ? "borrowed" : "available";
    }

    bool operator==(const Book& other) const {
        return title == other.title && author == other.author;
    }
};

// ==================== ХРАНИЛИЩЕ КНИГ ====================
class BookRepository {
protected:
    vector<Book> books;

public:
    virtual ~BookRepository() = default;

    virtual void add_book(const Book& book) {
        for (const auto& b : books) {
            if (b == book) {
                throw runtime_error("Book already exists: " + book.get_title());
            }
        }
        books.push_back(book);
    }

    virtual bool has_book(const string& title) const {
        return find_if(books.begin(), books.end(),
                       [&title](const Book& b) { return b.get_title() == title; }) != books.end();
    }

    virtual Book* find_book(const string& title) {
        auto it = find_if(books.begin(), books.end(),
                          [&title](const Book& b) { return b.get_title() == title; });
        return (it != books.end()) ? &(*it) : nullptr;
    }

    virtual const vector<Book>& get_all_books() const {
        return books;
    }

    virtual void remove_book(const Book& book) {
        books.erase(
            remove_if(books.begin(), books.end(),
                      [&book](const Book& b) { return b == book; }),
            books.end()
        );
    }

    virtual vector<Book> find_by_author(const string& author) const {
        vector<Book> result;
        for (const auto& b : books) {
            if (b.get_author() == author) {
                result.push_back(b);
            }
        }
        return result;
    }
};

// ==================== СЕРВИС БИБЛИОТЕКИ ====================
class LibraryService {
private:
    shared_ptr<BookRepository> repository;
    shared_ptr<LoggerInterface> logger;

public:
    LibraryService(shared_ptr<BookRepository> repo, shared_ptr<LoggerInterface> log)
        : repository(move(repo)), logger(move(log)) {
        logger->log(LogLevel::INFO, "LibraryService initialized");
    }

    void add_book(const Book& book) {
        try {
            repository->add_book(book);
            logger->log(LogLevel::INFO, "Book added: " + book.get_title());
        } catch (const exception& e) {
            logger->log(LogLevel::ERROR, "Failed to add book: " + string(e.what()));
            throw;
        }
    }

    void borrow_book(const string& title) {
        Book* book = repository->find_book(title);
        if (!book) {
            string msg = "Book not found: " + title;
            logger->log(LogLevel::ERROR, msg);
            throw runtime_error(msg);
        }
        if (book->is_borrowed_status()) {
            string msg = "Book already borrowed: " + title;
            logger->log(LogLevel::ERROR, msg);
            throw runtime_error(msg);
        }
        book->borrow();
        logger->log(LogLevel::INFO, "Book borrowed: " + title);
    }

    void return_book(const string& title) {
        Book* book = repository->find_book(title);
        if (!book) {
            string msg = "Book not found: " + title;
            logger->log(LogLevel::ERROR, msg);
            throw runtime_error(msg);
        }
        if (!book->is_borrowed_status()) {
            string msg = "Book was not borrowed: " + title;
            logger->log(LogLevel::WARNING, msg);
        } else {
            book->return_book();
            logger->log(LogLevel::INFO, "Book returned: " + title);
        }
    }

    void remove_book(const string& title) {
        Book* book = repository->find_book(title);
        if (!book) {
            string msg = "Book not found: " + title;
            logger->log(LogLevel::ERROR, msg);
            throw runtime_error(msg);
        }
        if (book->is_borrowed_status()) {
            string msg = "Cannot remove borrowed book: " + title;
            logger->log(LogLevel::ERROR, msg);
            throw runtime_error(msg);
        }
        repository->remove_book(*book);
        logger->log(LogLevel::INFO, "Book removed: " + title);
    }

    vector<Book> find_by_author(const string& author) const {
        auto result = repository->find_by_author(author);
        logger->log(LogLevel::INFO, "Found " + to_string(result.size()) + " books by author '" + author + "'");
        return result;
    }

    const vector<Book>& get_all_books() const {
        return repository->get_all_books();
    }

    size_t get_total_books() const {
        return repository->get_all_books().size();
    }

    size_t get_borrowed_count() const {
        size_t count = 0;
        for (const auto& book : repository->get_all_books()) {
            if (book.is_borrowed_status()) count++;
        }
        return count;
    }
};

// ==================== ИНТЕРФЕЙС ОТОБРАЖЕНИЯ ====================
class DisplayInterface {
public:
    virtual ~DisplayInterface() = default;
    virtual void show_message(const string& msg) = 0;
    virtual void show_books(const vector<Book>& books) = 0;
    virtual void show_stats(size_t total, size_t borrowed) = 0;
};

// ==================== ОТОБРАЖЕНИЕ В КОНСОЛИ ====================
class ConsoleDisplay : public DisplayInterface {
public:
    void show_message(const string& msg) override {
        cout << msg << endl;
    }

    void show_books(const vector<Book>& books) override {
        if (books.empty()) {
            cout << "The library has no books yet." << endl;
            return;
        }
        cout << "List of books in the library (" << books.size() << "):" << endl;
        for (const auto& book : books) {
            cout << "++===++ " << book.get_title() << " ("
                 << book.get_author() << ", " << book.get_year()
                 << ") - " << book.get_status() << endl;
        }
    }

    void show_stats(size_t total, size_t borrowed) override {
        cout << "\nLibrary Statistics:" << endl;
        cout << "++===++ Total books: " << total << endl;
        cout << "++===++ Borrowed books: " << borrowed << endl;
        cout << "++===++ Available books: " << (total - borrowed) << endl;
    }
};

// ==================== СЕРВИС ОТЧЁТОВ ====================
class ReportService {
private:
    shared_ptr<LibraryService> service;
    shared_ptr<DisplayInterface> display;
    shared_ptr<LoggerInterface> logger;

public:
    ReportService(shared_ptr<LibraryService> svc,
                  shared_ptr<DisplayInterface> disp,
                  shared_ptr<LoggerInterface> log)
        : service(move(svc)), display(move(disp)), logger(move(log)) {}

    void print_books() const {
        const auto& books = service->get_all_books();
        display->show_books(books);
        logger->log(LogLevel::INFO, "Displayed " + to_string(books.size()) + " books");
    }

    void print_stats() const {
        size_t total = service->get_total_books();
        size_t borrowed = service->get_borrowed_count();
        display->show_stats(total, borrowed);
        logger->log(LogLevel::INFO, "Stats: Total=" + to_string(total) + ", Borrowed=" + to_string(borrowed));
    }
};

// ==================== ТЕСТЫ ====================
void run_tests() {
    cout << "\n=== НАЧАЛО ТЕСТИРОВАНИЯ ===\n";

    auto logger = make_shared<ConsoleLogger>();
    auto repo = make_shared<BookRepository>();
    auto service = make_shared<LibraryService>(repo, logger);
    auto display = make_shared<ConsoleDisplay>();
    auto reporter = make_shared<ReportService>(service, display, logger);

    // Тест 1: Добавление книги
    Book book("Test Book", "Test Author", 2023);
    try {
        service->add_book(book);
        cout << "Тест 1: ПРОЙДЕН (книга добавлена)" << endl;
    } catch (...) {
        cout << "Тест 1: НЕ ПРОЙДЕН" << endl;
    }

    // Тест 2: Дубликат
    try {
        service->add_book(book);
        cout << "Тест 2: НЕ ПРОЙДЕН (дубликат добавлен)" << endl;
    } catch (const exception&) {
        cout << "Тест 2: ПРОЙДЕН (дубликат отклонён)" << endl;
    }

    // Тест 3: Взятие книги
    try {
        service->borrow_book("Test Book");
        cout << "Тест 3: ПРОЙДЕН (книга взята)" << endl;
    } catch (...) {
        cout << "Тест 3: НЕ ПРОЙДЕН" << endl;
    }

    // Тест 4: Возврат
    try {
        service->return_book("Test Book");
        cout << "Тест 4: ПРОЙДЕН (книга возвращена)" << endl;
    } catch (...) {
        cout << "Тест 4: НЕ ПРОЙДЕН" << endl;
    }

    // Тест 5: Статистика
    reporter->print_stats();
    cout << "Тест 5: ПРОЙДЕН (статистика выведена)" << endl;

    cout << "=== ТЕСТИРОВАНИЕ ЗАВЕРШЕНО ===\n\n";
}

// ==================== ОСНОВНАЯ ФУНКЦИЯ ====================
int main() {
    run_tests();

    try {
        // Логгер: в файл и в консоль
        auto file_logger = make_shared<FileLogger>("library.log");
        auto console_logger = make_shared<ConsoleLogger>();
        auto multi_logger = make_shared<MultiLogger>();
        multi_logger->add_logger(file_logger);
        multi_logger->add_logger(console_logger);

        auto repo = make_shared<BookRepository>();
        auto service = make_shared<LibraryService>(repo, multi_logger);
        auto display = make_shared<ConsoleDisplay>();
        auto reporter = make_shared<ReportService>(service, display, multi_logger);

        // Добавляем книги
        service->add_book(Book("1984", "Orwell", 1949));
        service->add_book(Book("Animal Farm", "Orwell", 1945));

        // Показываем список
        reporter->print_books();

        // Показываем статистику
        reporter->print_stats();

        // Берём книгу
        service->borrow_book("1984");

        // Снова статистика
        reporter->print_stats();

    } catch (const exception& e) {
        cerr << "Ошибка: " << e.what() << endl;
        return 1;
    }

    return 0;
}