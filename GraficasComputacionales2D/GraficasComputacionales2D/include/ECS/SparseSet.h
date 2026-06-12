#pragma once

#include "Types.h"
#include "Prerequisitesr.h"

namespace ECS {

    /**
     * @brief Contenedor de entidades con búsqueda, inserción y eliminación en O(1).
     *
     * Usa dos arrays paralelos:
     * - m_sparse: indexado por EntityIndex, apunta a la posición en m_dense.
     * - m_dense: array compacto con las entidades activas.
     *
     * La eliminación usa swap & pop para mantener m_dense contiguo.
     */
    class SparseSet {
    public:
        SparseSet() = default;
        virtual ~SparseSet() = default;

        /**
         * @brief Verifica si una entidad está en el set.
         *
         * Valida tanto el rango del índice como la versión del EntityID.
         *
         * @param entity Entidad a buscar.
         * @return true si la entidad está presente y es válida.
         */
        [[nodiscard]] bool Contains(EntityID entity) const noexcept {
            const EntityIndex idx = GetEntityIndex(entity);
            if (idx >= m_sparse.size()) return false;
            const EntityIndex denseIdx = m_sparse[idx];
            return denseIdx < m_dense.size() && m_dense[denseIdx] == entity;
        }

        /// @brief Retorna la cantidad de entidades activas en el set.
        [[nodiscard]] size_t size() const noexcept { return m_dense.size(); }

        /// @brief Retorna true si no hay entidades activas.
        [[nodiscard]] bool Empty() const noexcept { return m_dense.empty(); }

        /// @brief Retorna el array denso con todas las entidades activas.
        [[nodiscard]] const std::vector<EntityID>& GetEntities() const noexcept {
            return m_dense;
        }

        /**
         * @brief Elimina una entidad del set usando swap & pop.
         *
         * Mueve la última entidad al hueco dejado por la eliminada
         * para mantener m_dense contiguo en memoria.
         *
         * @param entity Entidad a eliminar.
         */
        virtual void Remove(EntityID entity) {
            if (!Contains(entity)) return;
            const EntityIndex sparseIdx = GetEntityIndex(entity);
            const EntityIndex denseIdx = m_sparse[sparseIdx];
            const EntityID    last = m_dense.back();
            m_dense[denseIdx] = last;         // mueve el último al hueco
            m_sparse[GetEntityIndex(last)] = denseIdx;     // actualiza el sparse del movido
            m_dense.pop_back();
            m_sparse[sparseIdx] = INVALID;                 // marca celda como vacía
        }

        /// @brief Vacía completamente el set.
        virtual void Clear() {
            m_sparse.clear();
            m_dense.clear();
        }

    protected:
        /**
         * @brief Registra una entidad en el set y retorna su posición en m_dense.
         *
         * Expande m_sparse si el índice de la entidad supera el tamaño actual.
         *
         * @param entity Entidad a insertar (no debe existir ya en el set).
         * @return Índice en m_dense donde quedó registrada la entidad.
         */
        EntityIndex InsertEntity(EntityID entity) {
            const EntityIndex sparseIdx = GetEntityIndex(entity);
            const EntityIndex denseIdx = static_cast<EntityIndex>(m_dense.size());
            if (sparseIdx >= m_sparse.size())
                m_sparse.resize(sparseIdx + 1, INVALID);     // expande con centinelas
            assert(m_sparse[sparseIdx] == INVALID && "Entity already exists in the set");
            m_sparse[sparseIdx] = denseIdx;
            m_dense.push_back(entity);
            return denseIdx;
        }

    protected:
        /// @brief Valor centinela que indica celda vacía en m_sparse.
        static constexpr EntityIndex INVALID = std::numeric_limits<EntityIndex>::max();

        std::vector<EntityIndex> m_sparse; ///< Índice directo: EntityIndex ? posición en m_dense.
        std::vector<EntityID>    m_dense;  ///< Array compacto con las entidades activas.
    };

} // namespace ECS