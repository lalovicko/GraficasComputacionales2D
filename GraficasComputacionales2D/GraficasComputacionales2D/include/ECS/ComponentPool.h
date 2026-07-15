#pragma once

#include "ECS/SparseSet.h"

namespace ECS {

class IComponentPool : public SparseSet {
public:
    ~IComponentPool() override = default;
    virtual void RemoveEntity(EntityID entity) = 0;
    virtual void* GetRaw(EntityID entity) noexcept = 0;
};

template<typename T>
class ComponentPool final : public IComponentPool {
public:
    template<typename... Args>
    T& Add(EntityID entity, Args&&... args) {
        assert(!Contains(entity) && "Entity already owns this component");
        InsertEntity(entity);
        m_components.emplace_back(std::forward<Args>(args)...);
        return m_components.back();
    }

    [[nodiscard]] T& Get(EntityID entity) noexcept {
        assert(Contains(entity) && "Entity does not own this component");
        return m_components[m_sparse[GetEntityIndex(entity)]];
    }

    [[nodiscard]] const T& Get(EntityID entity) const noexcept {
        assert(Contains(entity) && "Entity does not own this component");
        return m_components[m_sparse[GetEntityIndex(entity)]];
    }

    [[nodiscard]] T* TryGet(EntityID entity) noexcept {
        return Contains(entity) ? &Get(entity) : nullptr;
    }

    [[nodiscard]] const T* TryGet(EntityID entity) const noexcept {
        return Contains(entity) ? &Get(entity) : nullptr;
    }

    void Remove(EntityID entity) override {
        if (!Contains(entity)) {
            return;
        }

        const EntityIndex denseIndex = m_sparse[GetEntityIndex(entity)];
        if (denseIndex != m_components.size() - 1u) {
            m_components[denseIndex] = std::move(m_components.back());
        }
        m_components.pop_back();
        SparseSet::Remove(entity);
    }

    void RemoveEntity(EntityID entity) override { Remove(entity); }
    void* GetRaw(EntityID entity) noexcept override { return TryGet(entity); }

    [[nodiscard]] std::vector<T>& GetComponents() noexcept { return m_components; }
    [[nodiscard]] const std::vector<T>& GetComponents() const noexcept { return m_components; }

    void Clear() override {
        m_components.clear();
        SparseSet::Clear();
    }

private:
    std::vector<T> m_components;
};

} // namespace ECS
