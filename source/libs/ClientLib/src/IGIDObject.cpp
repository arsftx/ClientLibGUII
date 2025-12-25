#include "IGIDObject.h"
#include <memory/util.h>

wchar_t* CIGIDObject::GetName() const {
    //return m_name;
    return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x108,  wchar_t*);
}

const SCommonData *CIGIDObject::GetCommonData() const {
    return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x190, const SCommonData *);
}

const int CIGIDObject::GetUniqueId() const {
    return m_uniqueId;
}
