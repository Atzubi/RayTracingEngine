//
// Created by sebastian on 19.12.20.
//

#ifndef DBVH_TASKSTEALINGQUEUEV2_H
#define DBVH_TASKSTEALINGQUEUEV2_H

#include <atomic>
#include <thread>
#include "../Allocator/AlignedAllocator.h"

std::atomic_uint64_t count = 0;

namespace Atzubi {
    template<class T>
    class TaskStealingQueueV2 {
    private:
        static const uint16_t blockSize = 32;

        class Deque {
        private:
            struct Block {
                T data[blockSize];
                Block *next = nullptr;
                Block *previous = nullptr;
            };

            std::atomic_flag headLock{};
            Block *blockHead;
            uint64_t head;
            uint64_t localEmptyTail;
            uint64_t localFullTail;
            uint64_t headBlockSize;


            alignas(64) std::atomic_flag tailLock{};
            Block *blockTail;
            uint64_t tail;
            uint64_t localEmptyHead;
            uint64_t localFullHead;
            uint64_t tailBlockSize;

            uint64_t size;

        public:
            Deque() {
                head = 0;
                tail = 0;
                localEmptyHead = blockSize - 1;
                localEmptyTail = 0;
                localFullHead = 0;
                localFullTail = 0;
                headBlockSize = blockSize;
                tailBlockSize = blockSize;
                size = blockSize;

                headLock.clear(std::memory_order_relaxed);
                tailLock.clear(std::memory_order_relaxed);

                blockHead = new Block;

                blockHead->next = blockHead;
                blockHead->previous = blockHead;

                blockTail = blockHead;
            }

            ~Deque() {
                blockHead->previous->next = nullptr;
                Block *buffer;
                while (blockHead->next != nullptr) {
                    buffer = blockHead->next;
                    delete blockHead;
                    blockHead = buffer;
                }
                delete blockHead;
            }

            bool enqueue(T item) {
                while (tailLock.test_and_set(std::memory_order_acquire)) std::this_thread::yield();
                if (tail != localEmptyHead) {
                    blockTail->data[tail & (blockSize - 1)] = item;
                    tail++;
                    if ((tail & (blockSize - 1)) == 0) {
                        blockTail = blockTail->next;
                    }
                    tailLock.clear(std::memory_order_release);
                    return true;
                } else {
                    while (headLock.test_and_set(std::memory_order_acquire)) std::this_thread::yield();
                    size = headBlockSize + tailBlockSize - size;
                    headBlockSize = size;
                    tailBlockSize = size;
                    if (size - ((localEmptyHead + 1) - head) >= blockSize << 2) {
                        uint64_t midEmpty =
                                ((((size - ((localEmptyHead + 1) - head)) / blockSize) >> 1) - 1) * blockSize;
                        localEmptyHead += midEmpty;
                        localEmptyTail -= midEmpty;
                        blockTail->data[tail & (blockSize - 1)] = item;
                        ++tail;
                        if ((tail & (blockSize - 1)) == 0) {
                            blockTail = blockTail->next;
                        }
                        headLock.clear(std::memory_order_release);
                        tailLock.clear(std::memory_order_release);
                        return true;
                    }
                    headLock.clear(std::memory_order_release);
                    blockTail->data[blockSize - 1] = item;
                    ++tail;
                    Block *buffer = blockTail->next;
                    blockTail->next = new Block;
                    blockTail->next->next = buffer;
                    blockTail->next->next->previous = blockTail->next;
                    blockTail->next->previous = blockTail;
                    blockTail = blockTail->next;
                    tailBlockSize += blockSize;
                    localEmptyHead += blockSize;
                    tailLock.clear(std::memory_order_release);
                    return true;
                }
            }

            bool push(T item) {
                while (headLock.test_and_set(std::memory_order_acquire)) std::this_thread::yield();
                if (head != localEmptyTail) {
                    if ((head & (blockSize - 1)) == 0) {
                        blockHead = blockHead->next;
                    }
                    head--;
                    blockHead->data[head & (blockSize - 1)] = item;
                    headLock.clear(std::memory_order_release);
                    return true;
                } else {
                    headLock.clear(std::memory_order_release);
                    while (tailLock.test_and_set(std::memory_order_acquire)) std::this_thread::yield();
                    size = headBlockSize + tailBlockSize - size;
                    headBlockSize = size;
                    tailBlockSize = size;
                    if (size - (tail - localEmptyTail) >= blockSize << 2) {
                        uint64_t midEmpty =
                                ((((size - (tail - (localEmptyTail))) / blockSize + 1) >> 1) - 1) * blockSize;
                        localEmptyHead += midEmpty;
                        localEmptyTail -= midEmpty;
                        if ((head & (blockSize - 1)) == 0) {
                            blockHead = blockHead->next;
                        }
                        head--;
                        blockHead->data[head & (blockSize - 1)] = item;
                        tailLock.clear(std::memory_order_release);
                        headLock.clear(std::memory_order_release);
                        return true;
                    }
                    tailLock.clear(std::memory_order_release);
                    while (headLock.test_and_set(std::memory_order_acquire)) std::this_thread::yield();
                    Block *buffer = blockHead->previous;
                    blockHead->previous = new Block;
                    blockHead->previous->previous = buffer;
                    blockHead->previous->previous->next = blockHead->previous;
                    blockHead->previous->next = blockHead;
                    blockHead = blockHead->previous;
                    headBlockSize += blockSize;
                    localEmptyTail -= blockSize;
                    blockHead->data[blockSize - 1] = item;
                    head--;
                    headLock.clear(std::memory_order_release);
                    return true;
                }
            }

