/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_linopt.c
 *
 * @brief Handles Linear linoptization in Lua.
 */

#include <glpk.h>

/** @cond */
#include <lauxlib.h>

#include "naev.h"
/** @endcond */

#include "nlua_linopt.h"

#include "log.h"
#include "nluadef.h"


typedef struct LuaLinOpt_s {
   int ncols;
   int nrows;
   glp_prob *prob;
} LuaLinOpt_t;


/* Optim metatable methods. */
static int linoptL_gc( lua_State *L );
static int linoptL_eq( lua_State *L );
static int linoptL_new( lua_State *L );
static int linoptL_addcols( lua_State *L );
static int linoptL_addrows( lua_State *L );
static int linoptL_setcol( lua_State *L );
static int linoptL_setrow( lua_State *L );
static int linoptL_loadmatrix( lua_State *L );
static int linoptL_solve( lua_State *L );
static const luaL_Reg linoptL_methods[] = {
   { "__gc", linoptL_gc },
   { "__eq", linoptL_eq },
   { "new", linoptL_new },
   { "add_cols", linoptL_addcols },
   { "add_rows", linoptL_addrows },
   { "set_col", linoptL_setcol },
   { "set_row", linoptL_setrow },
   { "load_matrix", linoptL_loadmatrix },
   { "solve", linoptL_solve },
   {0,0}
}; /**< Optim metatable methods. */




/**
 * @brief Loads the linopt library.
 *
 *    @param env Environment to load linopt library into.
 *    @return 0 on success.
 */
int nlua_loadLinOpt( nlua_env env )
{
   nlua_register(env, LINOPT_METATABLE, linoptL_methods, 1);
   return 0;
}


/**
 * @brief Lua bindings to interact with linopts.
 *
 * @luamod linopt
 */
/**
 * @brief Gets linopt at index.
 *
 *    @param L Lua state to get linopt from.
 *    @param ind Index position to find the linopt.
 *    @return Optim found at the index in the state.
 */
LuaLinOpt_t* lua_tolinopt( lua_State *L, int ind )
{
   return (LuaLinOpt_t*) lua_touserdata(L,ind);
}
/**
 * @brief Gets linopt at index or raises error if there is no linopt at index.
 *
 *    @param L Lua state to get linopt from.
 *    @param ind Index position to find linopt.
 *    @return Optim found at the index in the state.
 */
LuaLinOpt_t* luaL_checklinopt( lua_State *L, int ind )
{
   if (lua_islinopt(L,ind))
      return lua_tolinopt(L,ind);
   luaL_typerror(L, ind, LINOPT_METATABLE);
   return NULL;
}
/**
 * @brief Pushes a linopt on the stack.
 *
 *    @param L Lua state to push linopt into.
 *    @param linopt Optim to push.
 *    @return Newly pushed linopt.
 */
LuaLinOpt_t* lua_pushlinopt( lua_State *L, LuaLinOpt_t linopt )
{
   LuaLinOpt_t *c;
   c = (LuaLinOpt_t*) lua_newuserdata(L, sizeof(LuaLinOpt_t));
   *c = linopt;
   luaL_getmetatable(L, LINOPT_METATABLE);
   lua_setmetatable(L, -2);
   return c;
}
/**
 * @brief Checks to see if ind is a linopt.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a linopt.
 */
int lua_islinopt( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, LINOPT_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}


/**
 * @brief Frees a linopt.
 *
 *    @luatparam Optim linopt Optim to free.
 * @luafunc __gc
 */
static int linoptL_gc( lua_State *L )
{
   LuaLinOpt_t *lp = luaL_checklinopt(L,1);
   glp_delete_prob(lp->prob);
   return 0;
}


/**
 * @brief Compares two linopts to see if they are the same.
 *
 *    @luatparam Optim d1 Optim 1 to compare.
 *    @luatparam Optim d2 Optim 2 to compare.
 *    @luatreturn boolean true if both linopts are the same.
 * @luafunc __eq
 */
static int linoptL_eq( lua_State *L )
{
   LuaLinOpt_t *lp1, *lp2;
   lp1 = luaL_checklinopt(L,1);
   lp2 = luaL_checklinopt(L,2);
   lua_pushboolean( L, (memcmp( lp1, lp2, sizeof(LuaLinOpt_t))==0) );
   return 1;
}


/**
 * @brief Opens a new linopt.
 *
 *    @luatparam[opt=nil] string name Name of the optimization program.
 *    @luatparam number cols Number of columns in the optimization program.
 *    @luatparam number rows Number of rows in the optimization program.
 *    @luatparam[opt=false] boolean maximize Whether or not to maximize or minimize the function.
 *    @luatreturn Optim New linopt object.
 * @luafunc new
 */
