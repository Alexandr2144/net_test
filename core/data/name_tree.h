#pragma once
#ifndef DATA_NAME_TREE_H
#define DATA_NAME_TREE_H

#include "core/data/hash_map.h"
#include "core/data/string.h"

#include "core/memory/containers.h"
#include "core/memory/plain.h"
#include "core/tools/stream.h"


namespace Data {
    class NameTree {
    public:
        struct Node {
            HashMap<String, Node*> next;
            Array<String> pass;
            String filter;
            size_t id;

        public:
            Node(size_t id, String filter);
        };

        struct Iterator {
            size_t id;
            Node* node;
            Node** link;
            Array<String> pass;

        public:
            Iterator(Ref<Node> node);
            Iterator();

            bool find(Ref<Tools::IStream<String>> fullname, String filter = String());
            bool next(Ref<Tools::IStream<String>> fullname, String filter = String());
            size_t step(String s, String filter = String());
        };

        struct SeqReader {
            Memory::RaStack<String> strings;

            void addString(Memory::IAllocator& a, String s);
            Array<String> run(Memory::IAllocator& a, Ref<Tools::IStream<String>> stream);
        };

    public:
        NameTree(size_t rootId);

        Iterator find(Ref<Tools::IStream<String>> fullname, String filter = String());

        bool insert(Ref<Tools::IStream<String>> fullname, size_t id, String filter = String(), bool doReplace = false);
        size_t remove(Ref<Tools::IStream<String>> fullname);

    private:
        Memory::ChainAllocator m_strings;
        Memory::Pool m_pool;
        SeqReader m_reader;
        Node* m_root;

    private:
        Node* new_node(size_t id, String filter);
    };

} // namespace Data


#endif // DATA_NAME_TREE_H