            bool pop(T &item) {
                while (headLock.test_and_set(std::memory_order_acquire)) std::this_thread::yield();
                if (head != localFullTail) {
                    item = blockHead->data[head & (blockSize - 1)];
                    head++;
                    if ((head & (blockSize - 1)) == 0) {
                        blockHead = blockHead->next;
                    }
                    headLock.clear(std::memory_order_release);
                    return true;
                } else {
                    headLock.clear(std::memory_order_release);
                    while (tailLock.test_and_set(std::memory_order_acquire)) std::this_thread::yield();
                    if (tail - head > 0) {
                        item = blockHead->data[head & (blockSize - 1)];
                        head++;
                        if ((head & (blockSize - 1)) == 0) {
                            blockHead = blockHead->next;
                        }
                        localFullTail = head + ((tail - head) >> 1);
                        localFullHead = head + ((tail - head+1) >> 1);
                        tailLock.clear(std::memory_order_release);
                        return true;
                    }else{
                        //TODO: deallocate memory

                        tailLock.clear(std::memory_order_release);
                        return false;
                    }
                }
            }

            bool steal(T &item) {
                while (tailLock.test_and_set(std::memory_order_acquire)) std::this_thread::yield();
                if (tail != localFullHead) {
                    if ((tail & (blockSize - 1)) == 0) {
                        blockTail = blockTail->previous;
                    }
                    tail--;
                    item = blockTail->data[tail & (blockSize - 1)];
                    tailLock.clear(std::memory_order_release);
                    return true;
                } else {
                    while (headLock.test_and_set(std::memory_order_acquire)) std::this_thread::yield();
                    if (tail - head > 0) {
                        if ((tail & (blockSize - 1)) == 0) {
                            blockTail = blockTail->previous;
                        }
                        tail--;
                        item = blockTail->data[tail & (blockSize - 1)];
                        localFullTail = head + ((tail - head) >> 1);
                        localFullHead = head + ((tail - head+1) >> 1);
                        headLock.clear(std::memory_order_release);
                        tailLock.clear(std::memory_order_release);
                        return true;
                    }else{
                        //TODO: deallocate memory

                        headLock.clear(std::memory_order_release);
                        tailLock.clear(std::memory_order_release);
                        return false;
                    }
                }
            }

            uint64_t getSize() {
                return tail - head;
            }
        };

        Deque **deque;
        uint16_t **randomList;
        AlignedWrapper<uint16_t> *randPos;
        uint16_t queueCount;

        std::atomic_flag registerLock{};

    public:
        TaskStealingQueueV2() {
            queueCount = 0;
            randomList = nullptr;
            randPos = nullptr;
            deque = nullptr;
            registerLock.clear();
        }

        ~TaskStealingQueueV2() {
            for (int i = 0; i < queueCount; i++) {
                delete deque[i];
                delete[] randomList[i];
            }
            delete deque;
            delete randomList;
            delete[] randPos;

            std::cout << count << std::endl;
        }

        int registerUser() {
            while (registerLock.test_and_set(std::memory_order_acquire));

            auto buffer = deque;
            deque = new Deque *[queueCount + 1];
            for (int i = 0; i < queueCount; i++) {
                deque[i] = buffer[i];
            }
            deque[queueCount] = new Deque();
            delete buffer;

            delete randomList;
            randomList = new uint16_t *[queueCount + 1];
            delete randPos;
            randPos = new AlignedWrapper<uint16_t>[queueCount + 1];
            for (int i = 0; i < queueCount + 1; i++) {
                randPos[i].data = 0;
                randomList[i] = new uint16_t[queueCount];
                for (int j = 0; j < queueCount; j++) {
                    uint16_t rand;
                    bool newRand = false;
                    while (!newRand) {
                        rand = random() % (queueCount + 1);
                        newRand = true;
                        for (int k = 0; k < j; k++) {
                            if (randomList[i][k] == rand || rand == i) {
                                newRand = false;
                            }
                        }
                    }
                    randomList[i][j] = rand;
                }
            }

            uint16_t result = queueCount++;

            registerLock.clear(std::memory_order_release);

            return result;
        }

        bool enqueue(T item, uint16_t id) {
            return deque[id]->enqueue(item);
        }

        bool push(T item, uint16_t id) {
            return deque[id]->push(item);
        }

        bool pop(T &item, uint16_t id) {
            if (deque[id]->pop(item)) {
                return true;
            } else {
                for (int i = 0; i < queueCount - 1; i++) {
                    randPos[id].data = (randPos[id].data + 1) % (queueCount - 1);
                    if (deque[randomList[id][randPos[id].data]]->steal(item)) {
                        return true;
                    }
                }
                return false;
            }
        }

        uint64_t size() {
            uint64_t size = 0;
            for (int i = 0; i < queueCount; i++) {
                size += deque[i]->getSize();
            }
            return size;
        }
    };
}

#endif //DBVH_TASKSTEALINGQUEUEV2_H
