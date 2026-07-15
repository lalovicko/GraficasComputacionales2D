#pragma once

#include "ECS/Types.h"

namespace ECS {

class SparseSet {
public:
    SparseSet() = default;
    virtual ~SparseSet() = default;

    [[nodiscard]] bool Contains(EntityID entity) const noexcept {
        const EntityIndex sparseIndex = GetEntityIndex(entity);
        if (sparseIndex >= m_sparse.size()) {
            return false;
        }

        const EntityIndex denseIndex = m_sparse[sparseIndex];
        return denseIndex < m_dense.size() && m_dense[denseIndex] == entity;
    }

    [[nodiscard]] std::size_t Size() const noexcept { return m_dense.size(); }
    [[nodiscard]] std::size_t size() const noexcept { return Size(); }
    [[nodiscard]] bool Empty() const noexcept { return m_dense.empty(); }

    [[nodiscard]] const std::vector<EntityID>& GetEntities() const noexcept {
        return m_dense;
    }

    virtual void Remove(EntityID entity) {
        if (!Contains(entity)) {
            return;
        }

        const EntityIndex sparseIndex = GetEntityIndex(entity);
        const EntityIndex denseIndex = m_sparse[sparseIndex];
        const EntityID lastEntity = m_dense.back();

        m_dense[denseIndex] = lastEntity;
        m_sparse[GetEntityIndex(lastEntity)] = denseIndex;
        m_dense.pop_back();
        m_sparse[sparseIndex] = INVALID;
    }

    virtual void Clear() {
        m_sparse.clear();
        m_dense.clear();
    }

protected:
    EntityIndex InsertEntity(EntityID entity) {
        const EntityIndex sparseIndex = GetEntityIndex(entity);
        const EntityIndex denseIndex = static_cast<EntityIndex>(m_dense.size());

        if (sparseIndex >= m_sparse.size()) {
            m_sparse.resize(static_cast<std::size_t>(sparseIndex) + 1u, INVALID);
        }

        assert(m_sparse[sparseIndex] == INVALID && "Entity already exists in SparseSet");
        m_sparse[sparseIndex] = denseIndex;
        m_dense.push_back(entity);
        return denseIndex;
    }

protected:
    static constexpr EntityIndex INVALID = std::numeric_limits<EntityIndex>::max();
    std::vector<EntityIndex> m_sparse;
    std::vector<EntityID> m_dense;
};

} // namespace ECS