static int linoptL_new( lua_State *L )
{
   LuaLinOpt_t lp;
   const char *name;
   int max;

   /* Input. */
   name     = luaL_optstring(L,1,NULL);
   lp.ncols = luaL_checkinteger(L,2);
   lp.nrows = luaL_checkinteger(L,3);
   max      = lua_toboolean(L,4);

   /* Initialize and create. */
   lp.prob = glp_create_prob();
   glp_set_prob_name( lp.prob, name );
   glp_add_cols( lp.prob, lp.ncols );
   glp_add_rows( lp.prob, lp.nrows );
   if (max)
      glp_set_obj_dir( lp.prob, GLP_MAX );

   lua_pushlinopt( L, lp );
   return 1;
}


/**
 * @brief Adds columns to the linear program.
 *
 *    @luatparam LinOpt lp Linear program to modify.
 *    @luatparam number cols Number of columns to add.
 * @luafunc add_cols
 */
static int linoptL_addcols( lua_State *L )
{
   LuaLinOpt_t *lp   = luaL_checklinopt(L,1);
   int toadd         = luaL_checkinteger(L,2);
   glp_add_cols( lp->prob, toadd );
   lp->ncols += toadd;
   return 0;
}


/**
 * @brief Adds rows to the linear program.
 *
 *    @luatparam LinOpt lp Linear program to modify.
 *    @luatparam number rows Number of rows to add.
 * @luafunc add_rows
 */
static int linoptL_addrows( lua_State *L )
{
   LuaLinOpt_t *lp   = luaL_checklinopt(L,1);
   int toadd         = luaL_checkinteger(L,2);
   glp_add_rows( lp->prob, toadd );
   lp->nrows += toadd;
   return 0;
}


/**
 * @brief Adds an optimization column.
 *
 *    @luatparam LinOpt lp Linear program to modify.
 *    @luatparam number index Index of the column to set.
 *    @luatparam string name Name of the column being added.
 *    @luatparam number coefficient Coefficient of the objective function being added.
 *    @luatparam[opt="real"] string kind Kind of the column being added. Can be either "real", "integer", or "binary".
 *    @luatparam[opt=nil] number lb Lower bound of the column.
 *    @luatparam[opt=nil] number ub Upper bound of the column.
 * @luafunc set_col
 */
static int linoptL_setcol( lua_State *L )
{
   LuaLinOpt_t *lp   = luaL_checklinopt(L,1);
   int idx           = luaL_checkinteger(L,2);
   const char *name  = luaL_checkstring(L,3);
   double coef       = luaL_checknumber(L,4);
   const char *skind = luaL_optstring(L,5,"real");
   int haslb, hasub, type, kind;
   double lb, ub;

   /* glpk stuff */
   glp_set_col_name( lp->prob, idx, name );
   glp_set_obj_coef( lp->prob, idx, coef );

   /* Determine bounds. */
   haslb = !lua_isnoneornil(L,6);
   hasub = !lua_isnoneornil(L,7);
   lb    = luaL_optnumber(L,6,0.0);
   ub    = luaL_optnumber(L,7,0.0);
   if (haslb && hasub)
      type = GLP_DB;
   else if (haslb)
      type = GLP_LO;
   else if (hasub)
      type = GLP_UP;
   else
      type = GLP_FR;
   glp_set_col_bnds( lp->prob, idx, type, lb, ub );

   /* Get kind. */
   if (strcmp(skind,"real")==0)
      kind = GLP_CV;
   else if (strcmp(skind,"integer")==0)
      kind = GLP_IV;
   else if (strcmp(skind,"binary")==0)
      kind = GLP_BV;
   else
      NLUA_ERROR(L,_("Unknown column kind '%s'!"), skind);
   glp_set_col_kind( lp->prob, idx, kind );

   return 0;
}


/**
 * @brief Adds an optimization row.
 *
 *    @luatparam LinOpt lp Linear program to modify.
 *    @luatparam number index Index of the row to set.
 *    @luatparam string name Name of the row being added.
 *    @luatparam[opt=nil] number lb Lower bound of the row.
 *    @luatparam[opt=nil] number ub Upper bound of the row.
 * @luafunc set_row
 */
