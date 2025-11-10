// Iterator.cpp - Helper implementations for Iterator

#include "Iterator.h"
#include "VM.h"

namespace rglite {

size_t Iterator::vmListCreate(const std::vector<Value>& items) const {
    if (!vm_) return 0;
    return vm_->getListStorage().createList(items);
}

} // namespace rglite

