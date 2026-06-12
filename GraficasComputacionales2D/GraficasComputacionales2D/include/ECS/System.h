namespace ECS {
    class Registry; // forward declaration para evitar include circular
    class System {
    public:
        virtual ~System() = default;
        virtual void OnStart(Registry& registry) {}                        // se llama una vez al registrar el sistema
        virtual void OnUpdate(Registry& registry, float deltaTime) = 0;   // se llama cada frame, obligatorio implementar
        virtual void OnDestroy(Registry& registry) {}                      // se llama al eliminar el sistema
        void SetEnable(bool enable) noexcept { m_enable = enable; }        // activa/desactiva el sistema
        [[nodiscard]] bool IsEnable() const noexcept { return m_enable; }  // retorna si el sistema está activo
    private:
        bool m_enable = true; // sistemas habilitados por defecto
    };
}