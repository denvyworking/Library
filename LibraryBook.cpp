#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <stdexcept>  // Для стандартных исключений

using namespace std;

// Enum для уровней логирования
enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

// Функция для получения текущего времени в формате [YYYY-MM-DD HH:MM:SS]
string get_current_time() {
    time_t now = time(0);
    tm* local_time = localtime(&now);
    char buffer[100];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", local_time);
    return string(buffer);
}

// Класс "Книга"
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

// Класс "Библиотека" с улучшенным логированием и обработкой исключений
class Library {
private:
    vector<Book> books;
    ofstream log_file;

    // Улучшенный метод для записи в лог с уровнями
    void write_log(LogLevel level, const string& message) {
        string level_str;
        switch(level) {
            case LogLevel::INFO:    level_str = "INFO";    break;
            case LogLevel::WARNING: level_str = "WARNING"; break;
            case LogLevel::ERROR:   level_str = "ERROR";   break;
        }
        
        string log_entry = "[" + get_current_time() + "] [" + level_str + "] " + message;
        
        if(log_file.is_open()) {
            log_file << log_entry << endl;
        }
        // Также выводим в консоль для удобства
        cout << log_entry << endl;
    }

public:
    // Конструктор - открываем файл лога с обработкой ошибок
    Library() {
        try {
            log_file.open("library.log", ios::app);
            if(!log_file.is_open()) {
                throw runtime_error("Failed to open log file");
            }
            write_log(LogLevel::INFO, "Library system started");
        } 
        catch (const exception& e) {
            cerr << "CRITICAL ERROR (logging): " << e.what() << endl;
            // Можно добавить fallback-логирование в консоль
        }
    }

    // Деструктор - закрываем файл лога
    ~Library() {
        if(log_file.is_open()) {
            write_log(LogLevel::INFO, "Library system shutdown");
            log_file.close();
        }
    }

    // Добавление книги с генерацией исключения при ошибке
    void add_book(const Book& book) {
        for(const auto& elem : books) {
            if(elem == book) {
                string error_msg = "Book '" + book.get_title() + "' already exists";
                write_log(LogLevel::ERROR, error_msg);
                throw runtime_error(error_msg);
            }
        }
        books.push_back(book);
        write_log(LogLevel::INFO, "Book added: " + book.get_title());
    }

    // Взятие книги с обработкой ошибок
    void borrow_book(const string& title) {
        for(auto& elem : books) {
            if(elem.get_title() == title) {
                if(!elem.get_is_borrowed()) {
                    elem.borrow();
                    write_log(LogLevel::INFO, "Book '" + title + "' borrowed");
                    return;
                }
                string error_msg = "Book '" + title + "' is already borrowed";
                write_log(LogLevel::ERROR, error_msg);
                throw runtime_error(error_msg);
            }
        }
        string error_msg = "Book '" + title + "' not found";
        write_log(LogLevel::ERROR, error_msg);
        throw runtime_error(error_msg);
    }

    // Возврат книги с обработкой ошибок
    void return_book(const string& title) {
        for(auto& elem : books) {
            if(elem.get_title() == title) {
                if(elem.get_is_borrowed()) {
                    elem.return_book();
                    write_log(LogLevel::INFO, "Book '" + title + "' returned");
                    return;
                }
                string error_msg = "Book '" + title + "' was not borrowed";
                write_log(LogLevel::WARNING, error_msg);
                throw runtime_error(error_msg);
            }
        }
        string error_msg = "Book '" + title + "' not found";
        write_log(LogLevel::ERROR, error_msg);
        throw runtime_error(error_msg);
    }

    // Вывод списка книг
    void print_books() const {
        if(books.empty()) {
            cout << "The library has no books yet." << endl;
            return;
        }
        
        cout << "List of books in the library (" << books.size() << "):" << endl;
        for(const auto& elem : books) {
            cout << "++===++ " << elem.get_title() << " (" << elem.get_author() 
                 << ", " << elem.get_year() << ") - " << elem.get_status() << endl;
        }
        
        string msg = "Displayed list of " + to_string(books.size()) + " books";
        const_cast<Library*>(this)->write_log(LogLevel::INFO, msg);
    }

    // Поиск по автору
    vector<Book> find_by_author(const string& author) const {
        vector<Book> result;
        for(const auto& elem : books) {
            if(elem.get_author() == author) {
                result.push_back(elem);
            }
        }
        string msg = "Found " + to_string(result.size()) + " books by author '" + author + "'";
        const_cast<Library*>(this)->write_log(LogLevel::INFO, msg);
        return result;
    }

    // Удаление книги с обработкой ошибок
    void remove_book(const string& title, const string& author) {
        for(auto it = books.begin(); it != books.end(); ++it) {
            if(it->get_title() == title && it->get_author() == author) {
                if(it->get_is_borrowed()) {
                    string error_msg = "Cannot delete borrowed book '" + title + "'";
                    write_log(LogLevel::ERROR, error_msg);
                    throw runtime_error(error_msg);
                }
                books.erase(it);
                write_log(LogLevel::INFO, "Book '" + title + "' deleted");
                return;
            }
        }
        string error_msg = "Book '" + title + "' not found";
        write_log(LogLevel::ERROR, error_msg);
        throw runtime_error(error_msg);
    }

    // Статистика библиотеки
    void print_stats() const {
        cout << "\nLibrary Statistics:" << endl;
        cout << "++===++ Total books: " << books.size() << endl;
        
        int borrowed = 0;
        for(const auto& book : books) {
            if(book.get_is_borrowed()) borrowed++;
        }
        cout << "++===++ Borrowed books: " << borrowed << endl;
        cout << "++===++ Available books: " << books.size() - borrowed << endl;
        
        string msg = "Statistics: Total=" + to_string(books.size()) + 
                    ", Borrowed=" + to_string(borrowed) + 
                    ", Available=" + to_string(books.size() - borrowed);
        const_cast<Library*>(this)->write_log(LogLevel::INFO, msg);
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