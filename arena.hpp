#pragma once
#include <cstddef>
#include <iostream>
class ArenaAllocater
{
public:
    explicit ArenaAllocater(size_t bytes)
        : m_size(bytes)
    {
        m_buffer = static_cast<std::byte*>(malloc(m_size));
        m_offset = m_buffer;
    }
    template<typename T>
    T* alloc()
    {
        void* offset = m_offset;
        m_offset += sizeof(T);
        return static_cast<T*>(offset);
    }

    ArenaAllocater(const ArenaAllocater& other) = delete;

    ArenaAllocater operator=(const ArenaAllocater& other) = delete;

    ~ArenaAllocater()
    {
        free(m_buffer);
    }
private:
    size_t m_size;
    std::byte* m_buffer;
    std::byte* m_offset;
};
