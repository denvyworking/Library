#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <stdexcept>

using namespace std;

enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};


class TimeUtil {
public:
    static string get_current_time() {
        time_t now = time(0);
        tm* local_time = localtime(&now);
        char buffer[100];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", local_time);
        return string(buffer);
    }
};

class Logger {
private:
    // говорит, что даже внутри const методов член класса можно менять
    mutable ofstream log_file;  // mutable, так как запись в файл - техническая деталь
    
public:
    Logger(const string& filename) {
        log_file.open(filename, ofstream::app);
        if (!log_file.is_open()) {
            throw runtime_error("Failed to open log file");
        }
        log(LogLevel::INFO, "Logger initialized");
    }
    
    ~Logger() {
        if (log_file.is_open()) {
            log(LogLevel::INFO, "Logger shutdown");
            log_file.close();
        }
    }
    
    void log(LogLevel level, const string& message) const {
        string level_str;
        switch(level) {
            case LogLevel::INFO:    level_str = "INFO";    break;
            case LogLevel::WARNING: level_str = "WARNING"; break;
            case LogLevel::ERROR:   level_str = "ERROR";   break;
        }
        
        string log_entry = "[" + TimeUtil::get_current_time() + "] [" + level_str + "] " + message;
        
        log_file << log_entry << endl;
        cout << log_entry << endl;
    }
};

class Book {
private:
    string title;
    string author;
    int year;
    bool is_borrowed;

public:
    Book(string title, string author, int year) 
        : title(title), author(author), year(year), is_borrowed(false) {}

    string get_title() const { return title; }
    string get_author() const { return author; }
    int get_year() const { return year; }
    bool get_is_borrowed() const { return is_borrowed; }

    void borrow() { is_borrowed = true; }
    void return_book() { is_borrowed = false; }
    string get_status() const { 
        return is_borrowed ? "borrowed" : "available"; 
    }
    
    bool operator==(const Book& other) const {
        return title == other.title && author == other.author;
    }
};

class Library {
private:
    vector<Book> books;
    mutable Logger logger;  // mutable, так как логирование не меняет состояние библиотеки

public:
    Library() : logger("library.log") {
        logger.log(LogLevel::INFO, "Library system started");
    }

    ~Library() {
        logger.log(LogLevel::INFO, "Library system shutdown");
    }

    void add_book(const Book& book) {
        for(const auto& elem : books) {
            if(elem == book) {
                string error_msg = "Book '" + book.get_title() + "' already exists";
                logger.log(LogLevel::ERROR, error_msg);
                throw runtime_error(error_msg);
            }
        }
        books.push_back(book);
        logger.log(LogLevel::INFO, "Book added: " + book.get_title());
    }

    void borrow_book(const string& title) {
        for(auto& elem : books) {
            if(elem.get_title() == title) {
                if(!elem.get_is_borrowed()) {
                    elem.borrow();
                    logger.log(LogLevel::INFO, "Book '" + title + "' borrowed");
                    return;
                }
                string error_msg = "Book '" + title + "' is already borrowed";
                logger.log(LogLevel::ERROR, error_msg);
                throw runtime_error(error_msg);
            }
        }
        string error_msg = "Book '" + title + "' not found";
        logger.log(LogLevel::ERROR, error_msg);
        throw runtime_error(error_msg);
    }

    void return_book(const string& title) {
        for(auto& elem : books) {
            if(elem.get_title() == title) {
                if(elem.get_is_borrowed()) {
                    elem.return_book();
                    logger.log(LogLevel::INFO, "Book '" + title + "' returned");
                    return;
                }
                string error_msg = "Book '" + title + "' was not borrowed";
                logger.log(LogLevel::WARNING, error_msg);
                throw runtime_error(error_msg);
            }
        }
        string error_msg = "Book '" + title + "' not found";
        logger.log(LogLevel::ERROR, error_msg);
        throw runtime_error(error_msg);
    }

