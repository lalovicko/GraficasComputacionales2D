#pragma once

#include "Types.h"
#include "Prerequisitesr.h"
#include "SparseSet.h"

namespace ECS {

    /**
     * @brief Interfaz base para pools de componentes de cualquier tipo.
     *
     * Permite almacenar pools de distintos tipos T en el mismo contenedor
     * (e.g. unordered_map<ComponentTypeID, unique_ptr<IComponentPool>>).
     */
    class IComponentPool : public SparseSet {
    public:
        virtual ~IComponentPool() = default;

        /// @brief Elimina el componente asociado a la entidad dada.
        /// @param entity Entidad cuyo componente se eliminará.
        virtual void RemoveEntity(EntityID entity) = 0;

        /**
         * @brief Retorna un puntero void al componente de la entidad.
         *
         * Útil para herramientas de inspección o serialización genérica.
         *
         * @param entity Entidad objetivo.
         * @return Puntero al componente, o nullptr si no existe.
         */
        virtual void* GetRaw(EntityID entity) noexcept = 0;
    };

    /**
     * @brief Pool tipado que almacena componentes de tipo T.
     *
     * Los componentes se guardan de forma contigua en m_components,
     * en el mismo orden que las entidades en m_dense (heredado de SparseSet).
     * Esto garantiza buena localidad de caché durante la iteración.
     *
     * @tparam T Tipo del componente a almacenar.
     */
    template<typename T>
    class ComponentPool final : public IComponentPool {
    public:
        /**
         * @brief Agrega un componente a la entidad, construyéndolo in-place.
         * @tparam Arg  Tipos de los argumentos del constructor de T.
         * @param entity Entidad a la que se le agrega el componente.
         * @param args   Argumentos reenviados al constructor de T.
         * @return Referencia al componente recién creado.
         */
        template<typename... Arg>
        T& Add(EntityID entity, Arg&&... args) {
            assert(!Contains(entity) && "La entidad ya tiene el componente");
            InsertEntity(entity);
            m_components.emplace_back(std::forward<Arg>(args)...);
            return m_components.back();
        }

        /**
         * @brief Retorna el componente de la entidad (versión mutable).
         * @param entity Entidad cuyo componente se retorna.
         * @return Referencia mutable al componente.
         */
        [[nodiscard]] T& Get(EntityID entity) noexcept {
            assert(Contains(entity) && "La entidad no tiene el componente");
            return m_components[m_sparse[GetEntityIndex(entity)]];
        }

        /**
         * @brief Retorna el componente de la entidad (versión constante).
         * @param entity Entidad cuyo componente se retorna.
         * @return Referencia constante al componente.
         */
        [[nodiscard]] const T& Get(EntityID entity) const noexcept {
            assert(Contains(entity) && "La entidad no tiene el componente");
            return m_components[m_sparse[GetEntityIndex(entity)]];
        }

        /**
         * @brief Intenta obtener el componente sin abortar si no existe.
         * @param entity Entidad objetivo.
         * @return Puntero al componente, o nullptr si la entidad no lo tiene.
         */
        [[nodiscard]] T* TryGet(EntityID entity) noexcept {
            if (!Contains(entity)) return nullptr;
            return &m_components[m_sparse[GetEntityIndex(entity)]];
        }

        /**
         * @brief Elimina el componente de la entidad usando swap & pop.
         *
         * Mueve el último componente al hueco para mantener m_components contiguo.
         *
         * @param entity Entidad cuyo componente se elimina.
         */
        void Remove(EntityID entity) override {
            if (!Contains(entity)) return;
            const EntityIndex denseIdx = m_sparse[GetEntityIndex(entity)];
            m_components[denseIdx] = std::move(m_components.back()); // swap & pop
            m_components.pop_back();
            SparseSet::Remove(entity);
        }

        /// @brief Implementa la interfaz de IComponentPool delegando a Remove.
        void RemoveEntity(EntityID entity) override { Remove(entity); }

        /// @brief Implementa GetRaw delegando a TryGet.
        void* GetRaw(EntityID entity) noexcept override { return TryGet(entity); }

        /// @brief Retorna el vector de componentes (versión mutable).
        [[nodiscard]] std::vector<T>& GetComponents() noexcept { return m_components; }

        /// @brief Retorna el vector de componentes (versión constante).
        [[nodiscard]] const std::vector<T>& GetComponents() const noexcept {
            return m_components;
        }

        /// @brief Vacía el pool eliminando todos los componentes y entidades.
        void Clear() override {
            m_components.clear();
            SparseSet::Clear();
        }

    private:
        std::vector<T> m_components; ///< Datos contiguos en memoria, mismo orden que m_dense.
    };

} // namespace ECS