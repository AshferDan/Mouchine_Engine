using CSharpScript;
using System;
using System.Drawing;
using System.Numerics;
using System.Threading;
using static System.Math;

namespace GameScripts
{
    public class Zipline
    {
        //[Tooltip("The BoxCollider2D used to define the zipline's path.")]
        //[SerializeField] private BoxCollider2D ziplineCollider;

        //[Header("Visuals")]
        //[SerializeField] private ParticleSystem debrisEffectPrefab;

        //private ParticleSystem debrisEffectInstance;

        // public properties to get the world-space start and end points of the zipline
        public string objectName;
        public vec2 colliderOffset;
        public float colliderWidth;

        public vec2 WorldStartPoint(string objectName, vec2 colliderOffset, float colliderWidth)
        {
            vec2 localpos = new vec2(colliderOffset.x - colliderWidth * 0.5f, colliderOffset.y);

            // world space
            return Transform.TransformPoint(objectName, localpos);
        }
        public vec2 WorldEndPoint(string objectName, vec2 colliderOffset, float colliderWidth)
        {
            vec2 localpos = new vec2(colliderOffset.x + colliderWidth * 0.5f, colliderOffset.y);

            return Transform.TransformPoint(objectName, localpos);
        }

        private void OnValidate()
        {
            // automatically get the collider if not assigned
            //if (ziplineCollider == null)
            {
                //ziplineCollider = GetComponent<BoxCollider2D>();
            }
        }

        // particle effects
        //public void PlayDebrisEffect()
        //{
        //    // Do nothing if no prefab is assigned
        //    if (debrisEffectPrefab == null) return;

        //    // Create an instance of the effect if it doesn't exist yet
        //    if (debrisEffectInstance == null)
        //    {
        //        debrisEffectInstance = Instantiate(debrisEffectPrefab, transform.position, Quaternion.identity);
        //    }

        //    // Play the effect
        //    debrisEffectInstance.Play();
        //}

        //public void UpdateDebrisEffectPosition(Vector2 position)
        //{
        //    if (debrisEffectInstance != null)
        //    {
        //        debrisEffectInstance.transform.position = position;
        //    }
        //}

        //public void StopDebrisEffect()
        //{
        //    if (debrisEffectInstance != null)
        //    {
        //        debrisEffectInstance.Stop();
        //    }
        //}
    }
}
