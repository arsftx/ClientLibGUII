#include "MsgStreamBuffer.h"

GlobalVar<CMemPool<(_TAG('MSG', 1)), CMsgStreamBuffer::SMsgStreamNode, 20, 1>, 0xA00EB0> CMsgStreamBuffer::SMsgStreamNode::m_mempool; //ECSRO

// IsOpcodeSupported_MAYBE .text 00841780 000000B4 00000008 00000008 R . . . . T .
bool IsOpcodeSupported(WORD msgid, int a2) {
    return reinterpret_cast<bool (__cdecl *)(WORD, int)>(0x5E4220)(msgid, a2); //ECSRO
}

CMsgStreamBuffer::CMsgStreamBuffer(WORD msgid) {
    field_0xc = 0;
    m_msgid = msgid;

    SMsgStreamNode *node = new SMsgStreamNode;

    m_node1 = node;
    m_node2 = node;
    m_availableBytesForReading = 0;
    m_currentReadBytes = 0;
}

void *CMsgStreamBuffer::SMsgStreamNode::operator new(std::size_t sz) {
    return m_mempool->alloc();
}

CMsgStreamBuffer::SMsgStreamNode::SMsgStreamNode() {
    data[0] = 0;
    currentPos = &data[1];
}

void CMsgStreamBuffer::Write(const void *data, size_t size) {
    reinterpret_cast<void(__thiscall *)(CMsgStreamBuffer *, const void *, size_t)>(0x41AB90)(this, data, size); //ECSRO
}

CMsgStreamBuffer &CMsgStreamBuffer::operator<<(const std::n_string &str) {
    (*this) << (WORD) str.length();
    Write(str.c_str(), str.length());

    return *this;
}

CMsgStreamBuffer &CMsgStreamBuffer::operator<<(const std::string &str) {
    (*this) << (WORD) str.length();
    Write(str.c_str(), str.length());

    return *this;
}

void CMsgStreamBuffer::ToggleBefore() {
    if (field_0xc != 1) {
        field_0xc = 1;
        m_node2 = m_node1;
        m_availableBytesForReading = 0;
    }
}

void CMsgStreamBuffer::ToggleAfter() {
    if (field_0xc != 0) {
        field_0xc = 0;
        m_node2 = m_node1;
        m_currentReadBytes = 0;
    }
}

CMsgStreamBuffer::~CMsgStreamBuffer() {
    reinterpret_cast<void(__thiscall *)(CMsgStreamBuffer *)>(0x41AAD0)(this); //ECSRO
}

WORD CMsgStreamBuffer::msgid() const {
    return m_msgid;
}

CMsgStreamBuffer &CMsgStreamBuffer::operator>>(std::n_string &str) {
    return reinterpret_cast<CMsgStreamBuffer &(__thiscall *) (CMsgStreamBuffer *, std::n_string &)>(0x004b6290)(this, str);
}

void CMsgStreamBuffer::Read(void *value, size_t numBytes) {
    reinterpret_cast<void(__thiscall *)(CMsgStreamBuffer *, void *, size_t)>(0x004b5da0)(this, value, numBytes);
}

void CMsgStreamBuffer::FlushRemaining() {
    m_currentReadBytes = m_availableBytesForReading;
}
