#ifndef IRProtocols_hpp
#define IRProtocols_hpp

#include <Arduino.h>
#include "Iterator.hpp"

/**
 * Encapsulates protocol timings.
 *
 * To add a new protocol, add a new entry on enum Id (and on Name member),
 * and create it on IRProtocols below
 */
class IRProtocol
{
    public:

        /**
         * Arbitrary ids (and names) for each protocol
         */
        enum Id : char
        {
            Junco = 1,
            Yawl,
            Draftee,
            Ampul
        };

        IRProtocol(Id i, uint16_t hm, uint16_t hs, uint16_t bm, uint16_t b0s, uint16_t b1s, uint16_t ts, uint16_t tm)
        {
            m_id = i;
            m_headerMark = hm;
            m_headerSpace = hs;
            m_bitMark = bm;
            m_bitZeroSpace = b0s;
            m_bitOneSpace = b1s;
            m_trailSpace = ts;
            m_trailMark = tm;
        }

        IRProtocol(Id i, uint16_t hm, uint16_t hs, uint16_t bm, uint16_t b0s, uint16_t b1s)
        : IRProtocol(i, hm, hs, bm, b0s, b1s, 0, 0) {};

        Id GetId() { return m_id; };
        uint16_t HeaderMark() { return m_headerMark; }
        uint16_t HeaderSpace() { return m_headerSpace; }
        uint16_t BitMark() { return m_bitMark; }
        uint16_t BitZeroSpace() { return m_bitZeroSpace; }
        uint16_t BitOneSpace() { return m_bitOneSpace; }
        uint16_t TrailSpace() { return m_trailSpace; }
        uint16_t TrailMark() { return m_trailMark; }

        bool HasTrail() { return m_trailMark > 0 && m_trailSpace > 0; }

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

    private:
        Id  m_id;
        uint16_t m_headerMark;
        uint16_t m_headerSpace;
        uint16_t m_bitMark;
        uint16_t m_bitZeroSpace;
        uint16_t m_bitOneSpace;
        uint16_t m_trailSpace;
        uint16_t m_trailMark;
};


/**
 * Collection of protocols to encode and decode IRData.
 *
 * Add a new protocol on class constructor
 */
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
            m_protocols[m_count++] = new IRProtocol(IRProtocol::Draftee, 6050, 7350, 550, 550, 1650, 7350, 550);
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


        /**
         * Find protocol by its id.
         * 
         * @param   id  to look for
         * 
         * @return  null if not found
         */
        IRProtocol * GetProtocol(IRProtocol::Id id)
        {
            First();
            while(!IsDone())
            {
                if(Current()->GetId() == id) return Current();
                Next();
            }
            return NULL;
        }
};

/**
 * Global instance of IRProtocols
 */
IRProtocols g_irProtocols;



#endif
