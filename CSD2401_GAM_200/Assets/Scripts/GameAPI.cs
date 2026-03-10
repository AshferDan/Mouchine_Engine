/* Start Header ************************************************************************/
/*!
\file       GameAPI.cs
\author     Jonathan Fam Qian Wei [100% - 372 lines]
\par        Fam.q@digipen.edu
\date       Novenber, 7, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;

namespace CSharpScript
{
    // --- Structs ---
    [StructLayout(LayoutKind.Sequential)]
    public struct vec2
    {
        public float x;
        public float y;

        public vec2(float x, float y)
        {
            this.x = x;
            this.y = y;
        }

        public override string ToString()
        {
            return $"({x}, {y})";
        }
    }
    [StructLayout(LayoutKind.Sequential)]
    public struct vec4
    {
        public float x, y, z, w;

        public vec4(float x, float y, float z, float w)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }

        public override string ToString()
        {
            return $"({x}, {y}, {z}, {w})";
        }
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
        Player = 0,
        Ground = 1,
        Enemy = 2,
        Platform = 3,
        Pickup = 4,
        Default = 5
        // Values must match the C++ enum in ColliderComponent.h
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
            int startRow, int startCol, int endRow, int endCol,
            float fps,
            int layer,
            bool hasPhysics,
            bool isStatic,
            bool hasGravity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern string FindGameObjectByName(string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern string FindGameObjectByID(int id);

        /// <summary>
        /// Destroys the named GameObject, triggering OnDestroy on its scripts.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Destroy(string name);

        /// <summary>
        /// Destroys a GameObject by its unique EntityID. Prefer over DestroyByName when IDs are available.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void DestroyByID(int id);

        /// <summary>
        /// Destroys the first GameObject found with the given name. Equivalent to Destroy().
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void DestroyByName(string name);
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

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetPosition_Internal(string objectName, out float x, out float y);

        public static vec2 GetPosition(string objectName)
        {
            GetPosition_Internal(objectName, out float x, out float y);
            return new vec2(x, y);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float GetRotation(string objectName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetScale_Internal(string objectName, out float x, out float y);

        public static vec2 GetScale(string objectName)
        {
            GetScale_Internal(objectName, out float x, out float y);
            return new vec2(x, y);
        }

        // world to screen nonsense, based off unity's TransformPoint
        public static vec2 TransformPoint(string objectName, vec2 localPoint)
        {
            vec2 pos = Transform.GetPosition(objectName);
            vec2 scale = Transform.GetScale(objectName);
            float rot = Transform.GetRotation(objectName);

            // local pos
            float localx = localPoint.x * scale.x;
            float localy = localPoint.y * scale.y;

            // world pos (takes int account rotation MATRIX NIGHTMARENIGHTMARENIGHTMARE
            float worldx = pos.x + (localx * (float)Math.Sin(rot) - localy * (float)Math.Sin(rot));
            float worldy = pos.y + (localx * (float)Math.Sin(rot) + localy * (float)Math.Cos(rot));

            return new vec2(worldx, worldy);
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

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern vec4 GetColor(string objectName);

        /// <summary>
        /// Sets the render layer category for a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="layer">The layer (0=Background, 1=Walls, 2=Player, 3=Enemies, 4=Foreground, 5=UI).</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetRenderLayer(string objectName, int layer);

        /// <summary>
        /// Gets the render layer category for a GameObject.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int GetRenderLayer(string objectName);

        /// <summary>
        /// Sets visibility for an entire layer category. All objects in this layer will be shown/hidden.
        /// </summary>
        /// <param name="layer">The layer (0=Background, 1=Walls, 2=Player, 3=Enemies, 4=Foreground, 5=UI).</param>
        /// <param name="visible">True to show, false to hide.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetLayerVisible(int layer, bool visible);

        /// <summary>
        /// Checks if a layer category is visible.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool IsLayerVisible(int layer);

        // Layer constants for convenience
        public const int LayerBackground = 0;
        public const int LayerWalls = 1;
        public const int LayerPlayer = 2;
        public const int LayerEnemies = 3;
        public const int LayerForeground = 4;
        public const int LayerUI = 5;
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

        /// <summary>
        /// Sets the mass of a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="mass">The mass value to set.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetMass(string objectName, float mass);

        /// <summary>
        /// Sets the friction coefficient of a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="friction">The friction coefficient value to set.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetFriction(string objectName, float friction);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetVelocity_Internal(string objectName, out float x, out float y);

        public static vec2 GetVelocity(string objectName)
        {
            GetVelocity_Internal(objectName, out float x, out float y);
            return new vec2(x, y);
        }

        public static vec2 gravity => new vec2(0.0f, -9.81f);
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

        /// <summary>
        /// Casts a ray into the scene and returns information about what it hits.
        /// </summary>
        /// <param name="origin">The starting point of the ray in world coordinates.</param>
        /// <param name="direction">The direction of the ray (should be a normalized vector).</param>
        /// <param name="distance">The maximum distance the ray should travel.</param>
        /// <param name="hitObjectName">Output: The name of the game object that was hit.</param>
        /// <param name="hitDistance">Output: The distance from the origin to the hit point.</param>
        /// <returns>True if the ray hit something, false otherwise.</returns>
        public static bool Raycast(string selfName, vec2 origin, vec2 direction, float distance, out string hitObjectName, out float hitDistance)
        {
            return Raycast_Internal(selfName, origin.x, origin.y, direction.x, direction.y, distance, out hitObjectName, out hitDistance);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Raycast_Internal(string selfName, float x, float y, float dx, float dy, float distance, out string hitObjectName, out float hitDistance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool OnTriggerEnter(string obj1, string obj2);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool OnTriggerExit(string obj1, string obj2);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool OnTriggerStay(string obj1, string obj2);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int GetCollisionLayer(string objectName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int GetCollisionShape(string objectName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool CheckColliderLayer(int layer1, int layer2);

        /// <summary>
        /// Returns true if the named object's collider is currently overlapping
        /// any object on the specified collision layer.
        /// Backed by CollisionSystem::IsCollidingWithLayer (spatial-grid narrow phase).
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool IsCollidingWithLayer(string objectName, int targetLayer);

        // --- Typed helpers (no extra internal calls needed) ---

        /// <summary>
        /// Sets the collision layer for a GameObject using the CollisionLayer enum.
        /// </summary>
        public static void SetLayer(string objectName, CollisionLayer layer)
            => SetCollisionLayer(objectName, (int)layer);

        /// <summary>
        /// Gets the collision layer for a GameObject as a CollisionLayer enum value.
        /// Returns CollisionLayer.Default if the object has no collider.
        /// </summary>
        public static CollisionLayer GetLayer(string objectName)
        {
            int raw = GetCollisionLayer(objectName);
            return raw < 0 ? CollisionLayer.Default : (CollisionLayer)raw;
        }

        /// <summary>
        /// Returns true if both layers are configured to collide with each other.
        /// </summary>
        public static bool DoLayersCollide(CollisionLayer a, CollisionLayer b)
            => CheckColliderLayer((int)a, (int)b);

        /// <summary>
        /// Returns true if the named object's collider is on the specified layer.
        /// </summary>
        public static bool IsOnLayer(string objectName, CollisionLayer layer)
            => GetLayer(objectName) == layer;
    }

    /// <summary>
    /// Component management functions.
    /// </summary>
    public static class Scripting
    {
        /// <summary>
        /// Attaches a script to a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="scriptClassName">The name of the script class to attach.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool AttachScript(string objectName, string scriptClassName);

        /// <summary>
        /// Returns the name of the first object on targetLayer currently colliding with objectName, or "" if none.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern string GetCollidingObjectName(string objectName, int layer);

        /// <summary>
        /// Returns the unique EntityID of the first object on targetLayer colliding with objectName, or 0 if none.
        /// Prefer this over GetCollidingObjectName when objects may share names.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int GetCollidingObjectID(string objectName, int layer);

        // ---- Field getters ----
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float  GetFloatField (string objectName, string scriptClass, string fieldName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int    GetIntField   (string objectName, string scriptClass, string fieldName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool   GetBoolField  (string objectName, string scriptClass, string fieldName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern string GetStringField(string objectName, string scriptClass, string fieldName);

        // ---- Field setters ----
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetFloatField (string objectName, string scriptClass, string fieldName, float  value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetIntField   (string objectName, string scriptClass, string fieldName, int    value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetBoolField  (string objectName, string scriptClass, string fieldName, bool   value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetStringField(string objectName, string scriptClass, string fieldName, string value);
    }

    public static class Components
    {
        /// <summary>
        /// Sets the active state of a component on a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="componentName">The name of the component.</param>
        /// <param name="active">The new active state.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetComponentActive(string objectName, string componentName, bool active);

        /// <summary>
        /// Removes a component from a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="componentName">The name of the component to remove.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void RemoveComponent(string objectName, string componentName);

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

        /// <summary>
        /// Adds a Text component to a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="fontPath">The file path to the font.</param>
        /// <param name="fontSize">The size of the font.</param>
        /// <param name="text">The initial text content.</param>
        /// <param name="r">Red component (0.0 to 1.0).</param>
        /// <param name="g">Green component (0.0 to 1.0).</param>
        /// <param name="b">Blue component (0.0 to 1.0).</param>
        /// <param name="a">Alpha component (0.0 to 1.0).</param>
        /// <param name="screenSpace">Whether to render in screen space.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AddTextComponent(string objectName, string fontPath, uint fontSize, string text, float r, float g, float b, float a, bool screenSpace);
    }

    /// <summary>
    /// Text component functions for managing text display.
    /// </summary>
    public static class Text
    {
        /// <summary>
        /// Sets the text content of a GameObject's TextComponent.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="text">The text content to set.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetText(string objectName, string text);

        /// <summary>
        /// Gets the text content of a GameObject's TextComponent.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <returns>The text content.</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern string GetText(string objectName);

        /// <summary>
        /// Sets the color of the text.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="r">Red component (0.0 to 1.0).</param>
        /// <param name="g">Green component (0.0 to 1.0).</param>
        /// <param name="b">Blue component (0.0 to 1.0).</param>
        /// <param name="a">Alpha component (0.0 to 1.0).</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetTextColor(string objectName, float r, float g, float b, float a);

        /// <summary>
        /// Gets the color of the text.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <returns>The text color as vec4.</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern vec4 GetTextColor(string objectName);

        /// <summary>
        /// Sets the scale of the text.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="scale">The scale factor.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetTextScale(string objectName, float scale);

        /// <summary>
        /// Gets the scale of the text.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <returns>The scale factor.</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float GetTextScale(string objectName);

        /// <summary>
        /// Sets whether the text is rendered in screen space.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="screenSpace">True for screen space, false for world space.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetTextScreenSpace(string objectName, bool screenSpace);

        /// <summary>
        /// Gets whether the text is rendered in screen space.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <returns>True if in screen space, false if in world space.</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool GetTextScreenSpace(string objectName);
    }


    /// <summary>
    /// Audio system functions.
    /// </summary>
    public static class Audio
    {
        /// <summary>
        /// Adds and loads a new sound into the audio system.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AddSound(string name, string filepath, bool loop, float volume, float pitch, string category);

        /// <summary>
        /// Plays a sound on the specified channel.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void PlaySound(string soundName, string channel = "");

        /// <summary>
        /// Stops a sound on the specified channel.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void StopSound(string channelName);

        /// <summary>
        /// Stops all currently playing sounds.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void StopAllSounds();

        /// <summary>
        /// Removes a sound from the audio system.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void RemoveSound(string soundName);

        /// <summary>
        /// Triggers an audio event by name.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void TriggerEvent(string eventName);

        /// <summary>
        /// Maps an event name to a sound name.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AddEventSound(string eventName, string soundName);

        /// <summary>
        /// Sets the volume of a specific sound.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetVolume(string soundName, float volume);

        /// <summary>
        /// Sets the volume for an entire category (music, sfx, voice, ambient).
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetCategoryVolume(string categoryName, float volume);

        /// <summary>
        /// Sets the pitch of a specific sound.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetPitch(string soundName, float pitch);

        /// <summary>
        /// Pauses or resumes a sound on a channel.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void PauseSound(string channelName, bool pause);

        /// <summary>
        /// Pauses or resumes all sounds.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void PauseAllSounds(bool pause);

        /// <summary>
        /// Toggles master mute on/off.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void ToggleMasterMute();

        /// <summary>
        /// Toggles mute for a specific category.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void ToggleCategoryMute(string category);

        /// <summary>
        /// Returns true if master audio is muted.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool IsMasterMuted();

        /// <summary>
        /// Returns true if a category is muted.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool IsCategoryMuted(string category);

        /// <summary>
        /// Fades out a category over the specified duration.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void FadeOutCategory(string category, float duration);

        /// <summary>
        /// Fades in a category over the specified duration to target volume.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void FadeInCategory(string category, float duration, float targetVolume = 1.0f);

        /// <summary>
        /// Fades out all audio categories over the specified duration.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void FadeOutAll(float duration);

        /// <summary>
        /// Sets the 3D position of the audio listener (usually the camera/player).
        /// Sounds will be positioned relative to this listener position.
        /// </summary>
        /// <param name="x">X position in world space.</param>
        /// <param name="y">Y position in world space.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Set3DListenerPosition(float x, float y);

        /// <summary>
        /// Sets the 3D position of a sound source for spatial audio.
        /// The sound will have distance-based attenuation and stereo panning.
        /// </summary>
        /// <param name="channelName">Name of the channel playing the sound.</param>
        /// <param name="x">X position of the sound source in world space.</param>
        /// <param name="y">Y position of the sound source in world space.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Set3DSoundPosition(string channelName, float x, float y);

        /// <summary>
        /// Loads a 3D positional sound with distance attenuation and panning.
        /// Sounds get quieter as the listener moves away and pan left/right based on position.
        /// </summary>
        /// <param name="name">Identifier for the sound.</param>
        /// <param name="filepath">Path to the audio file.</param>
        /// <param name="loop">Whether the sound should loop continuously.</param>
        /// <param name="minDistance">Minimum distance before sound starts attenuating (default: 100.0).</param>
        /// <param name="maxDistance">Maximum distance where sound is silent (default: 1000.0).</param>
        /// <param name="category">Audio category (default: "sfx").</param>
        /// <param name="volume">Initial volume 0.0-1.0 (default: 1.0).</param>
        /// <param name="pitch">Pitch multiplier (default: 1.0).</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Load3DSound(string name, string filepath, bool loop = false,
                                              float minDistance = 100.0f, float maxDistance = 1000.0f,
                                              string category = "sfx", float volume = 1.0f, float pitch = 1.0f);
    }

    /// <summary>
    /// Camera management functions.
    /// </summary>
    public static class Camera
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetPosition(float x, float y);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetZoom(float zoom);

        /// <summary>
        /// Sets camera mode. Ensure enum matches C++ CameraComponent::CameraMode.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetMode(int mode);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetPosition_Internal(out float x, out float y);

        public static vec2 GetPosition()
        {
            GetPosition_Internal(out float x, out float y);
            return new vec2(x, y);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float GetZoom();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void GetMode();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void CameraFlash(float duration, float r, float g, float b, float a, bool fadeout, float camShakeStrength);
    }

    public static class Animation
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void PlayAnimation(string objectName, string animationName, bool isLooping = true);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetLooping(string objectName, bool isLooping);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AddToQueue(string objectName, string animationName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void RegisterAnimation(string objectName, string animationName, string texturePath, int rows, int cols, int startRow, int startCol, int endRow, int endCol, float fps);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetAnimation(string objectName, int startFrame, int endFrame, float fps);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern string GetCurrentAnimationName(string objectName);
    }

    /// <summary>
    /// Utility functions.
    /// </summary>
    public static class Utils
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int RandomRange(int min, int max);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float GetDelta_Time();
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float GetFixed_Delta_Time();
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float GetFramesPerSecond();
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float MathfLerp(float a, float b, float t);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float MathfInverseLerp(float a, float b, float value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float MathfClamp(float value, float min, float max);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int MathiClamp(int value, int min, int max);

        public static float MathfMoveTowards(float current, float target, float maxDelta)
        {
            if (Math.Abs(target - current) <= maxDelta)
            {
                return target;
            }
            return current + Math.Sign(target - current) * maxDelta;
        }
    }

    /// <summary>
    /// Win/Lose system functions for triggering game state changes.
    /// </summary>
    public static class WinLoseSystem
    {
        /// <summary>
        /// Triggers the lose sequence, displaying the lose screen and pausing the game.
        /// This can be called from C# scripts to manually trigger a game over state.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void TriggerLose();

        /// <summary>
        /// Checks if the win screen is currently active and displayed.
        /// </summary>
        /// <returns>True if the win screen is currently shown, false otherwise.</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool IsWinScreenActive();

        /// <summary>
        /// Checks if the lose screen is currently active and displayed.
        /// </summary>
        /// <returns>True if the lose screen is currently shown, false otherwise.</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool IsLoseScreenActive();
    }

    /// <summary>
    /// Particle system functions for managing particle emitters.
    /// </summary>
    public static class Particles
    {
        #region Component Management
        /// <summary>
        /// Adds a ParticleEmitterComponent to a GameObject.
        /// </summary>
        /// <param name="objectName">The name of the GameObject.</param>
        /// <param name="maxParticles">The maximum number of particles this emitter can have.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AddParticleEmitterComponent(string objectName, int maxParticles = 100);
        #endregion

        #region Emission Control
        /// <summary>
        /// Starts emitting particles.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void StartEmission(string objectName);

        /// <summary>
        /// Stops emitting particles.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void StopEmission(string objectName);

        /// <summary>
        /// Emits a burst of particles immediately.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        /// <param name="count">The number of particles to emit.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Burst(string objectName, int count);

        /// <summary>
        /// Clears all active particles.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Clear(string objectName);

        /// <summary>
        /// Checks if the emitter is currently emitting.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        /// <returns>True if emitting, false otherwise.</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool IsEmitting(string objectName);

        /// <summary>
        /// Gets the number of currently active particles.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        /// <returns>The number of active particles.</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int GetActiveParticleCount(string objectName);

        /// <summary>
        /// Gets the maximum number of particles this emitter can have.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        /// <returns>The maximum particle count.</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int GetMaxParticles(string objectName);
        #endregion

        #region Emission Configuration
        /// <summary>
        /// Sets the emission rate (particles per second).
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        /// <param name="rate">Particles emitted per second.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetEmissionRate(string objectName, float rate);

        /// <summary>
        /// Sets the maximum number of particles.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        /// <param name="max">Maximum particle count.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetMaxParticles(string objectName, int max);

        /// <summary>
        /// Sets the duration of emission (how long the emitter will emit).
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        /// <param name="seconds">Duration in seconds.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetDuration(string objectName, float seconds);

        /// <summary>
        /// Sets whether the emitter should loop.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        /// <param name="shouldLoop">True to loop, false otherwise.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetLoop(string objectName, bool shouldLoop);

        /// <summary>
        /// Sets the lifetime of individual particles.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        /// <param name="seconds">Lifetime in seconds.</param>
        /// <param name="variation">Random variation (+/-).</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetLifetime(string objectName, float seconds, float variation = 0f);
        #endregion

        #region Spawn Configuration
        /// <summary>
        /// Sets the spawn offset relative to the emitter position.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        /// <param name="x">X offset.</param>
        /// <param name="y">Y offset.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetSpawnOffset(string objectName, float x, float y);

        /// <summary>
        /// Sets the spawn radius for circular spawn area.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        /// <param name="radius">Spawn radius.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetSpawnRadius(string objectName, float radius);

        /// <summary>
        /// Sets the spawn box for rectangular spawn area.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        /// <param name="halfWidth">Half width of the spawn box.</param>
        /// <param name="halfHeight">Half height of the spawn box.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetSpawnBox(string objectName, float halfWidth, float halfHeight);
        #endregion

        #region Physics Configuration
        /// <summary>
        /// Sets the initial velocity of particles.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        /// <param name="dirX">X direction component.</param>
        /// <param name="dirY">Y direction component.</param>
        /// <param name="magnitude">Speed magnitude.</param>
        /// <param name="variation">Random variation (+/-).</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetVelocity(string objectName, float dirX, float dirY, float magnitude, float variation = 0f);

        /// <summary>
        /// Sets the angle spread for velocity variation.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        /// <param name="degrees">Spread angle in degrees.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetAngleSpread(string objectName, float degrees);

        /// <summary>
        /// Sets the gravity affecting particles.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        /// <param name="gravX">X gravity component.</param>
        /// <param name="gravY">Y gravity component.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetGravity(string objectName, float gravX, float gravY);

        /// <summary>
        /// Sets the drag coefficient for particles.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        /// <param name="drag">Drag coefficient.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetDrag(string objectName, float drag);
        #endregion

        #region Appearance Configuration
        /// <summary>
        /// Sets the starting color of particles.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        /// <param name="r">Red component (0.0 to 1.0).</param>
        /// <param name="g">Green component (0.0 to 1.0).</param>
        /// <param name="b">Blue component (0.0 to 1.0).</param>
        /// <param name="a">Alpha component (0.0 to 1.0).</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetStartColor(string objectName, float r, float g, float b, float a);

        /// <summary>
        /// Sets the ending color of particles (interpolated over lifetime).
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        /// <param name="r">Red component (0.0 to 1.0).</param>
        /// <param name="g">Green component (0.0 to 1.0).</param>
        /// <param name="b">Blue component (0.0 to 1.0).</param>
        /// <param name="a">Alpha component (0.0 to 1.0).</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetEndColor(string objectName, float r, float g, float b, float a);

        /// <summary>
        /// Sets the starting size of particles.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        /// <param name="size">Starting size.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetStartSize(string objectName, float size);

        /// <summary>
        /// Sets the ending size of particles (interpolated over lifetime).
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        /// <param name="size">Ending size.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetEndSize(string objectName, float size);

        /// <summary>
        /// Sets the render layer for particles.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        /// <param name="layer">Render layer.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetLayer(string objectName, int layer);

        /// <summary>
        /// Sets the fade in duration for particles.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        /// <param name="duration">Fade in duration in seconds.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetFadeIn(string objectName, float duration);

        /// <summary>
        /// Sets the fade out duration for particles.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        /// <param name="duration">Fade out duration in seconds.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetFadeOut(string objectName, float duration);

        /// <summary>
        /// Sets the texture for particles.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        /// <param name="texturePath">Path to the texture file.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetTexture(string objectName, string texturePath);
        #endregion

        #region Rotation Configuration
        /// <summary>
        /// Sets the starting rotation of particles.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        /// <param name="degrees">Starting rotation in degrees.</param>
        /// <param name="variation">Random variation (+/-).</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetStartRotation(string objectName, float degrees, float variation = 0f);

        /// <summary>
        /// Sets the angular velocity of particles.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        /// <param name="degreesPerSec">Angular velocity in degrees per second.</param>
        /// <param name="variation">Random variation (+/-).</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetAngularVelocity(string objectName, float degreesPerSec, float variation = 0f);
        #endregion

        #region Presets
        /// <summary>
        /// Configures the emitter for a dash trail effect.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void ConfigureDashTrail(string objectName);

        /// <summary>
        /// Configures the emitter for a jump dust effect.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void ConfigureJumpDust(string objectName);

        /// <summary>
        /// Configures the emitter for wall slide particles.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void ConfigureWallSlideParticles(string objectName);

        /// <summary>
        /// Configures the emitter for speed lines effect.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void ConfigureSpeedLines(string objectName);

        /// <summary>
        /// Configures the emitter for a death explosion effect.
        /// </summary>
        /// <param name="objectName">The name of the GameObject with the particle emitter.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void ConfigureDeathExplosion(string objectName);
        #endregion
    }

    public static class Input
    {
        /// Key codes mapping to standard GLFW/ASCII constants.
        /// </summary>
        public enum KeyCode
        {
            // Printable keys
            Space = 32,
            Apostrophe = 39,  /* ' */
            Comma = 44,       /* , */
            Minus = 45,       /* - */
            Period = 46,      /* . */
            Slash = 47,       /* / */

            // Alphabet
            A = 65, B = 66, C = 67, D = 68, E = 69, F = 70, G = 71, H = 72,
            I = 73, J = 74, K = 75, L = 76, M = 77, N = 78, O = 79, P = 80,
            Q = 81, R = 82, S = 83, T = 84, U = 85, V = 86, W = 87, X = 88,
            Y = 89, Z = 90,

            // Function Keys & Modifiers
            Escape = 256,
            Enter = 257,
            Right = 262,
            Left = 263,
            Down = 264,
            Up = 265,
            Pause = 284,
            LeftShift = 340,
            LeftControl = 341,
            LeftAlt = 342,
            RightShift = 344,
            RightControl = 345,
            RightAlt = 346,
            Menu = 348
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsKeyPressed(int key);
        public static vec2 Movement()
        {
            float x = 0;
            if (GetKeyHeld(KeyCode.D)) x = 1;
            else if (GetKeyHeld(KeyCode.A)) x = -1;

            float y = 0;
            if (GetKeyHeld(KeyCode.W)) y = 1;
            else if (GetKeyHeld(KeyCode.S)) y = -1;
            return new vec2(x, y);
        }

            [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsKeyDown(int key);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsKeyReleased(int key);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsKeyHeld(int key);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsMouseButtonPressed(int button);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsMouseButtonReleased(int button);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsMouseButtonHeld(int button);


        /// <summary>
        /// Gets the mouse X position in screen coordinates.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float GetMouseX();

        /// <summary>
        /// Gets the mouse Y position in screen coordinates.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float GetMouseY();

        /// <summary>
        /// Gets the mouse X position in world coordinates (accounting for camera).
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float GetMouseWorldX();

        /// <summary>
        /// Gets the mouse Y position in world coordinates (accounting for camera).
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float GetMouseWorldY();


        /// <summary>
        /// Checks if a specific key was pressed in the current frame.
        /// </summary>
        /// <param name="key">The KeyCode to check.</param>
        /// <returns>True if the key was pressed in the current frame.</returns>
        public static bool GetKeyPressed(KeyCode key)
        {
            return IsKeyPressed((int)key);
        }

        /// <summary>
        /// Checks if a specific key was released in the current frame.
        /// </summary>
        /// <param name="key">The KeyCode to check.</param>
        /// <returns>True if the key was released in the current frame.</returns>
        public static bool GetKeyReleased(KeyCode key)
        {
            return IsKeyReleased((int)key);
        }

        /// <summary>
        /// Checks if a specific key is currently held down.
        /// </summary>
        /// <param name="key">The KeyCode to check.</param>
        /// <returns>True if the key is pressed.</returns>
        public static bool GetKeyHeld(KeyCode key)
        {
            return IsKeyHeld((int)key);
        }

        /// <summary>
        /// Checks if a mouse button was pressed in the current frame.
        /// 0 = Left, 1 = Right, 2 = Middle.
        /// </summary>
        public static bool GetMouseButtonPressed(int button)
        {
            return IsMouseButtonPressed(button);
        }

        /// <summary>
        /// Checks if a mouse button was released in the current frame.
        /// 0 = Left, 1 = Right, 2 = Middle.
        /// </summary>
        public static bool GetMouseButtonReleased(int button)
        {
            return IsMouseButtonReleased(button);
        }

        /// <summary>
        /// Checks if a mouse button is held down.
        /// 0 = Left, 1 = Right, 2 = Middle.
        /// </summary>
        public static bool GetMouseButtonHeld(int button)
        {
            return IsMouseButtonHeld(button);
        }
    }

    /// <summary>
    /// Ghost player system — records the player's position and animation each frame,
    /// then drives a ghost object from the best recorded run.
    ///
    /// Typical usage:
    ///   OnStart:  Ghost.LoadRecording("M4_LEVEL");
    ///             Ghost.StartRecording("Hero");
    ///             if (Ghost.IsGhostAvailable()) Ghost.StartPlayback("GhostHero");
    ///   OnFinish: Ghost.StopRecording();
    ///             Ghost.SaveBestRun("M4_LEVEL", finishTime);
    /// </summary>
    public static class Ghost
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void StartRecording(string targetName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void StopRecording();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool SaveBestRun(string levelName, float finishTime);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void StartPlayback(string ghostObjectName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void StopPlayback();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool LoadRecording(string levelName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsGhostAvailable();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float GetBestRunTime();

        /// <summary>Begin recording the named target's position and animation each tick.</summary>
        public static void BeginRecording(string targetName) => StartRecording(targetName);

        /// <summary>Stop recording. The recording stays in memory.</summary>
        public static void EndRecording() => StopRecording();

        /// <summary>
        /// If finishTime beats the current best, save as new best and write to disk.
        /// Returns true if this is the new best run.
        /// </summary>
        public static bool SubmitRun(string levelName, float finishTime) => SaveBestRun(levelName, finishTime);

        /// <summary>Drive the named ghost GameObject from the best recording.</summary>
        public static void BeginPlayback(string ghostObjectName) => StartPlayback(ghostObjectName);

        /// <summary>Stop driving the ghost object.</summary>
        public static void EndPlayback() => StopPlayback();

        /// <summary>
        /// Load the best recording from disk. Call this before BeginPlayback on a new session.
        /// Returns true if a saved file was found.
        /// </summary>
        public static bool Load(string levelName) => LoadRecording(levelName);

        /// <summary>True if a best recording is available to play back.</summary>
        public static bool HasGhost() => IsGhostAvailable();

        /// <summary>Best run time in seconds. Returns float.MaxValue if no run recorded yet.</summary>
        public static float BestTime() => GetBestRunTime();
    }


}