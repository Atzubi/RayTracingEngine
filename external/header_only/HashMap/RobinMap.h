//
// Created by sebastian on 09.12.20.
//

#ifndef DBVH_ROBINMAP_H
#define DBVH_ROBINMAP_H

namespace Atzubi::details {
    static inline std::uint64_t hash(std::uint64_t h) {
        h ^= h >> 33;
        h *= 0xff51afd7ed558ccd;
        h ^= h >> 33;
        h *= 0xc4ceb9fe1a85ec53;
        return h ^ (h >> 33);
    }

    // used for unique thread id
    typedef std::uintptr_t thread_id_t;

    // returns a unique thread id
    inline thread_id_t thread_id() {
        static __thread int x;
        return hash(reinterpret_cast<thread_id_t>(&x));
    }
}

namespace Atzubi {
    template<class K, class T>
    class RobinMap {
    private:
        struct Item {
            T item = nullptr;
            K key;
            uint64_t metaData = -1;
        };

        Item *map;
        uint64_t size;
        uint64_t itemCount;

        uint64_t hash(K key) {
            return details::hash(key);
        }

    public:
        RobinMap() {
            map = new Item[2];
            size = 2;
            itemCount = 0;
        }

        void insert(K key, T item) {
            if (size >> 1 <= itemCount) {
                Item *buffer = new Item[size << 1];
                for (int i = 0; i < size; i++) {
                    buffer[i] = map[i];
                }
                size = size << 1;
                map = buffer;
            }
            uint64_t counter = 0;
            while (map[(hash(key) + counter) % size].metaData != (uint64_t) -1) {
                if (map[(hash(key) + counter) % size].metaData < counter) {
                    T itemBuffer = map[(hash(key) + counter) % size].item;
                    K keyBuffer = map[(hash(key) + counter) % size].key;
                    uint64_t counterBuffer = map[(hash(key) + counter) % size].metaData;
                    map[(hash(key) + counter) % size].item = item;
                    map[(hash(key) + counter) % size].key = key;
                    map[(hash(key) + counter) % size].metaData = counter;
                    item = itemBuffer;
                    key = keyBuffer;
                    counter = counterBuffer;
                }
                counter++;
            }
            map[(hash(key) + counter) % size].item = item;
            map[(hash(key) + counter) % size].key = key;
            map[(hash(key) + counter) % size].metaData = counter;
            itemCount++;
        }

        T& at(K key) {
            uint64_t counter = 0;
            while (map[(hash(key) + counter) % size].key != key && counter != size) {
                counter++;
            }
            return map[(hash(key) + counter) % size].item;
        }

        bool remove(K key) {

        }

        bool remove(K key, T &item) {

        }

        T& operator[](K key) {
            return at(key);
        }
    };
}

#endif //DBVH_ROBINMAP_H
