// 基于skiplist跳表实现的key-value轻量级数据库引擎

#ifndef SKIPLIST_H
#define SKIPLIST_H

#include<iostream>
#include<cstdlib>
#include<cstring>
#include<mutex>
#include<fstream>

using namespace std;

#define STORE_FILE "store/dumpfile"

std::mutex mtx;

template<typename K, typename V>
class Node {
    public:
        Node() {};

        Node(const K&, const V&, const int&);

        ~Node();

        K get_key() const;

        V get_value() const;

        void set_value(const V&);
        // 节点的next指针数组，高度随机
        Node<K, V>** next;

        int node_level;

    private:
        K key;
        V value;
};

template<typename K, typename V>
class Skiplist {
    public:
        Skiplist(const int&);

        ~Skiplist();
        // 生成新节点的索引高度

        Node<K, V>* create_node(const K&, const V&, const int&);

        bool search_element(const K&);

        bool insert_element(const K&, const V&);

        bool delete_element(const K&);

        void display_list();

        int get_random_level();

        int get_size();

        void dump_file();

        void load_file();

    private:
        void get_kv_from_string(const string&, string&, string&);

        bool is_valid_string(const string&);

    private:
        // 跳表最大索引高度
        int max_level;
        // 跳表当前索引高度
        int skiplist_level;

        Node<K, V>* header;

        ofstream file_writer;
        ifstream file_reader;

        int element_count;        
};

template<typename K, typename V>
Node<K, V>::Node(const K& k, const V& v, const int& level) {
    this->key = k;
    this->value = v;
    this->node_level = level;

    // level + 1 因为下标从 0 开始
    this->next = new Node<K, V>* [level + 1];

    // 用 0 填充 next 指针数组区域
    memset(this->next, 0, sizeof(Node<K, V>*) * (level + 1));
}

template<typename K, typename V>
Node<K, V>::~Node() {
    delete[] next;
}

template<typename K, typename V>
K Node<K, V>::get_key() const {
    return this->key;
}

template<typename K, typename V>
V Node<K, V>::get_value() const {
    return this->value;
}

template<typename K, typename V>
void Node<K, V>::set_value(const V& value){
    this->value = value;
}

template<typename K, typename V>
Skiplist<K, V>::Skiplist(const int& max_level) {
    this->max_level = max_level;
    this->skiplist_level = 0;
    this->element_count = 0;

    K k;
    V v;
    // dummyhead
    this->header = create_node(k, v, max_level);
}

template<typename K, typename V>
Skiplist<K, V>::~Skiplist() {
    delete header;
}

template<typename K, typename V>
Node<K, V>* Skiplist<K, V>::create_node(const K& k, const V& v, const int& level) {
    return new Node<K, V>(k, v, level);
}

// Search for element in skip list 
/*
                           +------------+
                           |  select 60 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |
level 3         1+-------->10+------------------>50+           70       100
                                                   |
                                                   |
level 2         1          10         30         50|           70       100
                                                   |
                                                   |
level 1         1    4     10         30         50|           70       100
                                                   |
                                                   |
level 0         1    4   9 10         30   40    50+-->60      70       100
*/
template<typename K, typename V>
bool Skiplist<K, V>::search_element(const K& key) {
    cout << "searching element..." << endl;
    Node<K, V>* cur = header;

    // 从顶层开始尽可能向右前进，再向下
    for (int i = this->skiplist_level; i >= 0; i--) {
        while (cur->next[i] != nullptr && cur->next[i]->get_key() < key) {
            cur = cur->next[i];
        }
    }
    // 因为上一步的判断条件是 get_key() < key, 所以应该再往前一步，因为已经到底了所以直接[0]
    cur = cur->next[0];
    // 判断此节点是否为目标 key
    if (cur != nullptr && cur->get_key() == key) {
        cout << "Found key: " << key << ", value: " << cur->get_value() << endl;
        return true;
    } 
    cout << "Not found key: " << key << endl;
    return false;
}

