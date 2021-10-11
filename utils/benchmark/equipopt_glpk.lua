local equipopt = require 'equipopt'
local reps = 25
function benchmark( testname, sparams )
   local ships = {
      "Llama",
      "Hyena",
      "Ancestor",
      "Vendetta",
      "Lancelot",
      "Admonisher",
      "Pacifier",
      "Vigilance",
      "Kestrel",
      "Goddard",
   }
   local factions = {
      equipopt.generic,
      equipopt.empire,
      equipopt.zalek,
      equipopt.dvaered,
      equipopt.sirius,
      equipopt.soromid,
      equipopt.pirate,
      equipopt.thurion,
      equipopt.proteron,
   }

   pilot.clear()
   local pos = vec2.new(0,0)
   local vals = {}
   for i=1,reps do
      local rstart = naev.clock()
      for k,s in ipairs(ships) do
         for k,f in ipairs(factions) do
            local p = pilot.add( s, "Dummy", pos, nil, {naked=true} )
            if sparams then
               f( p )
               --f( p, {rnd=0.0})
            end
            p:rm()
         end
      end
      table.insert( vals, (naev.clock()-rstart)*1000 )
   end

   local mean = 0
   local stddev = 0
   for k,v in ipairs(vals) do
      mean = mean + v
   end
   mean = mean / #vals
   for k,v in ipairs(vals) do
      stddev = stddev + math.pow(v-mean, 2)
   end
   stddev = math.sqrt(stddev / #vals)

   return mean, stddev, vals
end

local csvfile = file.new("benchmark.csv")
csvfile:open("w")
csvfile:write( "mean,stddev,br_tech,bt_tech,pp_tech,sr_heur,fp_heur,ps_heur,gmi_cuts,mir_cuts,cov_cuts,clq_cuts" )
for i=1,reps do
   csvfile:write(string.format(",rep%d",i))
end
csvfile:write("\n")

function csv_writereps( vals )
   for i,v in ipairs(vals) do
      csvfile:write(string.format(",%f",v))
   end
   csvfile:write("\n")
end

local tstart = naev.clock()
local ntotal = math.pow(2,7)*3*4*5 + 1
print("====== BENCHMARK START ======")
local bl_mean, bl_stddev, bl_vals = benchmark( "Baseline" )
local def_mean, def_stddev, def_vals = benchmark( "Defaults", {} )
csvfile:write(string.format("%f,%f,-,-,-,-,-,-,-,-,-,-", bl_mean, bl_stddev ) )
csv_writereps( bl_vals )
csvfile:write(string.format("%f,%f,def,def,def,def,def,def,def,def,def,def,def", def_mean, def_stddev ) )
csv_writereps( def_vals )
local curbest = def_mean
local n = 2
for i,br_tech in ipairs{"ffv","lfv","mfv","dth","pch"} do
   for i,bt_tech in ipairs{"dfs","bfs","blb","bph"} do
      for i,pp_tech in ipairs{"none","root","all"} do
         for i,sr_heur in ipairs{"on","off"} do
            for i,fp_heur in ipairs{"on","off"} do
               for i,ps_heur in ipairs{"on","off"} do
                  for i,gmi_cuts in ipairs{"on","off"} do
                     for i,mir_cuts in ipairs{"on","off"} do
                        for i,cov_cuts in ipairs{"on","off"} do
                           for i,clq_cuts in ipairs{"on","off"} do
local s = string.format("br=%s,bt=%s,pp=%s,sr=%s,fp=%s,ps=%s,gmi=%s,mir=%s,cov=%s,clq=%s",
   br_tech, bt_tech, pp_tech,
   sr_heur, fp_heur, ps_heur,
   gmi_cuts, mir_cuts, cov_cuts, clq_cuts )
local mean, stddev, vals = benchmark( s, {
      br_tech=br_tech, bt_tech=bt_tech, pp_tech=pp_tech,
      sr_heur=sr_heur, fp_heur=fp_heur, ps_heur=ps_heur,
      gmi_cuts=gmi_cuts, mir_cuts=mir_cuts, cov_cuts=cov_cuts, clq_cuts=clq_cuts,
   } )
csvfile:write( string.format("%f,%f,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s", mean, stddev,
   br_tech, bt_tech, pp_tech,
   sr_heur, fp_heur, ps_heur,
   gmi_cuts, mir_cuts, cov_cuts, clq_cuts ) )
csv_writereps( vals )
if mean < curbest then
   curbest = mean
end
local left = ntotal - n - 1
local elapsed = naev.clock() - tstart
print(string.format("Best: %.3f ms, Cur: %.3f (%.3f) ms, (%d of %d done, %.3f hours left)",
   curbest, mean, stddev, n, ntotal, elapsed * left / n / 3600) )
n = n+1
                           end
                        end
                     end
                  end
               end
            end
         end
      end
   end
end
print("====== BENCHMARK END ======")
csvfile:close()

