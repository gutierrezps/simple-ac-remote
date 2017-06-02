#ifndef Iterator_hpp
#define Iterator_hpp

/**
 * Simple iterator template class.
 *
 * First and Next methods must be implemented by concrete class.
 */
template <class Item> class Iterator
{
public:
    virtual void First() = 0;
    virtual void Next() = 0;
    bool IsDone() const { return m_count == m_iteratorIndex; };
    Item Current() const { return m_current; };
    uint8_t Count() { return m_count; };

protected:
    Iterator() {};
    Item m_current;
    uint8_t m_count;
    uint8_t m_iteratorIndex;

};


#endif
