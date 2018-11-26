#include "core/data/name_tree.h"

#include "core/tools/utils.h"
#include <memory.h>


namespace Data {
    // ------------------------------------------------------------------------
    static size_t set_id(Tools::IStream<String>* fullname, NameTree::Iterator const& iter, String_CRef filter)
    {
        if (iter.node->id == SIZE_MAX) {
            return iter.id;
        }
        if (!isEmpty(filter) && iter.node->filter != filter) {
            return iter.id;

        }

        if (fullname) fullname->save();
        return iter.node->id;
    }

    // ------------------------------------------------------------------------
    // NameTree::PathReader implementation
    // ------------------------------------------------------------------------
    void NameTree::SeqReader::addString(Memory::IAllocator& a, String s)
    {
        size_t length = count(s);
        Bytes mem = a.alloc(length + 1);

        memcpy(mem.begin, s.begin, length);
        mem.begin[length] = '\0';

        strings.append(String({ mem.begin, mem.end - 1 }));
    }

    // ------------------------------------------------------------------------
    Array<String> NameTree::SeqReader::run(Memory::IAllocator& a, Ref<Tools::IStream<String>> stream)
    {
        if (stream->finished()) {
            return Array<String>();
        }

        strings.reset();
        while (!stream->finished()) {
            String str = stream->read();
            addString(a, str);
        }

        Array<String> buffer = strings.asArray();
        Array<String> result = Tools::newArray<String>(&a, count(buffer));
        memcpy(result.begin, buffer.begin, size(buffer));

        return result;
    }

    // ------------------------------------------------------------------------
    // NameTree::Iterator implementation
    // ------------------------------------------------------------------------
    NameTree::Iterator::Iterator(Ref<Node> node)
        : node(&node.ref), link(nullptr), id(SIZE_MAX), pass(node->pass)
    {
    }

    // ------------------------------------------------------------------------
    NameTree::Iterator::Iterator()
        : node(nullptr), link(nullptr), id(SIZE_MAX)
    {
    }

    // ------------------------------------------------------------------------
    bool NameTree::Iterator::find(Ref<Tools::IStream<String>> fullname, String filter)
    {
        if (isEmpty(filter) || node->filter == filter) {
            id = node->id;
        }
        while (node && !fullname->finished()) {
            if (!next(&fullname, filter)) {
                return false;
            }
        }
        return true;
    }

    // ------------------------------------------------------------------------
    bool NameTree::Iterator::next(Ref<Tools::IStream<String>> fullname, String filter)
    {
        while (!isEmpty(pass) && !fullname->finished()) {
            String name = fullname->read();
            if (name != *pass.begin) {
                return false;
            }

            pass.begin += 1;
        }
        id = isEmpty(pass) ? set_id(&fullname.ref, *this, filter) : id;

        if (!fullname->finished()) {
            String name = fullname->read();
            Node** pNext = node->next.lookup(name);
            if (pNext == nullptr) {
                return false;
            }

            link = pNext;
            node = *pNext;
            pass = node->pass;
            id = isEmpty(pass) ? set_id(&fullname.ref, *this, filter) : id;
        }
        return true;
    }

    // ------------------------------------------------------------------------
    size_t NameTree::Iterator::step(String s, String filter)
    {
        if (!isEmpty(pass)) {
            if (s != *pass.begin) {
                return id;
            }

            pass.begin += 1;
        }
        else {
            Node** pNext = node->next.lookup(s);
            if (pNext == nullptr) {
                return id;
            }

            node = *pNext;
            pass = node->pass;
        }

        id = isEmpty(pass) ? set_id(nullptr, *this, filter) : id;
        return id;
    }

    // ------------------------------------------------------------------------
    // NameTree::Node implementation
    // ------------------------------------------------------------------------
    NameTree::Node::Node(size_t id, String filter)
        : id(id), filter(filter)
    {
    }

    // ------------------------------------------------------------------------
    // NameTree implementation
    // ------------------------------------------------------------------------
    NameTree::NameTree(size_t rootId)
        : m_root(nullptr)
    {
        m_root = new_node(rootId, String());
    }

    // ------------------------------------------------------------------------
    NameTree::Iterator NameTree::find(Ref<Tools::IStream<String>> fullname, String filter)
    {
        Iterator iter(m_root);
        iter.find(&fullname, filter);
        fullname->restore();
        return iter;
    }

    // ------------------------------------------------------------------------
    bool NameTree::insert(Ref<Tools::IStream<String>> fullname, size_t id, String filter, bool doReplace)
    {
        Iterator iter(m_root);
        if (!iter.find(&fullname)) {
            fullname->restore();
        }

        Node* b = iter.node;
        Array<String> p1 = Array<String>(b->pass.begin, iter.pass.begin);
        Array<String> p2 = Array<String>(iter.pass.begin, b->pass.end);

        if (!isEmpty(p2)) {
            Node* c = nullptr;
            if (!fullname->finished()) {
                String name = fullname->read();

                Node* q = new_node(id, filter);
                q->pass = m_reader.run(m_strings, fullname);
                c = new_node(SIZE_MAX, String());
                c->next.insert(name, q);
            }
            else {
                c = new_node(id, filter);
            }

            b->pass = pop_front(p2);
            c->next.insert(p2.front(), b);
            c->pass = p1;

            *iter.link = c;
            return true;
        }
        else if (!fullname->finished()) {
            String name = fullname->read();

            Node* q = new_node(id, filter);
            q->pass = m_reader.run(m_strings, fullname);
            b->next.insert(name, q);
            return true;
        }
        else if (doReplace || b->id == SIZE_MAX) {
            b->filter = filter;
            b->id = id;
            return true;
        }
        return false;
    }

    // ------------------------------------------------------------------------
    size_t NameTree::remove(Ref<Tools::IStream<String>> fullname)
    {
        Iterator iter(m_root);
        if (!iter.find(&fullname)) {
            return false;
        }
        if (!isEmpty(iter.pass)) {
            return false;
        }

        // TODO: rm node from tree
        iter.node->id = SIZE_T_MAX;
        return true;
    }

    // ------------------------------------------------------------------------
    auto NameTree::new_node(size_t id, String filter) -> Node*
    {
        auto node = m_pool.alloc<Node>();
        new(node) Node(id, filter);
        return node;
    }

} // namespace Data