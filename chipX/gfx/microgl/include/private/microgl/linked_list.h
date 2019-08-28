#pragma once

template<typename T>
struct linked_list {

public:
    struct node_t {
        T data;
        node_t * prev = nullptr;
        node_t * next = nullptr;

        node_t* predecessor() const {
            return this->prev;
        }

        node_t* successor() const {
            return this->next;
        }
    };

    explicit linked_list(bool CYCLIC = false) :
            _cyclic{CYCLIC} {};

    unsigned int size() const {
        return _size;
    }

    void clear() {
        while (size() != 0)
            removeFirst();
    }

    void addLast(node_t * node) {
        if(node==nullptr)
            return;

        if(last) {
            last->next = node;
            node->prev = last;
            last = node;
        } else
            first = last = node;

        _size+=1;

        handle_cyclic();
    }

    void resetNode(node_t * node) {
        node->prev = nullptr;
        node->next = nullptr;
    }

    void removeLast() {
        if(last==nullptr)
            return;

        if(last->prev) {
            last->prev->next = nullptr;
            last = last->prev;
        }

        _size-=1;

        handle_cyclic();
    }

    void addFirst(node_t* node) {
        if(node==nullptr)
            return;

        if(first) {
            node->next = first;
            first->prev = node;
            first = node;
        } else
            first = last = node;

        _size+=1;

        handle_cyclic();
    }

    void removeFirst() {
        if(first==nullptr)
            return;

        if(first->prev) {
            first->next->prev = nullptr;
            first = first->next;
        }

        _size-=1;

        handle_cyclic();
    }

    // we assume that node is part of the linked list
    void unlink(node_t * node) {
        if(!node)
            return;

        if(node==first)
            removeFirst();
        else if(node==last)
            removeLast();
        else {
            bool has_prev = node->prev;
            bool has_next = node->next;

            if(has_prev)
                node->prev->next = node->next;

            if(has_next)
                node->next->prev = node->prev;

            _size-=1;
        }

        handle_cyclic();
    }

    node_t * getFirst() const {
        return first;
    }

    node_t *getLast() const {
        return last;
    }

    bool isCyclic() const {
        return _cyclic;
    }

private:
    unsigned int _size = 0;
    node_t * first = nullptr;
    node_t * last = nullptr;
    bool _cyclic = false;

    void handle_cyclic() {
        if(!isCyclic())
            return;

        if(first==last)
            return;

        first->prev = last;
        last->next = first;
    }

};
