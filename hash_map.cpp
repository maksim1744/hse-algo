#include <list>
#include <stdexcept>
#include <utility>
#include <vector>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> >
class HashMap {
 public:
    typedef typename std::list<std::pair<const KeyType, ValueType> >::iterator iterator;
    typedef typename std::list<std::pair<const KeyType, ValueType> >::const_iterator const_iterator;

    HashMap(Hash hash = Hash())
          : hash(hash),
            pointers(INITIAL_SIZE),
            states(INITIAL_SIZE, STATE_EMPTY) {
    }

    template<class InputIt>
    HashMap(InputIt first, InputIt last, Hash hash = Hash())
          : hash(hash),
            pointers(INITIAL_SIZE),
            states(INITIAL_SIZE, STATE_EMPTY) {
        for (auto it = first; it != last; ++it) {
            insert(*it);
        }
    }

    HashMap(std::initializer_list<std::pair<const KeyType, ValueType> > init, Hash hash = Hash())
          : hash(hash),
            pointers(INITIAL_SIZE),
            states(INITIAL_SIZE, STATE_EMPTY) {
        for (auto& pair : init) {
            insert(pair);
        }
    }

    HashMap(const HashMap& other, Hash hash = Hash())
          : hash(hash),
            pointers(INITIAL_SIZE),
            states(INITIAL_SIZE, STATE_EMPTY) {
        for (auto& pair : other.values) {
            insert(pair);
        }
    }

    HashMap& operator=(const HashMap& other) {
        if (this == &other) {
            return *this;
        }
        clear();
        for (auto& pair : other.values) {
            insert(pair);
        }
        return *this;
    }

    void insert(std::pair<KeyType, ValueType>&& pair);

    ValueType& operator[] (const KeyType& key);

    const ValueType& at(const KeyType& key) const;

    void erase(const KeyType& key);

    iterator find(const KeyType& key);
    const_iterator find(const KeyType& key) const;

    iterator begin() { return values.begin(); }
    iterator end() { return values.end(); }

    const_iterator begin() const { return values.begin(); }
    const_iterator end() const { return values.end(); }

    bool empty() const { return values.empty(); }

    size_t size() const { return values.size(); }

    void clear();

    Hash hash_function() const { return hash; }

 private:
    const size_t OCCUPANCY_COEFFICIENT = 4;
    const size_t INITIAL_SIZE = 8;
    const char STATE_EMPTY = 0;
    const char STATE_FILLED = 1;
    const char STATE_ERASED = 2;

    Hash hash;
    std::vector<typename std::list<std::pair<const KeyType, ValueType> >::iterator> pointers;
    std::list<std::pair<const KeyType, ValueType> > values;
    std::vector<char> states;
    size_t occupancy = 0;

    void check_size();
};


template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::insert(std::pair<KeyType, ValueType>&& pair) {
    check_size();
    size_t position = hash(pair.first) % pointers.size();
    while (states[position] == STATE_FILLED) {
        if (pointers[position]->first == pair.first) {
            return;
        }
        ++position;
        if (position >= pointers.size()) {
            position = 0;
        }
    }
    values.insert(end(), pair);
    pointers[position] = --end();
    if (states[position] == STATE_EMPTY) {
        ++occupancy;
    }
    states[position] = STATE_FILLED;
}

template<class KeyType, class ValueType, class Hash>
ValueType& HashMap<KeyType, ValueType, Hash>::operator[] (const KeyType& key) {
    check_size();
    size_t position = hash(key) % pointers.size();
    while (states[position] == STATE_ERASED || (states[position] == STATE_FILLED && !(pointers[position]->first == key))) {
        ++position;
        if (position >= pointers.size()) {
            position = 0;
        }
    }
    if (states[position] != STATE_FILLED) {
        values.insert(end(), std::make_pair(key, ValueType()));
        pointers[position] = --end();
        if (states[position] == STATE_EMPTY) {
            ++occupancy;
        }
        states[position] = STATE_FILLED;
    }
    return pointers[position]->second;
}

template<class KeyType, class ValueType, class Hash>
const ValueType& HashMap<KeyType, ValueType, Hash>::at(const KeyType& key) const {
    size_t position = hash(key) % pointers.size();
    while (states[position] == STATE_ERASED || (states[position] == STATE_FILLED && !(pointers[position]->first == key))) {
        ++position;
        if (position >= pointers.size()) {
            position = 0;
        }
    }
    if (states[position] != STATE_FILLED) {
        throw std::out_of_range("");
    }
    return pointers[position]->second;
}

template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::erase(const KeyType& key) {
    size_t position = hash(key) % pointers.size();
    while (states[position] == STATE_ERASED || (states[position] == STATE_FILLED && !(pointers[position]->first == key))) {
        ++position;
        if (position >= pointers.size()) {
            position = 0;
        }
    }
    if (states[position] == STATE_FILLED) {
        values.erase(pointers[position]);
        states[position] = STATE_ERASED;
    }
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::iterator HashMap<KeyType, ValueType, Hash>::find(const KeyType& key) {
    size_t position = hash(key) % pointers.size();
    while (states[position] == STATE_ERASED || (states[position] == STATE_FILLED && !(pointers[position]->first == key))) {
        ++position;
        if (position >= pointers.size()) {
            position = 0;
        }
    }
    if (states[position] != STATE_FILLED) {
        return end();
    }
    return pointers[position];
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::const_iterator HashMap<KeyType, ValueType, Hash>::find(const KeyType& key) const {
    size_t position = hash(key) % pointers.size();
    while (states[position] == STATE_ERASED || (states[position] == STATE_FILLED && !(pointers[position]->first == key))) {
        ++position;
        if (position >= pointers.size()) {
            position = 0;
        }
    }
    if (states[position] != STATE_FILLED) {
        return end();
    }
    return pointers[position];
}

template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::clear() {
    for (auto& pair : values) {
        size_t position = hash(pair.first) % pointers.size();
        while (states[position] != STATE_EMPTY) {
            states[position] = STATE_EMPTY;
            ++position;
            if (position >= pointers.size()) {
                position = 0;
            }
        }
    }
    values.clear();
    occupancy = 0;
}

template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::check_size() {
    if (occupancy * OCCUPANCY_COEFFICIENT >= pointers.size()) {
        try {
            std::vector<typename std::list<std::pair<const KeyType, ValueType> >::iterator> new_pointers(pointers.size() * 2);
            states.assign(new_pointers.size(), STATE_EMPTY);
            for (auto it = begin(); it != end(); ++it) {
                size_t position = hash(it->first) % new_pointers.size();
                while (states[position] == STATE_FILLED) {
                    ++position;
                    if (position >= new_pointers.size()) {
                        position = 0;
                    }
                }
                new_pointers[position] = it;
                states[position] = STATE_FILLED;
                occupancy = 0;
            }
            std::swap(pointers, new_pointers);
        } catch (std::bad_alloc) {}
    }
}
