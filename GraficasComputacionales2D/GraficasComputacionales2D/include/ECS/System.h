#pragma once

namespace ECS {

class Registry;

class System {
public:
    virtual ~System() = default;

    virtual void OnStart(Registry&) {}
    virtual void OnUpdate(Registry& registry, float deltaTime) = 0;
    virtual void OnDestroy(Registry&) {}

    void SetEnabled(bool enabled) noexcept { m_enabled = enabled; }
    [[nodiscard]] bool IsEnabled() const noexcept { return m_enabled; }

    // Alias para no romper codigo anterior.
    void SetEnable(bool enabled) noexcept { SetEnabled(enabled); }
    [[nodiscard]] bool IsEnable() const noexcept { return IsEnabled(); }

private:
    bool m_enabled{ true };
};

} // namespace ECS
