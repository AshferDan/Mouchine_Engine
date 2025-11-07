using System.Runtime.CompilerServices;

namespace CSharpScript
{
    /// <summary>
    /// Debug utilities for logging from C# to C++ console.
    /// </summary>
    public static class Debug
    {
        /// <summary>
        /// Logs a message to the C++ engine console.
        /// </summary>
        /// <param name="message">The message string to log.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Log(string message);
    }

    /// <summary>
    /// GameObject creation and management functions.
    /// </summary>
    public static class GameObject
    {
        /// <summary>
        /// Creates a shape-based game object with specified properties.
        /// </summary>
        /// <param name="name">The unique name identifier for the GameObject.</param>
        /// <param name="type">The shape type (use ShapeType enum values).</param>
        /// <param name="x">The initial X position.</param>
        /// <param name="y">The initial Y position.</param>
        /// <param name="width">The width of the shape.</param>
        /// <param name="height">The height of the shape.</param>
        /// <param name="r">Red component of the color (0.0 to 1.0).</param>
        /// <param name="g">Green component of the color (0.0 to 1.0).</param>
        /// <param name="b">Blue component of the color (0.0 to 1.0).</param>
        /// <param name="a">Alpha component of the color (0.0 to 1.0).</param>
        /// <param name="layer">The collision layer (use CollisionLayer enum values).</param>
        /// <param name="hasPhysics">Whether the object should have physics simulation.</param>
        /// <param name="isStatic">Whether the object is static (immovable).</param>
        /// <param name="hasGravity">Whether the object is affected by gravity.</param>
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
        /// Creates an image-based game object with a texture.
        /// </summary>
        /// <param name="name">The unique name identifier for the GameObject.</param>
        /// <param name="type">The shape type for the image (use ShapeType enum values).</param>
        /// <param name="x">The initial X position.</param>
        /// <param name="y">The initial Y position.</param>
        /// <param name="width">The width of the image.</param>
        /// <param name="height">The height of the image.</param>
        /// <param name="texturePath">The file path to the texture image.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void CreateImage(
            string name,
            int type,
            float x, float y,
            float width, float height,
            string texturePath);

        /// <summary>
        /// Creates an animated sprite game object with sprite sheet animation.
        /// </summary>
        /// <param name="name">The unique name identifier for the GameObject.</param>
        /// <param name="x">The initial X position.</param>
        /// <param name="y">The initial Y position.</param>
        /// <param name="width">The width of the image.</param>
        /// <param name="height">The height of the image.</param>
        /// <param name="texturePath">The file path to the sprite sheet texture.</param>
        /// <param name="rows">The number of rows in the sprite sheet.</param>
        /// <param name="columns">The number of columns in the sprite sheet.</param>
        /// <param name="startFrame">The starting frame index for the animation.</param>
        /// <param name="endFrame">The ending frame index for the animation.</param>
        /// <param name="fps">The frames per second for the animation playback.</param>
        /// <param name="layer">The collision layer (use CollisionLayer enum values).</param>
        /// <param name="hasPhysics">Whether the object should have physics simulation.</param>
        /// <param name="isStatic">Whether the object is static (immovable).</param>
        /// <param name="hasGravity">Whether the object is affected by gravity.</param>
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
    /// Transform component functions (position, rotation, scale).
    /// </summary>
    public static class Transform
    {
        /// <summary>
        /// Sets the position of a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="x">The X coordinate.</param>
        /// <param name="y">The Y coordinate.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetPosition(string objectName, float x, float y);

        /// <summary>
        /// Sets the rotation of a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="rotation">The rotation angle in radians.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetRotation(string objectName, float rotation);

        /// <summary>
        /// Adds to the current rotation of a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="rotation">The rotation angle to add (in radians).</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AddRotation(string objectName, float rotation);

        /// <summary>
        /// Sets uniform scale (same X and Y) - internal call.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="scale">The scale value for both X and Y axes.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetScale_Single(string objectName, float scale);

        /// <summary>
        /// Sets non-uniform scale (different X and Y) - internal call.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="x">The scale value for the X axis.</param>
        /// <param name="y">The scale value for the Y axis.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetScale_XY(string objectName, float x, float y);

        /// <summary>
        /// Helper overload for uniform scale (same for X and Y).
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="scale">The scale value for both X and Y axes.</param>
        public static void SetScale(string objectName, float scale)
        {
            SetScale_Single(objectName, scale);
        }

