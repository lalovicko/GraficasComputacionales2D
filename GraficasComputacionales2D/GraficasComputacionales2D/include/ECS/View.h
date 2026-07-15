#pragma once

#include "ECS/ComponentPool.h"

namespace ECS {

template<typename... Components>
class View {
public:
    explicit View(ComponentPool<Components>&... pools) noexcept
        : m_pools(&pools...) {
        FindSmallest();
    }

    template<typename Func>
    void Each(Func&& function) {
        if (!m_smallest) {
            return;
        }

        const auto& entities = m_smallest->GetEntities();
        for (std::size_t i = entities.size(); i > 0; --i) {
            const EntityID entity = entities[i - 1u];
            if (!AllHave(entity)) {
                continue;
            }

            std::apply(
                [&](auto*... pools) {
                    function(entity, pools->Get(entity)...);
                },
                m_pools
            );
        }
    }

    template<typename Func>
    void EachEntity(Func&& function) {
        if (!m_smallest) {
            return;
        }

        const auto& entities = m_smallest->GetEntities();
        for (std::size_t i = entities.size(); i > 0; --i) {
            const EntityID entity = entities[i - 1u];
            if (AllHave(entity)) {
                function(entity);
            }
        }
    }

    [[nodiscard]] bool Empty() const noexcept {
        return !m_smallest || m_smallest->Empty();
    }

    [[nodiscard]] std::size_t Size() const noexcept {
        return m_smallest ? m_smallest->Size() : 0u;
    }

private:
    template<std::size_t Index = 0u>
    void FindSmallest() noexcept {
        if constexpr (Index < sizeof...(Components)) {
            auto* pool = std::get<Index>(m_pools);
            if (pool && (!m_smallest || pool->Size() < m_smallest->Size())) {
                m_smallest = pool;
            }
            FindSmallest<Index + 1u>();
        }
    }

    [[nodiscard]] bool AllHave(EntityID entity) const noexcept {
        return std::apply(
            [entity](auto*... pools) {
                return (... && pools->Contains(entity));
            },
            m_pools
        );
    }

private:
    std::tuple<ComponentPool<Components>*...> m_pools;
    const SparseSet* m_smallest{ nullptr };
};

} // namespace ECS
