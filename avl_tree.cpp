#include <algorithm>

template<class ValueType>
struct Node;
template<class ValueType>
class SetIterator;

template<class ValueType>
class Set {
private:
    using Node = Node<ValueType>;

    Node* root = nullptr;
    Node* first_node = nullptr;
    size_t num_values = 0;

    void rotate_left(Node*& a) {
        //    a              b
        //   / \            / \
        //  L   b    =>    a   N
        //     / \        / \
        //    M   N      L   M
        Node* b = a->right;
        Node* m = b->left;
        b->parent = a->parent;
        a->right = m;
        if (m != nullptr) {
            m->parent = a;
        }
        b->left = a;
        a->parent = b;
        a->update_height();
        b->update_height();
        a = b;
    }

    void rotate_right(Node*& a) {
        //      a          b
        //     / \        / \
        //    b   N  =>  L   a
        //   / \            / \
        //  L   M          M   N
        Node* b = a->left;
        Node* m = b->right;
        b->parent = a->parent;
        a->left = m;
        if (m != nullptr) {
            m->parent = a;
        }
        b->right = a;
        a->parent = b;
        a->update_height();
        b->update_height();
        a = b;
    }

    void big_rotate_left(Node*& a) {
        //    a
        //   / \              c
        //  K   b            / \
        //     / \   =>     /   \
        //    c   N        a     b
        //   / \          / \   / \
        //  L   M        K   L M   N
        rotate_right(a->right);
        rotate_left(a);
    }

    void big_rotate_right(Node*& a) {
        //      a
        //     / \              c
        //    b   N            / \
        //   / \       =>     /   \
        //  K   c            b     a
        //     / \          / \   / \
        //    L   M        K   L M   N
        rotate_left(a->left);
        rotate_right(a);
    }

    void balance_node(Node*& node) {
        int balance = node->get_balance();
        if (balance == -2) {
            if (node->right->get_balance() == 1) {
                big_rotate_left(node);
            } else {
                rotate_left(node);
            }
        } else if (balance == 2) {
            if (node->left->get_balance() == -1) {
                big_rotate_right(node);
            } else {
                rotate_right(node);
            }
        }
    }

    void insert_value(Node*& node, ValueType& value, Node* parent) {
        if (node == nullptr) {
            node = new Node(value);
            node->parent = parent;
            ++num_values;
            return;
        }
        if (node->equal(value)) {
            return;
        }
        if (value < node->value) {
            insert_value(node->left, value, node);
        } else {
            insert_value(node->right, value, node);
        }
        node->update_height();
        balance_node(node);
    }

    void erase_value(Node*& node, ValueType& value) {
        if (node == nullptr) {
            return;
        }
        if (node->equal(value)) {
            if (node->left == nullptr && node->right == nullptr) {
                delete node;
                node = nullptr;
                --num_values;
                return;
            } else if (node->left == nullptr || (node->right != nullptr && node->right->height > node->left->height)) {
                Node* near = node->right;
                while (near->left != nullptr) {
                    near = near->left;
                }
                std::swap(node->value, near->value);
                erase_value(node->right, value);
            } else {
                Node* near = node->left;
                while (near->right != nullptr) {
                    near = near->right;
                }
                std::swap(node->value, near->value);
                erase_value(node->left, value);
            }
        } else if (value < node->value) {
            erase_value(node->left, value);
        } else {
            erase_value(node->right, value);
        }
        node->update_height();
        balance_node(node);
    }

    void update_first_node() {
        if (root == nullptr) {
            first_node = nullptr;
        } else {
            first_node = root;
            while (first_node->left != nullptr) {
                first_node = first_node->left;
            }
        }
    }

public:
    using iterator = SetIterator<ValueType>;

    Set() {}

    template<class InputIt>
    Set(InputIt first, InputIt last) {
        while (first != last) {
            insert(*first);
            ++first;
        }
    }

    Set(std::initializer_list<ValueType> init) {
        for (ValueType value : init) {
            insert(value);
        }
    }

    Set(Set& other) {
        *this = other;
    }

    Set& operator=(Set& other) {
        if (&other != this) {
            this->clear();
            for (ValueType value : other) {
                insert(value);
            }
        }
        return *this;
    }

