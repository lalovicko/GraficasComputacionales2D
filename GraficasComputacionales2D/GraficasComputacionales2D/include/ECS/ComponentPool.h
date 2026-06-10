#pragma once
#include "Types.h"
#include "Prerequisitesr.h"
#include "SparseSet.h"

namespace ECS {
    class ComponentPool : public SparseSet {
    public:
        virtual ~ComponentPool() = default;
        virtual void RemoveEntity(EntityID entity) = 0;
        virtual void* GetRaw(EntityID entity) noexcept = 0;
    };

    template<typename T>
    class componentPool final : public ComponentPool {
    public:

        template<typename... Arg>
        T& Add(EntityID entity, Arg&&... args) {
            assert(!Contains(entity) && "La entidad ya tiene el componente");
            InsertEntity(entity);
            m_components.emplace_back(std::forward<Arg>(args)...);
            return m_components.back();
        }

  
        [[nodiscard]] T& Get(EntityID entity) noexcept {
            assert(Contains(entity) && "La entidad no tiene el componente");
            return m_components[m_sparse[GetEntityIndex(entity)]];
        }
        [[nodiscard]] const T& Get(EntityID entity) const noexcept {
            assert(Contains(entity) && "La entidad no tiene el componente");
            return m_components[m_sparse[GetEntityIndex(entity)]];
        }
        [[nodiscard]] T* TryGet(EntityID entity) noexcept {
            if (!Contains(entity)) return nullptr;
            return &m_components[m_sparse[GetEntityIndex(entity)]];
        }
        void Remove(EntityID entity) override {
            if (!Contains(entity)) return;
            const EntityIndex denseIdx = m_sparse[GetEntityIndex(entity)];

            m_components[denseIdx] = std::move(m_components.back());
            m_components.pop_back();

            SparceSet::Remove(entity);
        }

    
        void RemoveEntity(EntityID entity) override {
            Remove(entity);
        }
        void* GetRaw(EntityID entity) noexcept override {
            return TryGet(entity);
        }
        [[nodiscard]] std::vector<T>& GetComponents() noexcept {
            return m_components;
        }
        [[nodiscard]] const std::vector<T>& GetComponents() const noexcept {
            return m_components;
        }

        void Clear() override {
            m_components.clear();
            SparceSet::Clear();
        }

    private:
        std::vector<T> m_components;
    };
}