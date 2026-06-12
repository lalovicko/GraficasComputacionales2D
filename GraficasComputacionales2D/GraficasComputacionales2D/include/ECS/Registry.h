#pragma once

#include "ComponentPool.h"
#include "View.h"
#include "Types.h"
#include "System.h"

namespace ECS {

    /**
     * @brief Núcleo del ECS. Gestiona entidades, componentes y sistemas.
     *
     * Mantiene un pool por tipo de componente y una lista de entidades
     * con versiones para invalidar IDs de entidades destruidas.
     * Los índices liberados se reutilizan mediante m_freeList.
     */
    class Registry {
    public:
        /**
         * @brief Crea una entidad nueva y retorna su ID.
         *
         * Reutiliza un índice de m_freeList si está disponible.
         * De lo contrario, expande m_entities y m_versions.
         *
         * @return EntityID único y válido para la nueva entidad.
         */
        EntityID CreateEntity() {
            EntityIndex idx;
            if (!m_freeList.empty()) {       // reutiliza índice de entidad destruida
                idx = m_freeList.front();
                m_freeList.pop();
            }
            else {                           // primera vez que se usa este índice
                idx = static_cast<EntityIndex>(m_versions.size());
                m_versions.push_back(0);
                m_entities.push_back(NULL_ENTITY);
            }
            EntityID id = MakeEntityID(idx, m_versions[idx]);
            m_entities[idx] = id;
            return id;
        }

        /**
         * @brief Destruye una entidad e invalida todos sus IDs previos.
         *
         * Elimina todos sus componentes, incrementa su versión
         * (lo que invalida cualquier EntityID guardado con el índice anterior)
         * y libera el índice para reutilización.
         *
         * @param entity EntityID de la entidad a destruir.
         */
        void DestroyEntity(EntityID entity) {
            assert(IsAlive(entity) && "DestroyEntity: entidad inválida o ya destruida");
            for (auto& [typeID, pool] : m_componentPools)
                pool->RemoveEntity(entity);          // limpia todos los componentes
            const EntityIndex idx = GetEntityIndex(entity);
            ++m_versions[idx];                     // invalida IDs viejos con este índice
            m_entities[idx] = NULL_ENTITY;
            m_freeList.push(idx);                  // índice disponible para reutilizar
        }

        /**
         * @brief Verifica si una entidad sigue viva y es válida.
         *
         * Compara el EntityID completo (incluye versión) para detectar
         * entidades destruidas y reutilizadas en el mismo índice.
         *
         * @param entity EntityID a verificar.
         * @return true si la entidad existe y no ha sido destruida.
         */
        [[nodiscard]] bool IsAlive(EntityID entity) const noexcept {
            const EntityIndex idx = GetEntityIndex(entity);
            return idx < m_versions.size() && m_entities[idx] == entity;
        }

        /**
         * @brief Retorna la cantidad de entidades actualmente vivas.
         * @return Total de entidades menos los índices libres.
         */
        [[nodiscard]] std::size_t EntityCount() const noexcept {
            return m_entities.size() - m_freeList.size();
        }

        /// @brief Retorna el vector de todos los EntityIDs (incluyendo NULL_ENTITY para slots libres).
        [[nodiscard]] const std::vector<EntityID>& GetEntities() const noexcept {
            return m_entities;
        }

        /**
         * @brief Agrega un componente de tipo T a la entidad.
         *
         * Crea el pool de T si no existe. Los argumentos se reenvían
         * al constructor de T mediante perfect forwarding.
         *
         * @tparam T    Tipo del componente a agregar.
         * @tparam Args Tipos de los argumentos del constructor de T.
         * @param entity Entidad destino.
         * @param args   Argumentos para construir el componente.
         * @return Referencia al componente recién creado.
         */
        template<typename T, typename... Args>
        T& AddComponent(EntityID entity, Args&&... args) {
            assert(IsAlive(entity) && "AddComponent: entidad inválida o destruida");
            return GetOrCreatePool<T>()->Add(entity, std::forward<Args>(args)...);
        }

