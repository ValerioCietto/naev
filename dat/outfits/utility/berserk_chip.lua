
require 'outfits.shaders'
threshold = 30 -- armour shield damage to turn off at
cooldown = 8 -- cooldown time in seconds
drain_shield = 1.0 / 2.0 -- inverse of number of seconds needed to drain shield
drain_armour = 1.0 / 50.0 -- inverse of number of seconds needed to drain armour (this is accelerated by this amount every second)
ppshader = shader_new([[
#include "lib/blend.glsl"
const vec3 colmod = vec3( 1.0, 0.0, 0.0 );
uniform float progress = 0;
vec4 effect( sampler2D tex, vec2 texcoord, vec2 pixcoord )
{
   vec4 color     = texture( tex, texcoord );
   float opacity  = clamp( progress, 0.0, 1.0 );
   color.rgb      = blendSoftLight( color.rgb, colmod, opacity );
   return color;
}
]])


function turnon( p, po )
   -- Still on cooldown
   if mem.timer and mem.timer > 0 then
      return false
   end
   -- Must be above armour threshold
   local a, s = p:health()
   if a < threshold then
      return false
   end
   po:state("on")
   po:progress(0) -- No progress so just fill out the bar
   mem.active = true

   -- Apply damaging effect
   local ps = p:stats()
   mem.ainc = ps.armour * drain_armour
   mem.admg = mem.ainc
   po:set( "armour_regen_malus", mem.admg )
   po:set( "shield_regen_malus",  ps.shield * drain_shield ) -- shield gone in 2 secs

   -- Visual effect
   if mem.isp then shader_on() end

   return true
end

function turnoff( p, po )
   if not mem.active then
      return false
   end
   po:state("cooldown")
   po:progress(1)
   mem.timer = cooldown
   mem.active = false
   shader_off()
   po:set( "armour_regen_malus", 0 )
   po:set( "shield_regen_malus",  0 )
   po:set( "launch_damage", -20 )
   po:set( "fwd_damage", -20 )
   po:set( "tur_damage", -20 )
   po:set( "turn_mod", -20 )
   po:set( "thrust_mod", -20 )
   po:set( "speed_mod", -20 )
   return true
end

function init( p, po )
   turnoff()
   mem.timer = nil
   po:state("off")
   po:clear() -- clear stat modifications
   mem.isp = (p == player.pilot())
   shader_force_off()
end

function update( p, po, dt )
   if mem.active then
      shader_update_on(dt)
      local a, s = p:health()
      if a < threshold then
         turnoff( p, po )
      else
         mem.admg = mem.admg + mem.ainc * dt
         po:set( "armour_regen_malus", mem.admg )
      end
   else
      if mem.timer then
         shader_update_cooldown(dt)
         mem.timer = mem.timer - dt
         po:progress( mem.timer / cooldown )
         if mem.timer < 0 then
            po:state("off")
            po:clear() -- clear stat modifications
            mem.timer = nil
            shader_force_off()
         end
      end
   end
end

function ontoggle( p, po, on )
   if on then
      return turnon( p, po )
   else
      return turnoff( p, po )
   end
end
