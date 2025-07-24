#include <iostream>
#include <vector>
#include <string>
#include <fstream>   // Для работы с файлами
#include <ctime>     // Для получения времени
#include <iomanip>   // Для форматирования времени

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

// Класс "Библиотека" с улучшенным логированием
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
    // Конструктор - открываем файл лога
    Library() {
        log_file.open("library.log", ios::app); // Открываем в режиме добавления
        if(log_file.is_open()) {
            write_log(LogLevel::INFO, "Library system started");
        } else {
            cerr << "Warning: Could not open log file!" << endl;
        }
    }

    // Деструктор - закрываем файл лога
    ~Library() {
        if(log_file.is_open()) {
            write_log(LogLevel::INFO, "Library system shutdown");
            log_file.close();
        }
    }

    bool add_book(const Book& book) {
        for(const auto& elem : books) {
            if(elem == book) {
                string error_msg = "Book '" + book.get_title() + 
                                 "' already exists in the library.";
                write_log(LogLevel::ERROR, error_msg);
                cerr << "[" << get_current_time() << "] [ERROR] " << error_msg << endl;
                return false;
            }
        }
        books.push_back(book);
        string success_msg = "Book '" + book.get_title() + "' added successfully.";
        write_log(LogLevel::INFO, success_msg);
        cout << "[" << get_current_time() << "] [INFO] " << success_msg << endl;
        return true;
    }

    bool borrow_book(const string& title) {
        for(auto& elem : books) {
            if(elem.get_title() == title) {
                if(!elem.get_is_borrowed()) {
                    elem.borrow();
                    string msg = "Book '" + title + "' borrowed successfully.";
                    write_log(LogLevel::INFO, msg);
                    cout << "[" << get_current_time() << "] [INFO] " << msg << endl;
                    return true;
                }
                string warning_msg = "Book '" + title + "' is already borrowed.";
                write_log(LogLevel::WARNING, warning_msg);
                cerr << "[" << get_current_time() << "] [WARNING] " << warning_msg << endl;
                return false;
            }
        }
        string error_msg = "Book '" + title + "' not found.";
        write_log(LogLevel::ERROR, error_msg);
        cerr << "[" << get_current_time() << "] [ERROR] " << error_msg << endl;
        return false;
    }

    bool return_book(const string& title) {
        for(auto& elem : books) {
            if(elem.get_title() == title) {
                if(elem.get_is_borrowed()) {
                    elem.return_book();
                    string msg = "Book '" + title + "' returned successfully.";
                    write_log(LogLevel::INFO, msg);
                    cout << "[" << get_current_time() << "] [INFO] " << msg << endl;
                    return true;
                }
                string warning_msg = "Book '" + title + "' was not borrowed.";
                write_log(LogLevel::WARNING, warning_msg);
                cerr << "[" << get_current_time() << "] [WARNING] " << warning_msg << endl;
                return false;
            }
        }
        string error_msg = "Book '" + title + "' not found.";
        write_log(LogLevel::ERROR, error_msg);
        cerr << "[" << get_current_time() << "] [ERROR] " << error_msg << endl;
        return false;
    }

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

    vector<Book> find_by_title(const string& title) const {
        vector<Book> result;
        for(const auto& elem : books) {
            if(elem.get_title() == title) {
                result.push_back(elem);
            }
        }
        string msg = "Found " + to_string(result.size()) + " books with title '" + title + "'";
        const_cast<Library*>(this)->write_log(LogLevel::INFO, msg);
        return result;
    }

    bool remove_book(const string& title, const string& author) {
        for(auto it = books.begin(); it != books.end(); ++it) {
            if(it->get_title() == title && it->get_author() == author) {
                if(it->get_is_borrowed()) {
                    string error_msg = "Cannot delete borrowed book '" + title 
                                     + "' by '" + author + "'.";
                    write_log(LogLevel::ERROR, error_msg);
                    cerr << "[" << get_current_time() << "] [ERROR] " << error_msg << endl;
                    return false;
                }
                books.erase(it);
                string msg = "Book '" + title + "' by '" + author + "' deleted successfully.";
                write_log(LogLevel::INFO, msg);
                cout << "[" << get_current_time() << "] [INFO] " << msg << endl;
                return true;
            }
        }
        string error_msg = "Book '" + title + "' by '" + author + "' not found.";
        write_log(LogLevel::ERROR, error_msg);
        cerr << "[" << get_current_time() << "] [ERROR] " << error_msg << endl;
        return false;
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
        
        string msg = "Statistics: Total=" + to_string(books.size()) + 
                    ", Borrowed=" + to_string(borrowed) + 
                    ", Available=" + to_string(books.size() - borrowed);
        const_cast<Library*>(this)->write_log(LogLevel::INFO, msg);
    }
};

int main() {
    Library lib;
    
    lib.add_book(Book("1984", "Orwell", 1949));
    lib.add_book(Book("Animal Farm", "Orwell", 1945));
    lib.add_book(Book("Crime and Punishment", "Dostoevsky", 1866));
    lib.add_book(Book("1984", "Orwell", 1949));

    lib.borrow_book("1984");
    lib.borrow_book("1984");
    lib.print_books();

    lib.return_book("1984");
    lib.print_books();

    auto orwell_books = lib.find_by_author("Orwell");
    if(!orwell_books.empty()) {
        cout << "\nBooks by Orwell:" << endl;
        for(const auto& book : orwell_books) {
            cout << "- " << book.get_title() << " (" << book.get_year() << ")" << endl;
        }
    }

    lib.remove_book("Animal Farm", "Orwell");
    lib.remove_book("Nonexistent Book", "Nonexistent Author");

    lib.print_stats();

    return 0;
}