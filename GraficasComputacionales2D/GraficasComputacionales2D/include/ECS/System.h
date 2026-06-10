namespace ECS {

    class Registry; 
    class System {
    public:
        virtual ~System() = default;

        virtual void OnStart(Registry& /*registry*/) {}

        virtual void OnUpdate(Registry& registry, float deltaTime) = 0;

        virtual void OnDestroy(Registry& /*registry*/) {}

        void SetEnable(bool enable) noexcept { m_enable = enable; }

        [[nodiscard]] bool IsEnable() const noexcept { return m_enable; }

    private:
        /// Estado del sistema (por defecto habilitado).
        bool m_enable = true;
    };

} 