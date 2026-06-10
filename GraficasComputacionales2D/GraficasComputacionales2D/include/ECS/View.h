#pragma once
#include "ECS/ComponentPool.h"
namespace ECS {
    // Itera solo sobre las entidades que tengan TODOS los componentes pedidos
    template<typename... Components>
    class View {
    public:
        explicit View(ComponentPool<Components>&... pools) noexcept
            : m_pool(&pools...) {
            FindSmallest(); // optimización: iterar el pool más pequeño reduce trabajo
        }

        template<typename Func>
        void Each(Func&& func) {
            if (!m_smallest) return;
            const auto& entities = m_smallest->GetEntities();
            for (std::size_t i = entities.size(); i > 0; --i) { // iteración inversa para soportar eliminaciones durante el loop
                const EntityID entity = entities[i - 1];
                if (AllHave(entity))                             // filtra entidades que no tienen todos los componentes
                    std::apply(
                        [&](auto*... pools) { func(entity, pools->Get(entity)...); }, // pasa entity + todos los componentes al callback
                        m_pool
                    );
            }
        }

        template<typename Func>
        void EachEntity(Func&& func) { // igual que Each pero sin pasar los componentes, solo el EntityID
            if (!m_smallest) return;
            const auto& entities = m_smallest->GetEntities();
            for (std::size_t i = entities.size(); i > 0; --i) {
                const EntityID entity = entities[i - 1];
                if (AllHave(entity)) func(entity);
            }
        }

        [[nodiscard]] bool   Empty() const noexcept { return !m_smallest || m_smallest->Empty(); }
        [[nodiscard]] size_t Size()  const noexcept { return m_smallest ? m_smallest->size() : 0; }

    private:
        template<std::size_t I = 0>
        void FindSmallest() noexcept {                           // recorre los pools en compile-time con recursión de templates
            if constexpr (I < sizeof...(Components)) {
                auto* pool = std::get<I>(m_pool);
                if (pool && (!m_smallest || pool->size() < m_smallest->size()))
                    m_smallest = pool;                           // guarda el pool con menos entidades
                FindSmallest<I + 1>();
            }
        }

        [[nodiscard]] bool AllHave(EntityID entity) const noexcept {
            return std::apply(
                [entity](auto*... pools) {
                    return (... && (pools && pools->Contains(entity))); // fold expression: todos deben contener la entidad
                },
                m_pool
            );
        }

    private:
        std::tuple<ComponentPool<Components>*...> m_pool;  // punteros a cada pool, uno por tipo de componente
        const SparseSet* m_smallest = nullptr;             // pool con menos entidades, punto de partida de la iteración
    };
}