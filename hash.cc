#include "hash.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <iostream>

// Stała określająca, w jakiej wersji kompilowany jest program.
#ifndef NDEBUG
    static const bool debug = true;
#else
    static const bool debug = false;
#endif



using seq_element_t = uint64_t;
using seq_t = std::vector<seq_element_t>;

struct hasher {
    hash_function_t fun;

    hasher(hash_function_t fun_) {
        fun = fun_;
    }
    std::size_t operator()(seq_t const& S) const {
        size_t res = fun(S.data(), S.size());
        return res;
    }
};

using hash_table_t = std::unordered_set<seq_t, hasher>;
using hash_tables_map_t = std::unordered_map<unsigned long, hash_table_t>;

namespace {
    static hash_tables_map_t & hash_tables_map() {
        static hash_tables_map_t * helper = new hash_tables_map_t();
        return * helper;
    }

    static unsigned long & next_id() {
        static unsigned long * helper = new unsigned long(0);
        return * helper;
    }

    unsigned long get_next_id() {
        return ++next_id();
    }

    void print_seq(const uint64_t * seq, const size_t size) {
        if (!debug) {
            return;
        }

        std::cerr << "\"";
            for (size_t i = 0; i < size; ++i) {
                std::cerr << seq[i];
                if (i < size - 1 && size > 0) {
                    std::cerr << " ";
                }
            }
        std::cerr << "\"";
    }

    void print_func_name_args(const std::string& func_name, 
    const unsigned long id, const size_t size,
    const hash_function_t hash_function,
    const uint64_t * seq) {
        if (!debug) {
            return;
        }

        std::cerr << func_name << "(";
        if (func_name == "hash_create") {
            std::cerr << &hash_function << ")\n";
            return;
        }
        if (func_name == "hash_delete" || func_name == "hash_size"
            || func_name == "hash_clear") {
            std::cerr << id << ")\n";
            return;
        }
        std::cerr << id << ", ";
        if (seq == NULL) {
            std::cerr << seq <<", ";
        }
        else {
            print_seq(seq, size);
            std::cerr << ", ";
        }
        std::cerr << size << ")\n";
    }

    void print_invalid_ptr_size(const std::string& func_name, bool null_ptr,
    bool size_is_zero) {
        if (!debug) {
            return;
        }

        if (null_ptr) {
            std::cerr << func_name << ": invalid pointer (NULL)\n";
        }
        if (size_is_zero) {
            std::cerr << func_name << ": invalid size (0)\n";
        }
    }

    void print_no_such_table(const std::string& func_name,
    const unsigned long id) {
        if (!debug) {
            return;
        }

        std::cerr << func_name << ": hash table #" << id << " does not exist\n";
    }

    void print_action(const std::string& func_name,
    const unsigned long id, bool valid) {
        if (!debug) {
            return;
        }

        std::cerr << func_name << ": hash table #" << id;

        if (func_name == "hash_delete") {
            std::cerr << " deleted\n";
        }
        else if (func_name == "hash_clear") {
            if (valid) {
                std::cerr << " cleared\n";
            }
            else {
                std::cerr << " was empty\n";
            }
        }
    }

    void print_seq_action(const std::string& func_name,
    const unsigned long id, uint64_t const * seq, size_t size, bool valid) {
        if (!debug) {
            return;
        }

        std::cerr << func_name << ": hash table #" << id << ", sequence ";
        print_seq(seq, size);

        if (func_name == "hash_insert") {
            if (valid) {
                std::cerr << " inserted\n";
            }
            else {
                std::cerr << " was present\n";
            }
        }
        else if (func_name == "hash_test") {
            if (valid) {
                std::cerr << " is present\n";
            }
            else {
                std::cerr << " is not present\n";
            }
        }
        else if (func_name == "hash_remove") {
            if (valid) {
                std::cerr << " removed\n";
            }
            else {
                std::cerr << " was not present\n";
            }
        }
    }

    void print_size(const unsigned long id, size_t size) {
        if (!debug) {
            return;
        }

        std::cerr << "hash_size: hash table #" << id <<
        " contains " << size << " element(s)\n";
    }
}

namespace jnp1 {
    // Tworzy tablicę haszującą i zwraca jej identyfikator. Parametr
    // hash_function jest wskaźnikiem na funkcję haszującą, która daje
    // w wyniku liczbę uint64_t i ma kolejno parametry
    // uint64_t const * oraz size_t. Jeżeli hash_function to NULL,
    // zwraca 0.
    unsigned long hash_create(hash_function_t hash_function) {
        print_func_name_args(__func__, 0, 0, hash_function, NULL);

        if (hash_function == NULL) {
            print_invalid_ptr_size(__func__, true, false);
            return 0;
        }
        else {
            unsigned long table_id = get_next_id();
            hash_table_t new_hash_table(0, hasher(hash_function));
            hash_tables_map().emplace(std::make_pair(table_id, new_hash_table));

            return table_id;
        }
    }

