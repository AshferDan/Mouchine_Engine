using System.Runtime.CompilerServices;

namespace CSharpScript
{
    /// <summary>
    /// Debug utilities for logging from C# to C++ console
    /// </summary>
    public static class Debug
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Log(string message);
    }

    /// <summary>
    /// GameObject creation and management functions
    /// </summary>
    public static class GameObject
    {
        /// <summary>
        /// Creates a shape-based game object
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void CreateShape(
            string name,
            int type,
            float x, float y,
            float width, float height,
            float r, float g, float b, float a,
            int layer,
            bool hasPhysics,
            bool isStatic,
            bool hasGravity);

        /// <summary>
        /// Creates an image-based game object
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void CreateImage(
            string name,
            int type,
            float x, float y,
            float width, float height,
            string texturePath);

        /// <summary>
        /// Creates an animated sprite game object
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void CreateAnimatedImage(
            string name,
            float x, float y,
            float width, float height,
            string texturePath,
            int rows, int columns,
            int startFrame, int endFrame,
            float fps,
            int layer,
            bool hasPhysics,
            bool isStatic,
            bool hasGravity);
    }

    /// <summary>
    /// Transform component functions (position, rotation, scale)
    /// </summary>
    public static class Transform
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetPosition(string objectName, float x, float y);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetRotation(string objectName, float rotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AddRotation(string objectName, float rotation);

        /// <summary>
        /// Sets uniform scale (same X and Y) - internal call
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetScale_Single(string objectName, float scale);

        /// <summary>
        /// Sets non-uniform scale (different X and Y) - internal call
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetScale_XY(string objectName, float x, float y);

        /// <summary>
        /// Helper overload for uniform scale
        /// </summary>
        public static void SetScale(string objectName, float scale)
        {
            SetScale_Single(objectName, scale);
        }

        /// <summary>
        /// Helper overload for non-uniform scale
        /// </summary>
        public static void SetScale(string objectName, float x, float y)
        {
            SetScale_XY(objectName, x, y);
        }
    }

    /// <summary>
    /// Graphics/rendering functions
    /// </summary>
    public static class Graphics
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetShapeType(string objectName, int shapeType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetTexture(string objectName, string filename);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetColor(string objectName, float r, float g, float b, float a);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetAlpha(string objectName, float alpha);
    }

    /// <summary>
    /// Physics component functions
    /// </summary>
    public static class Physics
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetVelocity(string objectName, float vx, float vy);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetJumpValue(string objectName, float jumpForce);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetGravity(string objectName, bool enabled);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetGravityValue(string objectName, float x, float y);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetStatic(string objectName, bool isStatic);
    }

    /// <summary>
    /// Collision detection functions
    /// </summary>
    public static class Collision
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetCollisionShape(string objectName, int shape);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetCollisionLayer(string objectName, int layer);
    }

    /// <summary>
    /// Component management functions
    /// </summary>
    public static class Components
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AddTransformComponent(string objectName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AddRenderComponent(string objectName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AddCollisionComponent(string objectName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AddPhysicsComponent(string objectName);
    }

    // --- Enums ---
    // These must match your C++ enum values exactly

    public enum ShapeType
    {
        Square = 0,
        Circle = 1,
        Point = 2,
        Line = 3
    }

    public enum CollisionLayer
    {
        Default = 0,
        Player = 1,
        Enemy = 2,
        Ground = 3
        // Add more layers as defined in your C++ Collision.h
    }

    public enum ColliderType
    {
        Box = 0,
        Circle = 1
        // Add more if you have them in C++
    }
}