template<typename K, typename V>
bool Skiplist<K, V>::insert_element(const K& key, const V& value) {
    // 对数据库操作前先上锁
    mtx.lock();
    Node<K, V>* cur = this->header;
    // pre 保存搜索时下降位置的节点以供后续修改节点指向 
    Node<K, V>* pre[max_level + 1];
    memset(pre, 0, sizeof(Node<K, V>*) * (max_level + 1));

    // 首先寻找待插入的位置，从顶层开始尽可能向右前进，再向下
    for (int i = this->skiplist_level; i >= 0; i--) {
        while (cur->next[i] != nullptr && cur->next[i]->get_key() < key) {
            cur = cur->next[i];
        }
        pre[i] = cur;
    }
    // 继续前进一格
    cur = cur->next[0];
    // 待插入位置节点 key 已经存在
    if (cur != nullptr && cur->get_key() == key) {
        cout << "key: " << key << " already exit, insert failed." << endl;
        // 解锁
        mtx.unlock();
        return false;
    }
    // 如果已经到达链表末尾或者 key 并不重复，可以插入新节点
    if (cur == nullptr || cur->get_key() != key) {
        int random_level = get_random_level();
        // 如果得到的随机高度比当前表高度还高，那么需要使头结点指向高出的部分
        if (random_level > skiplist_level) {
            for (int i = skiplist_level + 1; i < random_level + 1; i++) {
                // 此处先将超出部分的 pre 设为 header, 这样可以在随后的更改节点指向时一起操作
                pre[i] = header;
            }
            // 更新新的表高度
            skiplist_level = random_level;
        }
        Node<K, V>* new_node = create_node(key, value, random_level);
        // 开始插入(实际上就是更改前一个节点的 next 和新节点的 next 指向后一个节点)
        for (int i = 0; i < random_level + 1; i++) {
            new_node->next[i] = pre[i]->next[i];
            pre[i]->next[i] = new_node;
        }
        cout << "Successfully inserted key: " << key << ", value: " << value << endl;
        element_count++;
    }
    // 解锁
    mtx.unlock();
    return true;
}

template<typename K, typename V>
bool Skiplist<K, V>::delete_element(const K& key) {
    mtx.lock();
    Node<K, V>* cur = this->header;
    Node<K, V>* pre[max_level + 1];
    memset(pre, 0, sizeof(Node<K, V>*) * (max_level + 1));

    for (int i = skiplist_level; i >= 0; i--) {
        while (cur->next[i] != nullptr && cur->next[i]->get_key() < key) {
            cur = cur->next[i];
        }
        pre[i] = cur;
    }
    
    cur = cur->next[0];
    // 待删除节点存在
    if (cur != nullptr && cur->get_key() == key) {
        // 自底向上开始删除节点
        for (int i = 0; i < skiplist_level + 1; i++) {
            // 如果前一个节点 pre 的 next 并不指向待删除节点，说明该层上不存在待删除结点的索引
            // 可以直接 break 跳出循环因为更上层肯定也不存在，无需操作
            if (pre[i]->next[i] != cur) 
                break;
            pre[i]->next[i] = cur->next[i];
        }
        // 删除后可能需要降低当前表的高度
        while (skiplist_level > 0 && header->next[skiplist_level] == nullptr) {
            skiplist_level--;
        }
        cout << "Sucessfully deleted key: " << key << endl;
        element_count--;
    }
    mtx.unlock();
    return true;
}

template<typename K, typename V>
void Skiplist<K, V>::display_list() {
    cout << "********Skiplist********" << endl;
    for (int i = skiplist_level; i >= 0; i--) {
        Node<K, V>* cur = this->header->next[i];
        cout << "Level " << i << ": ";
        while (cur != nullptr) {
            cout << "[" << cur->get_key() << ":" << cur->get_value() << "]"<< " ";
            cur = cur->next[i];
        }
        cout << endl;
    }
}

template<typename K, typename V>
int Skiplist<K, V>::get_random_level() {
    int k = 1;
    while (rand() % 2) {
        k++;
    }
    k = min(k, max_level);
    return k;
}

template<typename K, typename V>
int Skiplist<K, V>::get_size() {
    return this->element_count;
}

template<typename K, typename V>
void Skiplist<K, V>::dump_file() {
    cout << "dumpping file..." << endl;
    file_writer.open(STORE_FILE);
    Node<K, V>* cur = this->header->next[0];

    while (cur != nullptr) {
        file_writer << cur->get_key() << ":" << cur->get_value() << "\n";
        cout << cur->get_key() << ":" << cur->get_value() << ";\n";
        cur = cur->next[0];
    }

    file_writer.flush();
    file_writer.close();
}

template<typename K, typename V>
void Skiplist<K, V>::load_file() {
    file_reader.open(STORE_FILE);
    cout << "loading file..." << endl;
    string line;
    string key;
    string value;
    
    while (getline(file_reader, line)) {
        get_kv_from_string(line, key, value);
        if (key.empty() || value.empty())
            continue;
        insert_element(stoi(key), value);
        cout << "key: " << key << " value: " << value << endl;
    }

    file_reader.close();
}

template<typename K, typename V>
void Skiplist<K, V>::get_kv_from_string(const string& str, string& key, string& value) {
    if (!is_valid_string(str))
        return;
    key = str.substr(0, str.find(":"));
    value = str.substr(str.find(":") + 1, str.length());
}

template<typename K, typename V>
bool Skiplist<K, V>::is_valid_string(const string& str) {
    if (str.empty())
        return false;
    if (str.find(':') == string::npos)
        return false;
    return true;
}

#endif