    Set& operator=(Set&& other) {
        if (&other != this) {
            this->clear();
            for (ValueType value : other) {
                insert(value);
            }
        }
        return *this;
    }

    void insert(ValueType value) {
        insert_value(root, value, nullptr);
        update_first_node();
    }

    void erase(ValueType value) {
        erase_value(root, value);
        update_first_node();
    }

    iterator lower_bound(ValueType value) const {
        if (num_values == 0 || *(--end()) < value) {
            return end();
        }
        Node* node = root;
        Node* lower_bound = nullptr;
        while (node != nullptr) {
            if (node->equal(value)) {
                return iterator(node, root);
            } else if (node->value < value) {
                node = node->right;
            } else {
                lower_bound = node;
                if (node->left == nullptr) {
                    return iterator(lower_bound, root);
                } else {
                    node = node->left;
                }
            }
        }
        return iterator(lower_bound, root);
    }

    iterator find(ValueType value) const {
        iterator it = lower_bound(value);
        if (it != end() && it.equal(value)) {
            return it;
        } else {
            return end();
        }
    }

    void clear() {
        delete root;
        num_values = 0;
        root = nullptr;
        first_node = nullptr;
    }

    size_t size() const {
        return num_values;
    }

    iterator begin() const {
        return iterator(first_node, root);
    }

    iterator end() const {
        return iterator(nullptr, root);
    }

    bool empty() const {
        return num_values == 0;
    }

    ~Set() {
        clear();
    }
};

template<class ValueType>
struct Node {
    Node* left = nullptr;
    Node* right = nullptr;
    Node* parent = nullptr;
    ValueType value;
    size_t height = 0;

    Node() {}
    Node(ValueType value) : value(value), height(1) {}

    void update_height() {
        height = 1;
        if (left != nullptr) {
            height = std::max(height, left->height + 1);
        }
        if (right != nullptr) {
            height = std::max(height, right->height + 1);
        }
    }

    int get_balance() const {
        return (left == nullptr ? 0 : (int)left->height) - (right == nullptr ? 0 : right->height);
    }

    bool equal(ValueType& val) const {
        return (!(val < value) && !(value < val));
    }

    ~Node() {
        delete left;
        delete right;
    }
};

template<class ValueType>
class SetIterator {
private:
    using Node = Node<ValueType>;

    Node* node = nullptr;
    Node* root = nullptr;

public:
    SetIterator() {}
    SetIterator(Node* node, Node* root) : node(node), root(root) {}
    SetIterator(SetIterator& iterator) : node(iterator.node), root(iterator.root) {}
    SetIterator(SetIterator&& iterator) : node(iterator.node), root(iterator.root) {}
    
    SetIterator& operator=(SetIterator other) {
           node = other.node;
           root = other.root;
           return *this;
    } 

    SetIterator& operator++() {
        if (node->right != nullptr) {
            node = node->right;
            while (node->left != nullptr) {
                node = node->left;
            }
        } else {
            while (node != root && node == node->parent->right) {
                node = node->parent;
            }
            node = node->parent;
        }
        return *this;
    }

    SetIterator operator++(int) {
        SetIterator tmp(*this);
        ++*this;
        return tmp;
    }

    SetIterator& operator--() {
        if (node == nullptr) {
            node = root;
            while (node->right != nullptr) {
                node = node->right;
            }
        } else if (node->left != nullptr) {
            node = node->left;
            while (node->right != nullptr) {
                node = node->right;
            }
        } else {
            while (node == node->parent->left) {
                node = node->parent;
            }
            node = node->parent;
        }
        return *this;
    }

    SetIterator operator--(int) {
        SetIterator tmp(*this);
        --*this;
        return tmp;
    }

    const ValueType& operator*() const {
        return node->value;
    }
    const ValueType* operator->() const {
        return &(node->value);
    }

    bool operator==(SetIterator<ValueType> other) const {
        return node == other.node && root == other.root;
    }
    bool operator!=(SetIterator<ValueType> other) const {
        return !(node == other.node && root == other.root);
    }
    
    bool equal(ValueType& value) const {
           return node->equal(value);
    } 
};
