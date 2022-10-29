#include "hash.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <iostream>
// #include <pair>

// Set flag indicating whether to print diagnostic messages
// or not.
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
// using hash_table_id_t = unsigned long;
using hash_tables_map_t = std::unordered_map<unsigned long, hash_table_t>;
// using func_arg_long_t = std::pair<unsigned long, bool>;
// using func_arg_size_t = std::pair<size_t, bool>;
// using func_arg_hash_t = std::pair<hash_function_t, bool>;
// using func_arg_seq_t = std::pair<uint64_t const*, bool>; 

namespace {
    static hash_tables_map_t hash_tables_map;

    // Debug functions:
    void print_func_name_args(const std::string& func_name, 
    const unsigned long id, const size_t size,
    const hash_function_t hash_function,
    const uint64_t * seq) {
        std::cerr << func_name << "(";
        if (func_name == "hash_create") {
            std::cerr << hash_function << ")\n";
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
            std::cerr << "\"";
            for (size_t i = 0; i < size; ++i) {
                std::cerr << seq[i];
                if (i < size - 1 && size > 0) {
                    std::cerr << " ";
                }
            }
            std::cerr << "\", ";
        }
        std::cerr << size << ")\n";
    }

    void print_invalid_ptr_size(const std::string& func_name, bool null_ptr, bool size_is_zero) {
        if (null_ptr) {
            std::cerr << func_name << ": invalid pointer (NULL)\n";
        }
        if (size_is_zero) {
            std::cerr << func_name << ": invalid size (0)\n";
        }
    }

}

namespace jnp1 {
    // Tworzy tablicę haszującą i zwraca jej identyfikator. Parametr
    // hash_function jest wskaźnikiem na funkcję haszującą, która daje
    // w wyniku liczbę uint64_t i ma kolejno parametry
    // uint64_t const * oraz size_t.
    unsigned long hash_create(hash_function_t hash_function) {
        if (debug) {
            print_func_name_args(__func__, 0, 0, hash_function, NULL);
        }
        static unsigned long table_id = 0;
        hash_table_t new_hash_table(0, hasher(hash_function));
        hash_tables_map.emplace(std::make_pair(table_id, new_hash_table));

        return table_id++;
    }

    // Usuwa tablicę haszującą o identyfikatorze id, o ile ona istnieje.
    // W przeciwnym przypadku nic nie robi.
    void hash_delete(unsigned long id) {
        if (debug) {
            print_func_name_args(__func__, id, 0, NULL, NULL);
        }
        hash_tables_map.erase(id);
        // TODO: komunikat o usunięciu.
    }

    // Daje liczbę ciągów przechowywanych w tablicy haszującej
    // o identyfikatorze id lub 0, jeśli taka tablica nie istnieje.
    size_t hash_size(unsigned long id) {
        if (debug) {
            print_func_name_args(__func__, id, 0, NULL, NULL);
        }
        size_t res;
        auto iter = hash_tables_map.find(id);
        if (iter == hash_tables_map.end()) {
            res = 0;
        }
        else {
            res = iter->second.size();
        }

        // TODO: komunikat o rozmiarze lub nieistnieniu.
        return res;
    }

    // Wstawia do tablicy haszującej o identyfikatorze id ciąg liczb
    // całkowitych seq o długości size. Wynikiem jest informacja, czy operacja
    // się powiodła. Operacja się nie powiedzie, jeśli nie ma takiej tablicy
    // haszującej, jeśli tablica haszująca zawiera już taki ciąg, jeśli
    // parametr seq ma wartość NULL lub parametr size ma wartość 0.
    bool hash_insert(unsigned long id, uint64_t const * seq, size_t size) {
        if (debug) {
            print_func_name_args(__func__, id, size, NULL, seq);
        }
        bool res = true;
        if (seq != NULL && size > 0) {
            auto iter_table = hash_tables_map.find(id);
            if (iter_table == hash_tables_map.end()) {
                res = false;
            }
            else {
                seq_t sequence(seq, seq + size);

                bool insertion_happened =
                (iter_table->second).emplace(sequence).second;
                
                if (!insertion_happened) {
                    // Sequence was already present!
                    res = false;
                }
            }
        }
        else {
            res = false;
        }
        if (debug) {
            print_invalid_ptr_size(__func__, (seq == NULL), (size == 0));
        }
        // TODO: komunikat o wyniku insercji.
        return res;
    }

    // Usuwa z tablicy haszującej o identyfikatorze id ciąg liczb całkowitych
    // seq o długości size. Wynikiem jest informacja, czy operacja się
    // powiodła. Operacja się nie powiedzie, jeśli nie ma takiej tablicy
    // haszującej, jeśli tablica haszująca nie zawiera takiego ciągu,
    // jeśli parametr seq ma wartość NULL lub parametr size ma wartość 0.
    bool hash_remove(unsigned long id, uint64_t const * seq, size_t size) {
        if (debug) {
            print_func_name_args(__func__, id, size, NULL, seq);
        }
        bool res = true;
        if (seq != NULL && size > 0) {
            auto iter_table = hash_tables_map.find(id);
            if (iter_table == hash_tables_map.end()) {
                // No such table!
                res = false;
            }
            else {
                seq_t sequence(seq, seq + size);
                bool was_erased = (iter_table->second).erase(sequence);
                if (!was_erased) {
                    // No such sequence in the table!
                    res = false;
                }
            }
        }
        else {
            res = false;
        }
        if (debug) {
            print_invalid_ptr_size(__func__, (seq == NULL), (size == 0));
        }
        //TODO: komunikat o usunięciu.
        return res;
    }

    // Jeśli tablica haszująca o identyfikatorze id istnieje i nie jest pusta,
    // to usuwa z niej wszystkie elementy. W przeciwnym przypadku nic nie robi.
    void hash_clear(unsigned long id) {
        if (debug) {
            print_func_name_args(__func__, id, 0, NULL, NULL);
        }
        auto iter_table = hash_tables_map.find(id);
        if (iter_table != hash_tables_map.end()) {
            // Table exists, check size.
            if (iter_table->second.size() > 0) {
                iter_table->second.clear();
            }
        }
        //TODO: komunikat o usunięciu / zerowym rozmiarze / nieistnieniu.
        return;
    }

    // Daje wynik true, jeśli istnieje tablica haszująca o identyfikatorze id
    // i zawiera ona ciąg liczb całkowitych seq o długości size. Daje wynik
    // false w przeciwnym przypadku oraz gdy parametr seq ma wartość NULL lub
    // parametr size ma wartość 0.
    bool hash_test(unsigned long id, uint64_t const * seq, size_t size) {
        if (debug) {
            print_func_name_args(__func__, id, size, NULL, seq);
        }
        bool res = true;
        if (seq != NULL && size > 0) {
            // Check if table exists.
            auto iter_table = hash_tables_map.find(id);
            if (iter_table != hash_tables_map.end()) {
                // Check if table contains sequence.
                seq_t sequence(seq, seq + size);
                auto iter_sequence = iter_table->second.find(sequence);
                if (iter_sequence == iter_table->second.end()) {
                    // Table does not contain sequence!
                    res = false;
                }
            }
            else {
                res = false;
            }
        }
        else {
            res = false;
        }

        if (debug) {
            print_invalid_ptr_size(__func__, (seq == NULL), (size == 0));
        }
        //TODO: komunikaty o nieistnieniu tablicy, istnieniu lub nie ciągu.
        return res;
    }
}