    // Usuwa tablicę haszującą o identyfikatorze id, o ile ona istnieje.
    // W przeciwnym przypadku nic nie robi.
    void hash_delete(unsigned long id) {
        if (debug) {
            print_func_name_args(__func__, id, 0, NULL, NULL);

            auto iter = hash_tables_map().find(id);
            if (iter == hash_tables_map().end()) {
                print_no_such_table(__func__, id);
            }
            else {
                hash_tables_map().erase(id);
                print_action(__func__, id, true);
            }
        }
        else {
            hash_tables_map().erase(id);
        }
    }

    // Daje liczbę ciągów przechowywanych w tablicy haszującej
    // o identyfikatorze id lub 0, jeśli taka tablica nie istnieje.
    size_t hash_size(unsigned long id) {
        print_func_name_args(__func__, id, 0, NULL, NULL);

        size_t res;
        auto iter = hash_tables_map().find(id);
        if (iter == hash_tables_map().end()) {
            res = 0;
            print_no_such_table(__func__, id);
        }
        else {
            res = iter->second.size();
            print_size(id, res);
        }

        return res;
    }

    // Wstawia do tablicy haszującej o identyfikatorze id ciąg liczb
    // całkowitych seq o długości size. Wynikiem jest informacja, czy operacja
    // się powiodła. Operacja się nie powiedzie, jeśli nie ma takiej tablicy
    // haszującej, jeśli tablica haszująca zawiera już taki ciąg, jeśli
    // parametr seq ma wartość NULL lub parametr size ma wartość 0.
    bool hash_insert(unsigned long id, uint64_t const * seq, size_t size) {
        print_func_name_args(__func__, id, size, NULL, seq);

        bool res = true;
        if (seq != NULL && size > 0) {
            auto iter_table = hash_tables_map().find(id);
            if (iter_table == hash_tables_map().end()) {
                print_no_such_table(__func__, id);
                res = false;
            }
            else {
                seq_t sequence(seq, seq + size);

                bool insertion_happened =
                (iter_table->second).emplace(sequence).second;
                
                if (!insertion_happened) {
                    res = false;
                }
                
                print_seq_action(__func__, id, seq, size,
                insertion_happened);
            }
        }
        else {
            res = false;
        }
        
        print_invalid_ptr_size(__func__, (seq == NULL), (size == 0));

        return res;
    }

    // Usuwa z tablicy haszującej o identyfikatorze id ciąg liczb całkowitych
    // seq o długości size. Wynikiem jest informacja, czy operacja się
    // powiodła. Operacja się nie powiedzie, jeśli nie ma takiej tablicy
    // haszującej, jeśli tablica haszująca nie zawiera takiego ciągu,
    // jeśli parametr seq ma wartość NULL lub parametr size ma wartość 0.
    bool hash_remove(unsigned long id, uint64_t const * seq, size_t size) {
        print_func_name_args(__func__, id, size, NULL, seq);

        bool res = true;
        if (seq != NULL && size > 0) {
            auto iter_table = hash_tables_map().find(id);
            if (iter_table == hash_tables_map().end()) {
                print_no_such_table(__func__, id);
                res = false;
            }
            else {
                seq_t sequence(seq, seq + size);
                bool was_erased = (iter_table->second).erase(sequence);
                if (!was_erased) {
                    res = false;
                }

                print_seq_action(__func__, id, seq, size,
                was_erased);
            }
        }
        else {
            res = false;
        }
        
        print_invalid_ptr_size(__func__, (seq == NULL), (size == 0));

        return res;
    }

    // Jeśli tablica haszująca o identyfikatorze id istnieje i nie jest pusta,
    // to usuwa z niej wszystkie elementy. W przeciwnym przypadku nic nie robi.
    void hash_clear(unsigned long id) {
        print_func_name_args(__func__, id, 0, NULL, NULL);

        auto iter_table = hash_tables_map().find(id);
        if (iter_table != hash_tables_map().end()) {
            // Tablica istnieje - sprawdzenie rozmiaru.
            if (iter_table->second.size() > 0) {
                iter_table->second.clear();
                print_action(__func__, id, true);
            }
            else {
                print_action(__func__, id, false);
            }
        }
        else {
            print_no_such_table(__func__, id);
        }
    }

    // Daje wynik true, jeśli istnieje tablica haszująca o identyfikatorze id
    // i zawiera ona ciąg liczb całkowitych seq o długości size. Daje wynik
    // false w przeciwnym przypadku oraz gdy parametr seq ma wartość NULL lub
    // parametr size ma wartość 0.
    bool hash_test(unsigned long id, uint64_t const * seq, size_t size) {
        print_func_name_args(__func__, id, size, NULL, seq);

        bool res = true;
        if (seq != NULL && size > 0) {
            auto iter_table = hash_tables_map().find(id);
            if (iter_table != hash_tables_map().end()) {
                seq_t sequence(seq, seq + size);
                auto iter_sequence = iter_table->second.find(sequence);
                if (iter_sequence == iter_table->second.end()) {
                    res = false;
                }

                print_seq_action(__func__, id, seq, size, res);
            }
            else {
                print_no_such_table(__func__, id);
                res = false;
            }
        }
        else {
            res = false;
        }
        
        print_invalid_ptr_size(__func__, (seq == NULL), (size == 0));
        return res;
    }
}