        /// <summary>
        /// Helper overload for non-uniform scale (different X and Y).
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="x">The scale value for the X axis.</param>
        /// <param name="y">The scale value for the Y axis.</param>
        public static void SetScale(string objectName, float x, float y)
        {
            SetScale_XY(objectName, x, y);
        }
    }

    /// <summary>
    /// Graphics/rendering functions.
    /// </summary>
    public static class Graphics
    {
        /// <summary>
        /// Sets the shape type for a GameObject's render component.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="shapeType">The shape type (use ShapeType enum values).</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetShapeType(string objectName, int shapeType);

        /// <summary>
        /// Sets the texture for a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="filename">The file path to the texture image.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetTexture(string objectName, string filename);

        /// <summary>
        /// Sets the color of a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="r">Red component (0.0 to 1.0).</param>
        /// <param name="g">Green component (0.0 to 1.0).</param>
        /// <param name="b">Blue component (0.0 to 1.0).</param>
        /// <param name="a">Alpha component (0.0 to 1.0).</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetColor(string objectName, float r, float g, float b, float a);

        /// <summary>
        /// Sets the alpha (transparency) value for a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="alpha">The alpha value (0.0 to 1.0, where 0.0 is fully transparent and 1.0 is fully opaque).</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetAlpha(string objectName, float alpha);
    }

    /// <summary>
    /// Physics component functions.
    /// </summary>
    public static class Physics
    {
        /// <summary>
        /// Sets the velocity of a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="vx">The X component of the velocity.</param>
        /// <param name="vy">The Y component of the velocity.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetVelocity(string objectName, float vx, float vy);

        /// <summary>
        /// Sets the jump velocity value for a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="jumpForce">The jump force/velocity value.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetJumpValue(string objectName, float jumpForce);

        /// <summary>
        /// Sets whether a GameObject is affected by gravity.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="enabled">True if the object should be affected by gravity, false otherwise.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetGravity(string objectName, bool enabled);

        /// <summary>
        /// Sets the gravity vector for a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="x">The X component of the gravity vector.</param>
        /// <param name="y">The Y component of the gravity vector.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetGravityValue(string objectName, float x, float y);

        /// <summary>
        /// Sets whether a GameObject is static (immovable).
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="isStatic">True if the object should be static, false otherwise.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetStatic(string objectName, bool isStatic);
    }

    /// <summary>
    /// Collision detection functions.
    /// </summary>
    public static class Collision
    {
        /// <summary>
        /// Sets the collision shape type for a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="shape">The collision shape type (use ColliderType enum values).</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetCollisionShape(string objectName, int shape);

        /// <summary>
        /// Sets the collision layer for a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="layer">The collision layer (use CollisionLayer enum values).</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetCollisionLayer(string objectName, int layer);
    }

    /// <summary>
    /// Component management functions.
    /// </summary>
    public static class Components
    {
        /// <summary>
        /// Adds a Transform component to a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AddTransformComponent(string objectName);

        /// <summary>
        /// Adds a Render component to a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AddRenderComponent(string objectName);

        /// <summary>
        /// Adds a Collision component to a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AddCollisionComponent(string objectName);

        /// <summary>
        /// Adds a Physics component to a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AddPhysicsComponent(string objectName);
    }

    // --- Enums ---
    // These must match your C++ enum values exactly

    /// <summary>
    /// Enumeration of shape types for rendering.
    /// Must match the C++ ShapeType enum values exactly.
    /// </summary>
    public enum ShapeType
    {
        /// <summary>Square shape.</summary>
        Square = 0,
        /// <summary>Circle shape.</summary>
        Circle = 1,
        /// <summary>Point shape.</summary>
        Point = 2,
        /// <summary>Line shape.</summary>
        Line = 3
    }

    /// <summary>
    /// Enumeration of collision layers for collision detection.
    /// Must match the C++ CollisionLayer enum values exactly.
    /// </summary>
    public enum CollisionLayer
    {
        /// <summary>Default collision layer.</summary>
        Default = 0,
        /// <summary>Player collision layer.</summary>
        Player = 1,
        /// <summary>Enemy collision layer.</summary>
        Enemy = 2,
        /// <summary>Ground collision layer.</summary>
        Ground = 3
        // Add more layers as defined in your C++ Collision.h
    }

    /// <summary>
    /// Enumeration of collider types for collision shapes.
    /// Must match the C++ ColliderType enum values exactly.
    /// </summary>
    public enum ColliderType
    {
        /// <summary>Box collider type.</summary>
        Box = 0,
        /// <summary>Circle collider type.</summary>
        Circle = 1
        // Add more if you have them in C++
    }
}