    void print_books() const {
        if(books.empty()) {
            cout << "The library has no books yet." << endl;
            // можно менять в const мутодах просто, без const_cast
            logger.log(LogLevel::INFO, "Displayed empty book list");
            return;
        }
        
        cout << "List of books in the library (" << books.size() << "):" << endl;
        for(const auto& elem : books) {
            cout << "++===++ " << elem.get_title() << " (" << elem.get_author() 
                 << ", " << elem.get_year() << ") - " << elem.get_status() << endl;
        }
        
        logger.log(LogLevel::INFO, "Displayed list of " + to_string(books.size()) + " books");
    }

    vector<Book> find_by_author(const string& author) const {
        vector<Book> result;
        for(const auto& elem : books) {
            if(elem.get_author() == author) {
                result.push_back(elem);
            }
        }
        logger.log(LogLevel::INFO, 
            "Found " + to_string(result.size()) + " books by author '" + author + "'");
        return result;
    }

    void remove_book(const string& title, const string& author) {
        for(auto it = books.begin(); it != books.end(); ++it) {
            if(it->get_title() == title && it->get_author() == author) {
                if(it->get_is_borrowed()) {
                    string error_msg = "Cannot delete borrowed book '" + title + "'";
                    logger.log(LogLevel::ERROR, error_msg);
                    throw runtime_error(error_msg);
                }
                books.erase(it);
                logger.log(LogLevel::INFO, "Book '" + title + "' deleted");
                return;
            }
        }
        string error_msg = "Book '" + title + "' not found";
        logger.log(LogLevel::ERROR, error_msg);
        throw runtime_error(error_msg);
    }

    void print_stats() const {
        cout << "\nLibrary Statistics:" << endl;
        cout << "++===++ Total books: " << books.size() << endl;
        
        int borrowed = 0;
        for(const auto& book : books) {
            if(book.get_is_borrowed()) borrowed++;
        }
        cout << "++===++ Borrowed books: " << borrowed << endl;
        cout << "++===++ Available books: " << books.size() - borrowed << endl;
        
        logger.log(LogLevel::INFO, 
            "Statistics: Total=" + to_string(books.size()) + 
            ", Borrowed=" + to_string(borrowed) + 
            ", Available=" + to_string(books.size() - borrowed));
    }
};

// ==================== ТЕСТЫ ====================
void run_tests() {
    cout << "\n=== НАЧАЛО ТЕСТИРОВАНИЯ ===\n";
    
    // Тест 1: Создание книги
    Book book("Test Book", "Test Author", 2023);
    cout << "Тест 1: " << (book.get_title() == "Test Book" ? "ПРОЙДЕН" : "НЕ ПРОЙДЕН") << endl;
    
    // Тест 2: Статус книги
    cout << "Тест 2: " << (book.get_status() == "available" ? "ПРОЙДЕН" : "НЕ ПРОЙДЕН") << endl;
    
    // Тест 3: Взятие книги
    book.borrow();
    cout << "Тест 3: " << (book.get_status() == "borrowed" ? "ПРОЙДЕН" : "НЕ ПРОЙДЕН") << endl;
    
    // Тест 4: Возврат книги
    book.return_book();
    cout << "Тест 4: " << (book.get_status() == "available" ? "ПРОЙДЕН" : "НЕ ПРОЙДЕН") << endl;
    
    // Тест 5: Добавление книги в библиотеку
    Library lib;
    try {
        lib.add_book(book);
        cout << "Тест 5: ПРОЙДЕН (книга добавлена)" << endl;
    } catch (...) {
        cout << "Тест 5: НЕ ПРОЙДЕН (не удалось добавить книгу)" << endl;
    }
    
    // Тест 6: Попытка добавить дубликат
    try {
        lib.add_book(book);
        cout << "Тест 6: НЕ ПРОЙДЕН (дубликат добавлен)" << endl;
    } catch (const exception& e) {
        cout << "Тест 6: ПРОЙДЕН (" << e.what() << ")" << endl;
    }
    
    cout << "=== ТЕСТИРОВАНИЕ ЗАВЕРШЕНО ===\n\n";
}

int main() {
    run_tests();  // Сначала запускаем тесты
    
    try {
        // Затем основной код программы
        Library lib;
        
        lib.add_book(Book("1984", "Orwell", 1949));
        lib.add_book(Book("Animal Farm", "Orwell", 1945));
        
        lib.print_books();
        
    } catch (const exception& e) {
        cerr << "Ошибка: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}