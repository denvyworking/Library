#include <iostream>
#include <vector>
#include <string>

using namespace std;

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
public:
    bool add_book(const Book& book) {
        for(const auto& elem : books) {
            if(elem == book) {
                cerr << "Error: Book '" << book.get_title() 
                     << "' already exists in the library." << endl;
                return false;
            }
        }
        books.push_back(book);
        cout << "Book '" << book.get_title() << "' added successfully." << endl;
        return true;
    }

    bool borrow_book(const string& title) {
        for(auto& elem : books) {
            if(elem.get_title() == title) {
                if(!elem.get_is_borrowed()) {
                    elem.borrow();
                    cout << "Book '" << title << "' borrowed successfully." << endl;
                    return true;
                }
                cerr << "Error: Book '" << title << "' is already borrowed." << endl;
                return false;
            }
        }
        cerr << "Error: Book '" << title << "' not found." << endl;
        return false;
    }

    bool return_book(const string& title) {
        for(auto& elem : books) {
            if(elem.get_title() == title) {
                if(elem.get_is_borrowed()) {
                    elem.return_book();
                    cout << "Book '" << title << "' returned successfully." << endl;
                    return true;
                }
                cerr << "Error: Book '" << title << "' was not borrowed." << endl;
                return false;
            }
        }
        cerr << "Error: Book '" << title << "' not found." << endl;
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
    }

    vector<Book> find_by_author(const string& author) const {
        vector<Book> result;
        for(const auto& elem : books) {
            if(elem.get_author() == author) {
                result.push_back(elem);
            }
        }
        return result;
    }

    vector<Book> find_by_title(const string& title) const {
        vector<Book> result;
        for(const auto& elem : books) {
            if(elem.get_title() == title) {
                result.push_back(elem);
            }
        }
        return result;
    }

    bool remove_book(const string& title, const string& author) {
        for(auto it = books.begin(); it != books.end(); ++it) {
            if(it->get_title() == title && it->get_author() == author) {
                if(it->get_is_borrowed()) {
                    cerr << "Error: Cannot delete borrowed book '" << title 
                         << "' by '" << author << "'." << endl;
                    return false;
                }
                books.erase(it);
                cout << "Book '" << title << "' by '" << author << "' deleted successfully." << endl;
                return true;
            }
        }
        cerr << "Error: Book '" << title << "' by '" << author << "' not found." << endl;
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