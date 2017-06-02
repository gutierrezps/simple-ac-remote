#ifndef IRProtocols_hpp
#define IRProtocols_hpp

#include <Arduino.h>
#include "Iterator.hpp"

class IRProtocol
{
    public:
        enum Id : char
        {
            Junco = 1,
            Yawl,
            Draftee,
            Ampul
        };

    private:
        Id  m_id;
        uint16_t m_headerMark;
        uint16_t m_headerSpace;
        uint16_t m_bitMark;
        uint16_t m_bitZeroSpace;
        uint16_t m_bitOneSpace;

    public:

        IRProtocol(Id i, uint16_t hm, uint16_t hs, uint16_t bm, uint16_t b0s, uint16_t b1s)
        {
            m_id = i;
            m_headerMark = hm;
            m_headerSpace = hs;
            m_bitMark = bm;
            m_bitZeroSpace = b0s;
            m_bitOneSpace = b1s;
        }

        Id GetId() { return m_id; };
        uint16_t HeaderMark() { return m_headerMark; }
        uint16_t HeaderSpace() { return m_headerSpace; }
        uint16_t BitMark() { return m_bitMark; }
        uint16_t BitZeroSpace() { return m_bitZeroSpace; }
        uint16_t BitOneSpace() { return m_bitOneSpace; }
        

        String Name()
        {
            switch(m_id)
            {
                case Junco: return String(F("Junco"));
                case Yawl: return String(F("Yawl"));
                case Draftee: return String(F("Draftee"));
                case Ampul: return String(F("Ampul"));
                default: return String(F("Unknown"));
            }
        }

};

class IRProtocols : public Iterator<IRProtocol *>
{
    private:
        IRProtocol *m_protocols[10];

    public:
        IRProtocols()
        {
            m_count = 0;     // from Iterator

            m_protocols[m_count++] = new IRProtocol(IRProtocol::Junco, 9000, 4500, 560, 600, 1690);
            m_protocols[m_count++] = new IRProtocol(IRProtocol::Yawl, 3400, 1650, 425, 425, 1250);
            m_protocols[m_count++] = new IRProtocol(IRProtocol::Draftee, 500, 1700, 450, 650, 1700);
            m_protocols[m_count++] = new IRProtocol(IRProtocol::Ampul, 4400, 4400, 500, 600, 1650);
        }

        void First()        // from Iterator
        {
            if(m_count == 0) return;

            m_current = m_protocols[0];
            m_iteratorIndex = 0;
        }

        void Next()        // from Iterator
        {
            if(IsDone()) return;
            m_iteratorIndex++;
            if(IsDone()) return;
            m_current = m_protocols[m_iteratorIndex];
        }

        using Iterator<IRProtocol *>::Count;
        using Iterator<IRProtocol *>::IsDone;
        using Iterator<IRProtocol *>::Current;

        IRProtocol * GetProtocol(IRProtocol::Id i)
        {
            First();
            while(!IsDone())
            {
                if(Current()->GetId() == i) return Current();
                Next();
            }
            return NULL;
        }
};

IRProtocols g_irProtocols;



#endif