        /**
         * @brief Elimina el componente de tipo T de la entidad.
         *
         * No hace nada si la entidad no tiene ese componente o el pool no existe.
         *
         * @tparam T Tipo del componente a eliminar.
         * @param entity Entidad objetivo.
         */
        template<typename T>
        void RemoveComponent(EntityID entity) {
            if (auto pool = GetPool<T>()) pool->Remove(entity);
        }

        /**
         * @brief Asigna un componente a la entidad, sobrescribiendo si ya existe.
         *
         * Si la entidad ya tiene el componente, lo sobreescribe con move.
         * Si no lo tiene, llama a Add para crearlo.
         *
         * @tparam T Tipo del componente.
         * @param entity Entidad objetivo.
         * @param value  Nuevo valor del componente.
         * @return Referencia al componente actualizado o creado.
         */
        template<typename T>
        T& SetComponent(EntityID entity, T value) {
            assert(IsAlive(entity) && "SetComponent: entidad inválida");
            auto* pool = GetOrCreatePool<T>();
            if (pool->Contains(entity)) {
                pool->Get(entity) = std::move(value); // sobreescribe si ya existe
                return pool->Get(entity);
            }
            return pool->Add(entity, std::move(value));
        }

        /**
         * @brief Verifica si la entidad tiene el componente de tipo T.
         * @tparam T Tipo del componente.
         * @param entity Entidad a consultar.
         * @return true si el pool existe y contiene a la entidad.
         */
        template<typename T>
        [[nodiscard]] bool HasComponent(EntityID entity) const noexcept {
            const auto* pool = GetPool<T>();
            return pool && pool->Contains(entity);
        }

        /**
         * @brief Retorna el componente de tipo T de la entidad (versión mutable).
         * @tparam T Tipo del componente.
         * @param entity Entidad objetivo.
         * @return Referencia mutable al componente.
         */
        template<typename T>
        [[nodiscard]] T& GetComponent(EntityID entity) {
            assert(IsAlive(entity));
            auto* pool = GetPool<T>();
            assert(pool && "GetComponent: pool no existe para este tipo");
            return pool->Get(entity);
        }

        /**
         * @brief Retorna el componente de tipo T de la entidad (versión constante).
         * @tparam T Tipo del componente.
         * @param entity Entidad objetivo.
         * @return Referencia constante al componente.
         */
        template<typename T>
        [[nodiscard]] const T& GetComponent(EntityID entity) const {
            assert(IsAlive(entity));
            const auto* pool = GetPool<T>();
            assert(pool && "GetComponent: pool no existe para este tipo");
            return pool->Get(entity);
        }

        /**
         * @brief Intenta obtener el componente de tipo T sin abortar si no existe.
         * @tparam T Tipo del componente.
         * @param entity Entidad objetivo.
         * @return Puntero al componente, o nullptr si no existe.
         */
        template<typename T>
        [[nodiscard]] T* TryGetComponent(EntityID entity) noexcept {
            if (!IsAlive(entity)) return nullptr;
            auto* pool = GetPool<T>();
            return pool ? pool->TryGet(entity) : nullptr;
        }

        /**
         * @brief Retorna una vista que itera entidades con todos los componentes dados.
         *
         * Crea los pools si no existen. La vista es ligera (solo punteros).
         *
         * @tparam Components Tipos de componentes requeridos.
         * @return Vista lista para iterar con Each() o EachEntity().
         */
        template<typename... Components>
        [[nodiscard]] View<Components...> GetView() {
            return View<Components...>(GetOrCreatePool<Components>()...);
        }

        // -------------------------------------------------------------------------
        // Sistemas
        // -------------------------------------------------------------------------

