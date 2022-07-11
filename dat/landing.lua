--[[
   Prototype function:

      Parameter: pnt - Planet to set landing stuff about.
      Return: 1) Boolean whether or not can land
              2) Land message which should be denial if can't land or acceptance if can
              3) (optional) Bribe price or message that can't be bribed or nil in the case of no possibility to bribe.
              4) (Needed with bribe price) Bribe message telling the price to pay
              5) (Needed with bribe price) Bribe acceptance message

   Examples:

   function yesland( pnt )
      return true, "Come on in dude"
   end

   function noland( pnt )
      return false, "Nobody expects the Spanish Inquisition!"
   end

   function noland_nobribe( pnt )
      return false, "No can do.", "Seriously, don't even think of bribing me dude."
   end

   function noland_yesbribe( pnt )
      return false, "You can't land buddy", 500, "But you can bribe for 500 credits", "Thanks for the money"
   end

--]]
local fmt = require "format"

-- Common utility functions, defined below.
local land_civilian

-- Default function. Any spob that has no landing script explicitly defined will use this.
function land( pnt )
   return land_civilian(pnt, 0, -30)
end

-- Specialized landing functions: Planets may specify <land>funcname</land> under services in their XML data.
-- The name will be looked up as a global function in this file. We declare each global to Luacheck to avoid warnings.

-- luacheck: globals pir_clanworld (Pirate clanworld.)
function pir_clanworld( pnt )
   local fct = pnt:faction()
   local standing = fct:playerStanding()
   local can_land = standing > 20 or pnt:getLandOverride()

   local land_msg
   if can_land then
      land_msg = _("Permission to land granted. Welcome, mate.")
   elseif standing >= 0 then
      land_msg = _("Small time pirates have no business on our clanworld!")
   else
      land_msg = _("Get out of here!")
   end

   -- Calculate bribe price. Custom for pirates.
   local bribe_price, bribe_msg, bribe_ack_msg
   if not can_land and standing >= -50 then
      bribe_price = (20 - standing) * 500 + 1000 -- 36K max, at -50 rep. Pirates are supposed to be cheaper than regular factions.
      local str   = fmt.credits( bribe_price )
      bribe_msg   = fmt.f(
               _([["Well, I think you're scum, but I'm willing to look the other way for {credits}. Deal?"]]),
	       {credits=str} )
      bribe_ack_msg = _("Heh heh, thanks. Now get off the comm, I'm busy!")
   end
   return can_land, land_msg, bribe_price, bribe_msg, bribe_ack_msg
end

-- Helper function for determining the bribe cost multiplier for the player's current ship.
-- Returns the factor the bribe cost is multiplied by when the player tries to bribe.
-- NOTE: This should be replaced by something better in time.
local function getshipmod()
   return player.pilot():ship():size()
end

-- Helper function for calculating bribe availability and cost.
-- Expects the faction, the minimum standing to land, the minimum standing to bribe, and a going rate for bribes.
-- Returns whether the planet can be bribed, and the cost for doing so.
local function getcost(fct, land_floor, bribe_floor, rate)
   local standing = fct:playerStanding()
   if standing < bribe_floor then
      return _("\"I'm not dealing with dangerous criminals like you!\"")
   else
      -- Assume standing is always lower than the land_floor.
      return (land_floor - standing) * rate * getshipmod() + 5000
   end
end

-- Civilian planet landing logic.
-- Expects the planet, the lowest standing at which landing is allowed, and the lowest standing at which bribing is allowed.
function land_civilian( pnt, land_floor, bribe_floor )
   local fct = pnt:faction()
   local can_land = fct:playerStanding() >= land_floor or pnt:getLandOverride()

   -- Get land message
   local land_msg
   if can_land then
      land_msg = _("Permission to land granted.")
   else
      land_msg = _("Landing request denied.")
   end

   local bribe_msg, bribe_ack_msg
   -- Calculate bribe price. Note: Assumes bribe floor < land_floor.
   local bribe_price = getcost(fct, land_floor, bribe_floor, 1000) -- TODO: different rates for different factions.
   if not can_land and type(bribe_price) == "number" then
       local str      = fmt.credits( bribe_price )
       bribe_msg      = fmt.f(
               _("\"I'll let you land for the modest price of {credits}.\"\n\nPay {credits}?"),
	       {credits=str} )
       bribe_ack_msg  = _("Make it quick.")
   end
   return can_land, land_msg, bribe_price, bribe_msg, bribe_ack_msg
end
