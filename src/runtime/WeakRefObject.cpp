/*
 * Copyright (c) 2021-present Samsung Electronics Co., Ltd
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 */

#include "Escargot.h"
#include "WeakRefObject.h"
#include "ArrayObject.h"
#include "Context.h"

namespace Escargot {

WeakRefObject::WeakRefObject(ExecutionState& state, Object* target)
    : WeakRefObject(state, state.context()->globalObject()->weakRefPrototype(), target)
{
}

WeakRefObject::WeakRefObject(ExecutionState& state, Object* proto, Object* target)
    : Object(state, proto)
    , m_target(target)
{
    ASSERT(m_target);
    m_target->addFinalizer(WeakRefObject::finalizer, this);
    addFinalizer([](Object* self, void* data) {
        WeakRefObject* s = (WeakRefObject*)self;
        if (s->m_target) {
            s->m_target->removeFinalizer(WeakRefObject::finalizer, self);
            s->m_target = nullptr;
        }
    },
                 nullptr);
}

void WeakRefObject::finalizer(Object* self, void* data)
{
    WeakRefObject* s = (WeakRefObject*)data;
    ASSERT(s->m_target);
    s->m_target = nullptr;
}

void* WeakRefObject::operator new(size_t size)
{
#ifdef NDEBUG
    static MAY_THREAD_LOCAL bool typeInited = false;
    static MAY_THREAD_LOCAL GC_descr descr;
    if (!typeInited) {
        GC_word obj_bitmap[GC_BITMAP_SIZE(WeakRefObject)] = { 0 };
        Object::fillGCDescriptor(obj_bitmap);
        descr = GC_make_descriptor(obj_bitmap, GC_WORD_LEN(WeakRefObject));
        typeInited = true;
    }
    return GC_MALLOC_EXPLICITLY_TYPED(size, descr);
#else
    return CustomAllocator<WeakRefObject>().allocate(1);
#endif
}

bool WeakRefObject::deleteOperation(ExecutionState& state)
{
    if (m_target) {
        m_target->removeFinalizer(WeakRefObject::finalizer, this);
        m_target = nullptr;
        return true;
    }
    return false;
}

} // namespace Escargot
