#pragma once

#include "ECS/ComponentPool.h"

namespace ECS {

    /**
     * @brief Vista de iteración sobre entidades que poseen todos los componentes indicados.
     *
     * Busca el pool con menor cantidad de entidades (m_smallest) como punto
     * de partida para minimizar iteraciones, y filtra con AllHave() para
     * confirmar que cada entidad tenga todos los componentes requeridos.
     *
     * @tparam Components Tipos de componentes que deben tener las entidades iteradas.
     */
    template<typename... Components>
    class View {
    public:
        /**
         * @brief Construye la vista a partir de los pools de cada componente.
         * @param pools Referencias a los ComponentPool de cada tipo.
         */
        explicit View(ComponentPool<Components>&... pools) noexcept
            : m_pool(&pools...) {
            FindSmallest(); // elige el pool más pequeño para optimizar la iteración
        }

        /**
         * @brief Itera sobre las entidades válidas y llama a func con entity + componentes.
         *
         * La iteración es inversa para soportar eliminaciones durante el recorrido.
         *
         * @tparam Func Callable con firma: void(EntityID, Components&...).
         * @param func  Función a invocar por cada entidad válida.
         */
        template<typename Func>
        void Each(Func&& func) {
            if (!m_smallest) return;
            const auto& entities = m_smallest->GetEntities();
            for (std::size_t i = entities.size(); i > 0; --i) {
                const EntityID entity = entities[i - 1];
                if (AllHave(entity))
                    std::apply(
                        [&](auto*... pools) { func(entity, pools->Get(entity)...); },
                        m_pool
                    );
            }
        }

        /**
         * @brief Itera sobre las entidades válidas pasando solo el EntityID a func.
         *
         * Útil cuando no se necesita acceder a los componentes directamente.
         *
         * @tparam Func Callable con firma: void(EntityID).
         * @param func  Función a invocar por cada entidad válida.
         */
        template<typename Func>
        void EachEntity(Func&& func) {
            if (!m_smallest) return;
            const auto& entities = m_smallest->GetEntities();
            for (std::size_t i = entities.size(); i > 0; --i) {
                const EntityID entity = entities[i - 1];
                if (AllHave(entity)) func(entity);
            }
        }

        /// @brief Retorna true si no hay entidades que cumplan el filtro.
        [[nodiscard]] bool Empty() const noexcept {
            return !m_smallest || m_smallest->Empty();
        }

        /// @brief Retorna la cantidad de entidades en el pool más pequeño.
        [[nodiscard]] size_t Size() const noexcept {
            return m_smallest ? m_smallest->size() : 0;
        }

    private:
        /**
         * @brief Recorre los pools en tiempo de compilación y guarda el más pequeño.
         *
         * Usa recursión de templates con índice I para iterar la tupla m_pool.
         *
         * @tparam I Índice actual en la tupla (comienza en 0).
         */
        template<std::size_t I = 0>
        void FindSmallest() noexcept {
            if constexpr (I < sizeof...(Components)) {
                auto* pool = std::get<I>(m_pool);
                if (pool && (!m_smallest || pool->size() < m_smallest->size()))
                    m_smallest = pool;
                FindSmallest<I + 1>();
            }
        }

        /**
         * @brief Verifica que una entidad esté presente en todos los pools.
         *
         * Usa fold expression para evaluar todos los pools en una sola expresión.
         *
         * @param entity Entidad a verificar.
         * @return true si todos los pools contienen a la entidad.
         */
        [[nodiscard]] bool AllHave(EntityID entity) const noexcept {
            return std::apply(
                [entity](auto*... pools) {
                    return (... && (pools && pools->Contains(entity)));
                },
                m_pool
            );
        }

    private:
        std::tuple<ComponentPool<Components>*...> m_pool; ///< Punteros a cada pool, uno por tipo.
        const SparseSet* m_smallest = nullptr;            ///< Pool con menos entidades; punto de partida.
    };

} // namespace ECS