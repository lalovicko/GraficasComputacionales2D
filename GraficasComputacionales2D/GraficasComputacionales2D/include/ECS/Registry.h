#pragma once

#include "ECS/ComponentPool.h"
#include "ECS/System.h"
#include "ECS/View.h"

namespace ECS {

class Registry {
public:
    Registry() = default;
    Registry(const Registry&) = delete;
    Registry& operator=(const Registry&) = delete;

    EntityID CreateEntity() {
        EntityIndex index{};

        if (!m_freeList.empty()) {
            index = m_freeList.front();
            m_freeList.pop();
        } else {
            index = static_cast<EntityIndex>(m_versions.size());
            m_versions.push_back(0u);
            m_entities.push_back(NULL_ENTITY);
        }

        const EntityID entity = MakeEntityID(index, m_versions[index]);
        m_entities[index] = entity;
        return entity;
    }

    void DestroyEntity(EntityID entity) {
        assert(IsAlive(entity) && "DestroyEntity received an invalid entity");

        for (auto& [typeId, pool] : m_componentPools) {
            (void)typeId;
            pool->RemoveEntity(entity);
        }

        const EntityIndex index = GetEntityIndex(entity);
        ++m_versions[index];
        m_entities[index] = NULL_ENTITY;
        m_freeList.push(index);
    }

    [[nodiscard]] bool IsAlive(EntityID entity) const noexcept {
        if (entity == NULL_ENTITY) {
            return false;
        }

        const EntityIndex index = GetEntityIndex(entity);
        return index < m_entities.size() && m_entities[index] == entity;
    }

    [[nodiscard]] std::size_t EntityCount() const noexcept {
        return m_entities.size() - m_freeList.size();
    }

    [[nodiscard]] const std::vector<EntityID>& GetEntities() const noexcept {
        return m_entities;
    }

    template<typename T, typename... Args>
    T& AddComponent(EntityID entity, Args&&... args) {
        assert(IsAlive(entity) && "AddComponent received an invalid entity");
        return GetOrCreatePool<T>()->Add(entity, std::forward<Args>(args)...);
    }

    template<typename T>
    void RemoveComponent(EntityID entity) {
        if (auto* pool = GetPool<T>()) {
            pool->Remove(entity);
        }
    }

    template<typename T>
    T& SetComponent(EntityID entity, T value) {
        assert(IsAlive(entity) && "SetComponent received an invalid entity");
        auto* pool = GetOrCreatePool<T>();

        if (pool->Contains(entity)) {
            pool->Get(entity) = std::move(value);
            return pool->Get(entity);
        }

        return pool->Add(entity, std::move(value));
    }

    template<typename T>
    [[nodiscard]] bool HasComponent(EntityID entity) const noexcept {
        const auto* pool = GetPool<T>();
        return pool && pool->Contains(entity);
    }

    template<typename T>
    [[nodiscard]] T& GetComponent(EntityID entity) {
        assert(IsAlive(entity));
        auto* pool = GetPool<T>();
        assert(pool && "Component pool does not exist");
        return pool->Get(entity);
    }

    template<typename T>
    [[nodiscard]] const T& GetComponent(EntityID entity) const {
        assert(IsAlive(entity));
        const auto* pool = GetPool<T>();
        assert(pool && "Component pool does not exist");
        return pool->Get(entity);
    }

    template<typename T>
    [[nodiscard]] T* TryGetComponent(EntityID entity) noexcept {
        if (!IsAlive(entity)) {
            return nullptr;
        }

        auto* pool = GetPool<T>();
        return pool ? pool->TryGet(entity) : nullptr;
    }

    template<typename T>
    [[nodiscard]] const T* TryGetComponent(EntityID entity) const noexcept {
        if (!IsAlive(entity)) {
            return nullptr;
        }

        const auto* pool = GetPool<T>();
        return pool ? pool->TryGet(entity) : nullptr;
    }

    template<typename... Components>
    [[nodiscard]] View<Components...> GetView() {
        return View<Components...>(*GetOrCreatePool<Components>()...);
    }

    template<typename T, typename... Args>
    T& AddSystem(Args&&... args) {
        static_assert(std::is_base_of_v<System, T>, "T must derive from ECS::System");

        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        T& reference = *system;
        system->OnStart(*this);
        m_systems.push_back(std::move(system));
        return reference;
    }

    void UpdateSystems(float deltaTime) {
        for (auto& system : m_systems) {
            if (system->IsEnabled()) {
                system->OnUpdate(*this, deltaTime);
            }
        }
    }

    void RemoveAllSystems() {
        for (auto& system : m_systems) {
            system->OnDestroy(*this);
        }
        m_systems.clear();
    }

    void Clear() {
        RemoveAllSystems();

        for (auto& [typeId, pool] : m_componentPools) {
            (void)typeId;
            pool->Clear();
        }

        m_componentPools.clear();
        m_entities.clear();
        m_versions.clear();
        while (!m_freeList.empty()) {
            m_freeList.pop();
        }
    }

    [[nodiscard]] const std::unordered_map<ComponentTypeID,
        std::unique_ptr<IComponentPool>>& GetPools() const noexcept {
        return m_componentPools;
    }

private:
    template<typename T>
    ComponentPool<T>* GetOrCreatePool() {
        const ComponentTypeID typeId = GetComponentTypeID<T>();
        auto iterator = m_componentPools.find(typeId);

        if (iterator == m_componentPools.end()) {
            auto [newIterator, inserted] = m_componentPools.emplace(
                typeId,
                std::make_unique<ComponentPool<T>>()
            );
            (void)inserted;
            iterator = newIterator;
        }

        return static_cast<ComponentPool<T>*>(iterator->second.get());
    }

    template<typename T>
    ComponentPool<T>* GetPool() noexcept {
        const ComponentTypeID typeId = GetComponentTypeID<T>();
        const auto iterator = m_componentPools.find(typeId);
        return iterator == m_componentPools.end()
            ? nullptr
            : static_cast<ComponentPool<T>*>(iterator->second.get());
    }

    template<typename T>
    const ComponentPool<T>* GetPool() const noexcept {
        const ComponentTypeID typeId = GetComponentTypeID<T>();
        const auto iterator = m_componentPools.find(typeId);
        return iterator == m_componentPools.end()
            ? nullptr
            : static_cast<const ComponentPool<T>*>(iterator->second.get());
    }

private:
    std::vector<EntityID> m_entities;
    std::vector<EntityVersion> m_versions;
    std::queue<EntityIndex> m_freeList;
    std::unordered_map<ComponentTypeID, std::unique_ptr<IComponentPool>> m_componentPools;
    std::vector<std::unique_ptr<System>> m_systems;
};

} // namespace ECS