        /**
         * @brief Registra y activa un sistema de tipo T en el Registry.
         *
         * Construye el sistema con los args dados, llama a OnStart y lo guarda.
         *
         * @tparam T    Tipo del sistema (debe derivar de ECS::System).
         * @tparam Args Tipos de los argumentos del constructor de T.
         * @param args  Argumentos para construir el sistema.
         * @return Referencia al sistema registrado.
         */
        template<typename T, typename... Args>
        T& AddSystem(Args&&... args) {
            static_assert(std::is_base_of_v<System, T>, "T debe derivar de ECS::System");
            auto system = std::make_unique<T>(std::forward<Args>(args)...);
            T& ref = *system;
            system->OnStart(*this);              // inicializa antes de guardar
            m_systems.push_back(std::move(system));
            return ref;
        }

        /**
         * @brief Llama a OnUpdate en todos los sistemas habilitados.
         * @param deltaTime Tiempo transcurrido desde el último frame.
         */
        void UpdateSystems(float deltaTime) {
            for (auto& system : m_systems)
                if (system->IsEnable())
                    system->OnUpdate(*this, deltaTime);
        }

        /// @brief Notifica OnDestroy a todos los sistemas y los elimina.
        void RemoveAllSystems() {
            for (auto& system : m_systems)
                system->OnDestroy(*this);
            m_systems.clear();
        }

        /**
         * @brief Elimina todos los sistemas, componentes y entidades del Registry.
         *
         * Deja el Registry en el mismo estado que recién construido.
         */
        void Clear() {
            RemoveAllSystems();
            for (auto& [typeID, pool] : m_componentPools)
                pool->Clear();
            m_entities.clear();
            m_versions.clear();
            while (!m_freeList.empty()) m_freeList.pop();
        }

        /**
         * @brief Retorna el mapa de todos los pools de componentes registrados.
         * @return Referencia constante al mapa de pools.
         */
        [[nodiscard]] const std::unordered_map
            ComponentTypeID, std::unique_ptr<IComponentPool >> &
            GetPools() const noexcept {
            return m_componentPools;
        }

    private:
        /**
         * @brief Retorna el pool de tipo T, creándolo si no existe.
         * @tparam T Tipo del componente.
         * @return Puntero al ComponentPool<T>.
         */
        template<typename T>
        ComponentPool<T>* GetOrCreatePool() {
            const ComponentTypeID typeID = GetComponentTypeID<T>();
            auto it = m_componentPools.find(typeID);
            if (it == m_componentPools.end()) {
                auto [newIt, ok] = m_componentPools.emplace(
                    typeID, std::make_unique<ComponentPool<T>>());
                return static_cast<ComponentPool<T>*>(newIt->second.get());
            }
            return static_cast<ComponentPool<T>*>(it->second.get());
        }

        /**
         * @brief Retorna el pool de tipo T si existe, o nullptr si no.
         * @tparam T Tipo del componente.
         * @return Puntero al pool o nullptr.
         */
        template<typename T>
        ComponentPool<T>* GetPool() noexcept {
            const ComponentTypeID typeID = GetComponentTypeID<T>();
            auto it = m_componentPools.find(typeID);
            return (it != m_componentPools.end())
                ? static_cast<ComponentPool<T>*>(it->second.get())
                : nullptr;
        }

        /**
         * @brief Versión constante de GetPool.
         * @tparam T Tipo del componente.
         * @return Puntero constante al pool o nullptr.
         */
        template<typename T>
        const ComponentPool<T>* GetPoolConst() const noexcept {
            const ComponentTypeID typeID = GetComponentTypeID<T>();
            auto it = m_componentPools.find(typeID);
            return (it != m_componentPools.end())
                ? static_cast<const ComponentPool<T>*>(it->second.get())
                : nullptr;
        }

    private:
        std::vector<EntityID>      m_entities;      ///< ID actual por índice (NULL_ENTITY si está muerta).
        std::vector<EntityVersion> m_versions;      ///< Versión por índice; se incrementa al destruir.
        std::queue<EntityIndex>    m_freeList;      ///< Índices disponibles para reutilizar.

        /// @brief Un pool por tipo de componente, indexado por ComponentTypeID.
        std::unordered_map<ComponentTypeID, std::unique_ptr<IComponentPool>> m_componentPools;

        std::vector<std::unique_ptr<System>> m_systems; ///< Sistemas en orden de registro.
    };

} // namespace ECS