static int linoptL_setrow( lua_State *L )
{
   LuaLinOpt_t *lp   = luaL_checklinopt(L,1);
   int idx           = luaL_checkinteger(L,2);
   const char *name  = luaL_checkstring(L,3);
   int haslb, hasub, type;
   double lb, ub;

   /* glpk stuff */
   glp_set_row_name( lp->prob, idx, name );

   /* Determine bounds. */
   haslb = !lua_isnoneornil(L,4);
   hasub = !lua_isnoneornil(L,5);
   lb    = luaL_optnumber(L,4,0.0);
   ub    = luaL_optnumber(L,5,0.0);
   if (haslb && hasub)
      type = GLP_DB;
   else if (haslb)
      type = GLP_LO;
   else if (hasub)
      type = GLP_UP;
   else
      type = GLP_FR;
   glp_set_row_bnds( lp->prob, idx, type, lb, ub );

   return 0;
}


/**
 * @brief Loads the entire matrix for the linear program.
 *
 *    @luatparam LinOpt lp Linear program to modify.
 *    @luatparam number row_indices Indices of the rows.
 *    @luatparam number col_indices Indices of the columns.
 *    @luatparam number coefficients Values of the coefficients.
 * @luafunc load_matrix
 */
static int linoptL_loadmatrix( lua_State *L )
{
   size_t i, n;
   int *ia, *ja;
   double *ar;
   LuaLinOpt_t *lp   = luaL_checklinopt(L,1);
   luaL_checktype(L, 2, LUA_TTABLE);
   luaL_checktype(L, 3, LUA_TTABLE);
   luaL_checktype(L, 4, LUA_TTABLE);

   /* Make sure size is ok. */
   n = lua_objlen(L,2);
#if DEBUGGING
   if ((n != lua_objlen(L,3)) || (n != lua_objlen(L,4)))
      NLUA_ERROR(L, _("Table lengths don't match!"));
#endif /* DEBUGGING */

   /* Load everything from tables, has to be 1-index based. */
   ia = calloc( n+1, sizeof(int) );
   ja = calloc( n+1, sizeof(int) );
   ar = calloc( n+1, sizeof(double) );
   for (i=1; i<=n; i++) {
      lua_rawgeti(L, 2, i);
      lua_rawgeti(L, 3, i);
      lua_rawgeti(L, 4, i);
#if DEBUGGING
      ia[i] = luaL_checkinteger(L,-3);
      ja[i] = luaL_checkinteger(L,-2);
      ar[i] = luaL_checknumber(L,-1);
#else /* DEBUGGING */
      ia[i] = lua_tointeger(L,-3);
      ja[i] = lua_tointeger(L,-2);
      ar[i] = lua_tonumber(L,-1);
#endif /* DEBUGGING */
      lua_pop(L,3);
   }

   /* Set up the matrix. */
   glp_load_matrix( lp->prob, n, ia, ja, ar );

   /* Clean up. */
   free(ia);
   free(ja);
   free(ar);
   return 0;
}


/**
 * @brief Solves the linear optimization problem.
 *
 *    @luatparam LinOpt lp Linear program to modify.
 *    @luatreturn number The value of the primal funcation.
 *    @luatreturn table Table of column values.
 * @luafunc solve
 */
static int linoptL_solve( lua_State *L )
{
   LuaLinOpt_t *lp   = luaL_checklinopt(L,1);
   double z;
   //const char *name;
   int i, ismip;

   /* Parameters. */
   ismip = (glp_get_num_int( lp->prob ) > 0);

   /* Optimization. */
   if (ismip) {
      glp_iocp parm;
      glp_init_iocp(&parm);
      parm.presolve = GLP_ON;
      //parm.msg_lev = GLP_MSG_ERR;
      glp_intopt( lp->prob, &parm );
      /* TODO handle errors. */
      z = glp_mip_obj_val( lp->prob );
   }
   else {
      glp_smcp parm;
      glp_init_smcp(&parm);
      //parm.msg_lev = GLP_MSG_ERR;
      glp_simplex( lp->prob, &parm );
      /* TODO handle errors. */
      z = glp_get_obj_val( lp->prob );
   }

   /* Output function value. */
   lua_pushnumber(L,z);

   /* Go over variables and store them. */
   lua_newtable(L); /* t */
   for (i=1; i<=lp->ncols; i++) {
      if (ismip)
         z = glp_mip_col_val( lp->prob, i );
      else
         z = glp_get_col_prim( lp->prob, i );
      lua_pushnumber( L, z ); /* t, z */
      lua_rawseti( L, -2, i ); /* t */
   }

   /* Go over constraints and store them. */
   lua_newtable(L); /* t */
   for (i=1; i<=lp->nrows; i++) {
      if (ismip)
         z = glp_mip_row_val( lp->prob, i );
      else
         z = glp_get_row_prim( lp->prob, i );
      lua_pushnumber( L, z ); /* t, z */
      lua_rawseti( L, -2, i ); /* t */
   }

   return 3;
}

