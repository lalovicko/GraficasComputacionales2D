#pragma once

#include "Prerequisitesr.h"

namespace ECS {

	using EntityIndex = uint32_t;   ///< Posición del entity en el array denso.
	using EntityVersion = uint32_t;   ///< Versión para invalidar IDs viejos al destruir.
	using EntityID = uint64_t;   ///< ID compuesto: [version(32 bits altos) | index(32 bits bajos)].
	using ComponentTypeID = uint32_t; ///< ID único generado por tipo de componente en tiempo de compilación.

	/// @brief Valor centinela que representa una entidad inválida o destruida.
	inline constexpr EntityID NULL_ENTITY = std::numeric_limits<EntityID>::max();

	/**
	 * @brief Extrae el índice de una entidad desde su ID.
	 * @param id EntityID del que se extraerá el índice.
	 * @return Los 32 bits bajos del ID (índice en el array denso).
	 */
	[[nodiscard]] inline EntityIndex GetEntityIndex(EntityID id) noexcept {
		return static_cast<EntityIndex>(id & 0xFFF'FFFFFull);
	}

	/**
	 * @brief Extrae la versión de una entidad desde su ID.
	 * @param id EntityID del que se extraerá la versión.
	 * @return Los 32 bits altos del ID (versión de la entidad).
	 */
	[[nodiscard]] inline EntityVersion GetEntityVersion(EntityID id) noexcept {
		return static_cast<EntityVersion>((id >> 32) & 0xFFF'FFFFFull);
	}

	/**
	 * @brief Construye un EntityID empaquetando índice y versión.
	 * @param index   Posición de la entidad en el array denso.
	 * @param version Versión actual de la entidad.
	 * @return EntityID resultante del empaquetado.
	 */
	[[nodiscard]] inline EntityID MakeEntityID(
		EntityIndex index, EntityVersion version) noexcept {
		return (static_cast<EntityID>(version) << 32) | static_cast<EntityID>(index);
	}

	/**
	 * @brief Genera un ComponentTypeID único e incremental.
	 * @return Siguiente ID disponible (contador global estático).
	 */
	[[nodiscard]] inline ComponentTypeID NextComponentTypeID() noexcept {
		static ComponentTypeID counter = 0;
		return counter++;
	}

	/**
	 * @brief Retorna el ComponentTypeID asociado al tipo T.
	 *
	 * El ID es estático por tipo: T siempre recibe el mismo ID
	 * durante toda la ejecución del programa.
	 *
	 * @tparam T Tipo del componente.
	 * @return ID único del tipo T.
	 */
	template<typename T>
	[[nodiscard]] ComponentTypeID GetComponentTypeID() noexcept {
		static const ComponentTypeID ID = NextComponentTypeID();
		return ID;
	}

} // namespace ECS